#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    //for sleep function testing
#include <ncurses.h>

#define DISPLAY_RESOLUTION_HORIZONTAL 64
#define DISPLAY_RESOLUTION_VERTICAL 32
#define OPCODE_SIZE_INBYTES 2
#define CHIP8_MEMORY_LIMIT 4096
#define CHIP8_STACK_SIZE 12

typedef struct chip8processor { 
    uint8_t memory[CHIP8_MEMORY_LIMIT]; 
    uint8_t registers[16];
    // uint16_t stack[CHIP8_STACK_SIZE];        //Stack is now inside of memory
    uint16_t stackSize;
    uint8_t delayTimer;
    uint8_t delayFlag;                          //If 1, active delay 
    uint8_t soundTimer;
    uint8_t soundFlag;                          //If 1, active sound 
    unsigned char userinput;                    //saves the last user input
    uint8_t userinput_flag;                     //if 0, no key have ever been pressed
    uint16_t programCounter;
    uint16_t stackPointer;
    uint16_t addressRegister;
    double time_spent_sound;                    
    double time_spent_delay;
    uint64_t displayGrid[DISPLAY_RESOLUTION_VERTICAL];  //Hold display
} chip8processor;   

chip8processor* init_chip8(void);
void destory_chip(chip8processor* pi);
void debug_chip8_state(chip8processor* p1, FILE* debug_File) ;
void view_program_memory(chip8processor* p1, FILE* debug_File) ;
void close_program(chip8processor* pi , int randomData);
void draw_display(chip8processor* p1, WINDOW * win);


int main(int argc, char *argv[]) {
    //Check too she if a filename was given
    if (argc != 2) {
        fprintf(stderr,"Error: useage: chip8emulator <filename>\n");
        return 1;
    }

    //Variables used
    FILE *programFile;
    long int fileSize;
    unsigned char * buffer = (unsigned char*) malloc (sizeof(char)*OPCODE_SIZE_INBYTES); //stores op code
    //size_t result;
    uint32_t* randbits = 0;                                                         //Stores Ranbit from /dev/urandom
    clock_t time_begin = 0, time_end = 0;                                       //Used to generate time delay

    //Open file from arg
    programFile = fopen(argv[1], "r");

    //Check to see if the file can be opened
    if(programFile == NULL) {
        fprintf(stderr,"Error: Cannot open file or file not found\n");
        return 1;
    }

    // obtain file size and check to see if it is a valid file
    fseek (programFile , 0 , SEEK_END);
    fileSize = ftell (programFile);
    rewind (programFile); //rewind back to beginning of program

    // Check to see if the file size matches the opcode format
    if(fileSize % 2 != 0) {
        fprintf(stderr,"Error: file is not the correct format\n");
        return 1;
    }

    //Create a chip8 processor with zero values
    chip8processor* p1 = init_chip8();

    //start ncurses
	initscr();			        // Start curses mode 		
	cbreak();			        // Line buffering disabled
	keypad(stdscr, TRUE);		// Capture special key such f1 etc. 
    noecho();                   //Supress echo
    refresh();

    #ifdef DEBUG
        FILE* debug_File = fopen("./debug.log","w");
    #endif

    //Load rom into ram
    while(!feof(programFile)) {
        //set the buffer to all zeros
        memset(buffer,0,OPCODE_SIZE_INBYTES);

        //Get the 2 byte opcode and put it into the buffer
        //result = fread(buffer,1,OPCODE_SIZE_INBYTES, programFile);
        fread(buffer,1,OPCODE_SIZE_INBYTES, programFile);

        //if end of file, exit loop
        if(feof(programFile)) {
            break;
        }

        p1->memory[p1->programCounter] = buffer[0];
        p1->programCounter++;
        p1->memory[p1->programCounter] = buffer[1];
        p1->programCounter++;

        // If the program being loaded goes pass limit of the memory, throw a error;
        if(p1->programCounter > 0xF00) {
            perror("Error: Program being loaded excedes the memory of the processor\n");
            free(p1);
            exit(1);
        }
    }

    //Free buffer after use
    free(buffer);

    // Reset program counter to base and closes the file
    p1->programCounter = 0x200;
    fclose(programFile);

    int randomData = open("/dev/urandom", O_RDONLY);// 
    if (randomData < 0)
    {
        // something went wrong
        perror("ERROR: Cannot open /dev/urandom\n");
        exit(1);
    }   

    #ifdef DEBUG
        view_program_memory(p1, debug_File);
    #endif

    WINDOW * win = newwin(DISPLAY_RESOLUTION_VERTICAL + 2, DISPLAY_RESOLUTION_HORIZONTAL + 2, 0, 0);
    box(win, 0 , 0);
    wrefresh(win);
    wmove(win, 1, 1);

    while(0 == 0) {
        #ifdef DEBUG
            debug_chip8_state(p1, debug_File);
        #endif

        if(p1->delayFlag != 0) {
            //Delay program
            if(p1->time_spent_delay >= (double)p1->delayTimer/60) {
                p1->delayFlag = 0;
                p1->programCounter += 2;
            }
            time_end = clock();
            p1->time_spent_delay += (double)(time_end - time_begin) / CLOCKS_PER_SEC;
            time_begin = time_end;
            continue;
        }

        if ((p1->memory[p1->programCounter] >> 4) == 0x0){
            //0x00 intructsions
            if((p1->memory[p1->programCounter] & 0xf) == 0x0 && 
            (p1->memory[p1->programCounter + 1] ) == 0xe0) {
                //call clear screen
                werase(win);
                box(win, 0 , 0);
                wmove(win, 1, 1);
                wrefresh(win);
            }
            else if ((p1->memory[p1->programCounter] & 0xf) == 0 && 
            (p1->memory[p1->programCounter + 1] == 0xee)) {
                //Return from subroutine
                if(p1->stackSize == 0) {
                    perror("ERROR: Stack underflowed into memory bounds\n");
                    close_program(p1, randomData);
                    exit(1);
                }
                p1->stackPointer -= 2;
                uint16_t tempAddress = 0;
                tempAddress = p1->memory[p1->stackPointer] & 0x0f;
                tempAddress = tempAddress << 8;
                tempAddress = tempAddress | p1->memory[p1->stackPointer + 1];
                p1->stackSize -= 1;
                p1->programCounter = tempAddress;
                continue;
            }
            else {
                // call subroutine RCA, this can go into below 0x200 region
            
                //Store program counter into stack
                if(p1->stackSize > CHIP8_STACK_SIZE) {
                    perror("ERROR: Stack overflowed into memory bounds\n");
                    close_program(p1, randomData);
                    exit(1);
                }
                p1->memory[p1->stackPointer] = ((p1->programCounter + 2) & 0xf00) >> 8;    //grab the programcounter's first 4 bits
                p1->memory[p1->stackPointer + 1] = (p1->programCounter & 0xff);
                p1->stackPointer += 2;
                p1->stackSize += 1;

                uint16_t tempAddress = 0;
                tempAddress = p1->memory[p1->programCounter] & 0x0f;
                tempAddress = tempAddress << 8;
                tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

                if(tempAddress > 0xEA0) {
                    perror("ERROR: Program ran out of memory bounds\n");
                    close_program(p1, randomData);
                    exit(1);
                }

                p1->programCounter = tempAddress;

                continue;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x1) {
            //jump command
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1->memory[p1->programCounter] & 0x0f;
            tempAddress = tempAddress << 8;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            p1->programCounter = tempAddress;
            continue;
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x2) {
            // call subroutine
            
            //Store program counter into stack
            if(p1->stackSize > CHIP8_STACK_SIZE) {
                perror("ERROR: Stack overflowed into memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }
            p1->memory[p1->stackPointer] = ((p1->programCounter + 2) & 0xf00) >> 8;    //grab the programcounter's first 4 bits
            p1->memory[p1->stackPointer + 1] = ((p1->programCounter + 2) & 0xff);
            p1->stackPointer += 2;
            p1->stackSize += 1;

            uint16_t tempAddress = 0;
            tempAddress = p1->memory[p1->programCounter] & 0x0f;
            tempAddress = tempAddress << 8;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            p1->programCounter = tempAddress;

            continue;
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x3) {
            // Skip if Vx == NN
            if(p1->registers[p1->memory[p1->programCounter] & 0xf] == p1->memory[p1->programCounter + 1]) {
                p1->programCounter += 2;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x4) {
            // Skip if Vx != NN
            if(p1->registers[p1->memory[p1->programCounter] & 0xf] != p1->memory[p1->programCounter + 1]) {
                p1->programCounter += 2;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x5) {
            // Skip if Vx == Vy
            
            if(p1->registers[p1->memory[p1->programCounter] & 0xf] == p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4] &&
            (p1->memory[p1->programCounter + 1] & 0x0f) == 0) {
                p1->programCounter += 2;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x6) {
           //  Vx = NN
            p1->registers[p1->memory[p1->programCounter] & 0xf] = p1->memory[p1->programCounter + 1];
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x7) {
            // Vx += NN
            p1->registers[p1->memory[p1->programCounter] & 0xf] += p1->memory[p1->programCounter + 1];
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x8) {
            // Commmand Vx = Vy
            if((p1->memory[p1->programCounter + 1] & 0x0f) == 0x0) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4];
            }
            // Command Vx = Vx | Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x1) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4] | p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Commmand 	Vx=Vx&Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x2) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4] & p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Commmand 	Vx=Vx^Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x3) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4] ^ p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Command Vx += Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x4) {
                uint32_t check_flag = p1->registers[p1->memory[p1->programCounter] & 0x0f] + p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4];
                if (check_flag > 0xff) {
                    p1->registers[0xf] = 1;
                }
                p1->registers[p1->memory[p1->programCounter] & 0x0f] += p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4];
            }
            // Command Vx -= Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x5) {
                int32_t check_flag = p1->registers[p1->memory[p1->programCounter] & 0x0f] - p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4];
                if (check_flag < 0) {
                     p1->registers[0xf] = 1;
                }
                p1->registers[p1->memory[p1->programCounter] & 0x0f] -= p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4];
            }
            // Command Vx >>= 1
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x6) {
                p1>registers[0xf] = p1->registers[p1->memory[p1->programCounter] & 0x0f] & 0x1;
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter] & 0x0f] >> 1;
            }
            // Command Vx = Vy - Vx
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x7) {
                int32_t check_flag = p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4] - p1->registers[p1->memory[p1->programCounter] & 0x0f] ;
                if (check_flag < 0) {
                     p1->registers[0xf] = 1;
                }
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4] - p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Command Vx <<= 1
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0xe) {
                p1>registers[0xf] = p1->registers[p1->memory[p1->programCounter] & 0x0f] & 0x80;
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter] & 0x0f] << 1;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x9) {
            // Skip if Vx != Vy
            if(p1->registers[p1->memory[p1->programCounter] & 0xf] != p1->registers[(p1->memory[p1->programCounter + 1] & 0xf0) >> 4] &&
            (p1->memory[p1->programCounter + 1] & 0x0f) == 0) {
                p1->programCounter += 2;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xa) {
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1->memory[p1->programCounter] & 0x0f;
            tempAddress = tempAddress << 8;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            p1->addressRegister = tempAddress;
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xb) {
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1->memory[p1->programCounter] & 0x0f;
            tempAddress = tempAddress << 8;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            tempAddress += p1->registers[0];

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            p1->programCounter = tempAddress;
            continue;
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xc) {
            //ssize_t result = read(randomData, randbits, sizeof(randbits));
            read(randomData, randbits, sizeof(randbits));
            p1->registers[p1->memory[p1->programCounter] & 0xf ] = *randbits & p1->memory[p1->programCounter + 1];
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xd) {
            //Draw DXYN
            uint8_t height = p1->memory[p1->programCounter + 1] & 0xf // Grab N from opcode
            uint16_t i_temp = p1->addressRegister;
            uint16_t disp_add = 0xF00;
            uint8_t xPixel = p1->registers[p1->memory[p1->programCounter] & 0xf];
            uint8_t yPixel = p1->registers[p1->memory[p1->programCounter + 1] >> 4];
            uint8_t check_flip = 0;

            if(xPixel > DISPLAY_RESOLUTION_HORIZONTAL - 8 || yPixel + height > DISPLAY_RESOLUTION_VERTICAL) {
                //Check draw boundry
                perror("ERROR: Program Draw out of bound\n");
                close_program(p1, randomData);
                exit(1);
            }

            for(uint8_t j = 0; j < 0; j++) {
                check_flip = (p1->dispayGird[yPixel] >> (56 - xPixel) ) & p1->memory[i_temp]; 
                if(check_flip > 0) {
                    p1->registers[0xf] = 1;
                }   
                p1->displayGird[yPixel] = p1->diplayGrid ^ (p1->memory[i_temp] << (56 - xPixel));
            }
            /*
            wmove(win, 1, 1);
            waddstr(win, "THIS IS S TEXT");
            wrefresh(win);
            */
            draw_display(chip8processor* p1, WINDOW * win);
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xe) {
            if(p1->memory[p1->programCounter + 1] == 0x9e) {
                // Skip if Vx == key()
                if(p1->registers[p1->memory[p1->programCounter] & 0xf] == p1->userinput && p1->userinput_flag != 0) {
                    p1->programCounter += 2;
                }
            }
            else if(p1->memory[p1->programCounter + 1] == 0xa1){
                if(p1->registers[p1->memory[p1->programCounter] & 0xf] != p1->userinput && p1->userinput_flag != 0) {
                    p1->programCounter += 2;
                }
            }
            
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xf) {
            if(p1->memory[p1->programCounter + 1] == 0x07) {
                p1->registers[p1->memory[p1->programCounter] & 0xf] = p1->delayTimer;
            }
            else if(p1->memory[p1->programCounter + 1] == 0x0a) {
                int valid_character = 0, ch = 0;
                
                while(valid_character == 0) {
                    ch = getch();   //Get keybaord input

                    switch(ch) {
                        case '0':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf] = 0x0;
                            break;
                        case '1':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x1;
                            break;
                        case '2':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x2;
                            break;
                        case '3':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x3;
                            break;
                        case '4':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x4;
                            break;
                        case '5':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x5;
                            break;
                        case '6':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x6;
                            break;
                        case '7':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x7;
                            break;
                        case '8':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x8;
                            break;
                        case '9':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0x9;
                            break;
                        case 'a':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0xa;
                            break;
                        case 'b':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0xb;
                            break;
                        case 'c':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0xc;
                            break;
                        case 'd':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0xd;
                            break;
                        case 'e':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0xe;
                            break;
                        case 'f':
                            valid_character = 1;
                            p1->registers[p1->memory[p1->programCounter] & 0xf]  = 0xf;
                            break;
                        default :
                            break;
                    }
                }
                p1->userinput = p1->registers[p1->memory[p1->programCounter] & 0xf];
                p1->userinput_flag = 1;
            }
            else if(p1->memory[p1->programCounter + 1] == 0x15) {
                //delay
                time_begin = clock();
                p1->delayFlag = 1;
                p1->delayTimer = p1->registers[p1->memory[p1->programCounter] & 0x0f];
                p1->time_spent_delay = 0;
                continue;
            }
            else if(p1->memory[p1->programCounter + 1] == 0x18) {
                //sound timer
                time_begin = clock();
                p1->soundFlag = 1;
                p1->soundTimer = p1->registers[p1->memory[p1->programCounter] & 0x0f];
                p1->time_spent_sound = 0;
            }
            else if(p1->memory[p1->programCounter + 1] == 0x1e) {
                // add to memory
                uint32_t tempAddress = p1->addressRegister + p1->registers[p1->memory[p1->programCounter] & 0x0f];
                p1->addressRegister += p1->registers[p1->memory[p1->programCounter] & 0x0f];

                if(tempAddress > 0xFFF) {
                    p1->registers[15] = 1;
                }
                else {
                    p1->registers[15] = 0;
                }
            
            }
            else if(p1->memory[p1->programCounter + 1] == 0x29) {
                //Sprites
                
            }
            else if(p1->memory[p1->programCounter + 1] == 0x33) {
                // BCD
                p1->memory[p1->addressRegister] = p1->registers[p1->memory[p1->programCounter] & 0x0f] / 100;
                p1->memory[p1->addressRegister + 1] = (p1->registers[p1->memory[p1->programCounter] & 0x0f] % 100) / 10 ;
                p1->memory[p1->addressRegister + 2] = (p1->registers[p1->memory[p1->programCounter] & 0x0f] % 10);
            }
            else if(p1->memory[p1->programCounter + 1] == 0x55) {
                // REGDUMP
                for (uint8_t i = 0; i <= (p1->memory[p1->programCounter] & 0x0f); i += 1) {
                    p1->memory[p1->addressRegister + i]  = p1->registers[i];
                    
                }
            }
            else if(p1->memory[p1->programCounter + 1] == 0x65) {
                // Regload
                for (uint8_t i = 0; i <= (p1->memory[p1->programCounter] & 0x0f); i += 1) {
                    p1->registers[i] = p1->memory[p1->addressRegister + i];
                }
            }
            else if(p1->memory[p1->programCounter + 1] == 0xff) {
                //Special DEBUG command to exit process because there no exist command in chip8 atm

                close_program(p1, randomData);
                exit(1);
            }
            else {
                printf("DEBUG: opcode at 0xfxxx instruction\n");
            }
        }
        else {
            perror("ERROR: Unintended behavio\n");
            close_program(p1, randomData);
            exit(1);
        }

        // Increment programer
        p1->programCounter += 2;

        if(p1->soundFlag != 0) {
            //Sound will play
            if(p1->time_spent_sound >= (double)p1->soundTimer/60) {
                printf("\a");
                p1->soundFlag = 0;
            }
            time_end = clock();
            p1->time_spent_sound += (double)(time_end - time_begin) / CLOCKS_PER_SEC;
            time_begin = time_end;
        }
        
    }

    close_program(p1, randomData);
    return 1;
}

chip8processor* init_chip8(void) {
    chip8processor* p1 = (chip8processor*)malloc(sizeof(chip8processor));
    memset(p1->memory,0,sizeof(uint8_t)* CHIP8_MEMORY_LIMIT);
    memset(p1->registers,0,sizeof(uint8_t)*16);
    // memset(p1->stack,0,sizeof(uint16_t)*CHIP8_STACK_SIZE);
    p1->stackSize = 0x0;
    p1->delayTimer = 0x0;
    p1->delayFlag = 0;
    p1->soundTimer = 0x0;
    p1->soundFlag = 0;
    p1->userinput = 0x0;
    p1->userinput_flag = 0;
    p1->programCounter = 0x200;
    p1->stackPointer = 0xea0;
    p1->addressRegister = 0x0;
    p1->time_spent_sound = 0.0;
    p1->time_spent_delay = 0.0;
    memset(p1->displayGrid, 0, sizeof(uint32_t)* DISPLAY_RESOLUTION_VERTICAL); //Zero out display
    return p1;
}

void destory_chip(chip8processor* p1) {
    free(p1);
}

void debug_chip8_state(chip8processor* p1, FILE* debug_File) {
    char str[50];
    int n;
    memset(str,'\0',sizeof(str));

    n = sprintf(str,"\nDEBUG: PRCESSOR STATE\n");
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"Reg: \n");
    fwrite(str,1,n,debug_File);
    for(int cnt = 0; cnt < 16; cnt += 1) {
       n = sprintf(str,"REG %d = %x\n", cnt, p1->registers[cnt]);
       fwrite(str,1,n,debug_File);
    }
    n = sprintf(str,"program counter: %x\n", p1->programCounter);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"Address Reg %x\n", p1->addressRegister);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"Current opcode %#0X%02X\n", p1->memory[p1->programCounter],p1->memory[p1->programCounter+1]);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"SoundTimer = %x\n",p1->soundTimer);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"DelayTimer = %x\n",p1->delayTimer);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"stack size = %d\n", p1->stackSize);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"stackpointer = %x\n", p1->stackPointer);
    fwrite(str,1,n,debug_File);
    for(int cnt = 0xea0; cnt < (0xea0 + CHIP8_STACK_SIZE); cnt+=2) {
       n = sprintf(str,"Stack %X = %#02X %02X\n", cnt,p1->memory[cnt], p1->memory[cnt +1]);
       fwrite(str,1,n,debug_File);
    }
    n = sprintf(str,"user input = %x\n", p1->userinput);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"user flag = %d\n", p1->userinput_flag);
    fwrite(str,1,n,debug_File);
    n = sprintf(str,"\n");
    fwrite(str,1,n,debug_File);
}

void view_program_memory(chip8processor* p1, FILE* debug_File) {
    char str[50];
    int n;
    memset(str,'\0',sizeof(str));
    for(unsigned int i = 0x200; (p1->memory[i] != 0) || (p1->memory[i+1] != 0); i += 2) {
        n = sprintf(str, "DEBUG: opcode at memory[%#5X]\t%#5X %02X\n", i, p1->memory[i],p1->memory[i+1]);
        fwrite(str,1,n,debug_File);
        memset(str,'\0',sizeof(str));
    }
}

void close_program(chip8processor* p1 , int randomData) {
    close(randomData);
    destory_chip(p1);
    endwin();
}

void draw_display(chip8processor* p1, WINDOW * win) {
    box(win, 0 , 0);
    wmove(win, 1, 1);
    for(int i = 0; i < DISPLAY_RESOLUTION_VERTICAL; i++) {
        for(int j = 0;j < DISPLAY_RESOLUTION_HORIZONTAL; i++) {
            if(((p1->displayGird[i] >> (63 - j) & 0x1) == 1){
                waddch(win,'#');
            }
            else {
                waddch(win,' ');
            }
        }
        wmove(win, i+2, 1);

    }
    wrefresh(win);
}