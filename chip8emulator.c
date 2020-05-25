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
#include <termios.h>

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
    uint8_t displayGrid[DISPLAY_RESOLUTION_HORIZONTAL][DISPLAY_RESOLUTION_VERTICAL];  //Hold display
} chip8processor;   

chip8processor* init_chip8(void);
void destory_chip(chip8processor* pi);
void debug_chip8_state(chip8processor* p1, FILE* debug_File) ;
void view_program_memory(chip8processor* p1, FILE* debug_File) ;
void close_program(chip8processor* pi , int randomData);
void draw_display(chip8processor* p1, WINDOW * win);
int kbhit(void);
void nonblock(int state);


int main(int argc, char *argv[]) {
    //Check too she if a filename was given
    if (argc != 2) {
        fprintf(stderr,"Error: useage: chip8emulator <filename>\n");
        return 1;
    }

    //Variables used
    FILE *programFile;
    char ch = 0;
    // long int fileSize;
    unsigned char * buffer = (unsigned char*) malloc (sizeof(char)*OPCODE_SIZE_INBYTES); //stores op code
    uint8_t randbits = 0;                                                         //Stores Ranbit from /dev/urandom
    clock_t time_begin = clock(), time_end = 0;                                       //Used to generate time delay
    double period_clock = 17;                                                 //Clock period is 16 milliseconds

    //Open file from arg
    programFile = fopen(argv[1], "r");

    //Check to see if the file can be opened
    if(programFile == NULL) {
        fprintf(stderr,"Error: Cannot open file or file not found\n");
        return 1;
    }

    //Create a chip8 processor 
    chip8processor* p1 = init_chip8();

    //start ncurses
	initscr();			        // Start curses mode 		
	

	keypad(stdscr, TRUE);		// Capture special key such f1 etc. 
    noecho();                   // Supress echo
    refresh();

    #ifdef DEBUG
        // If debug is declared, write to debug.log
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

    // Free buffer after use
    free(buffer);

    // Reset program counter to base and closes the file
    p1->programCounter = 0x200;
    fclose(programFile);

    // strong random using uradom
    int randomData = open("/dev/urandom", O_RDONLY);// 
    if (randomData < 0)
    {
        // something went wrong
        perror("ERROR: Cannot open /dev/urandom\n");
        exit(1);
    }   

    #ifdef DEBUG
        //If debug, Print out memory to debug log
        view_program_memory(p1, debug_File);
    #endif

    // Create ncurse UI for chip8
    WINDOW * win = newwin(DISPLAY_RESOLUTION_VERTICAL + 4, DISPLAY_RESOLUTION_HORIZONTAL + 2, 0, 0);
    cbreak();			        // Line buffering disabled
    // nodelay(stdscr, TRUE);
    box(win, 0 , 0);
    wrefresh(win);
    wmove(win, 1, 1);

    // Main loop
    while(0 == 0)
    {
        #ifdef DEBUG
            // If debug, print out chip state to debug.log
            debug_chip8_state(p1, debug_File);
        #endif

        // Parse current opcode
        uint8_t opcode_NN = p1->memory[p1->programCounter + 1];
        uint8_t opcode_N = p1->memory[p1->programCounter + 1] & 0x0f;
        uint8_t opcode_Vx = p1->memory[p1->programCounter] & 0xf;
        uint8_t opcode_Vy = p1->memory[p1->programCounter + 1] >> 4;
        uint8_t opcode_most_significant_bit = (p1->memory[p1->programCounter] & 0xf0) >> 4;
        uint8_t opcode_least_significant_bit = p1->memory[p1->programCounter + 1] & 0x0f;
        uint8_t opcode_upper_half = p1->memory[p1->programCounter];
        uint8_t opcode_lower_half = p1->memory[p1->programCounter + 1];

        uint16_t opcode_NNN = 0;
        opcode_NNN = opcode_Vx;
        opcode_NNN =  opcode_NNN << 8;
        opcode_NNN =  opcode_NNN | p1->memory[p1->programCounter + 1];

        // 60hz timer for chip
        time_end = clock();
        p1->time_spent_delay = (double)(time_end - time_begin) / CLOCKS_PER_SEC;    // Return a time in seconds
        p1->time_spent_delay /= 1000; // Convert seconds to milli seconds
        time_begin = time_end;
        double elapsed_time = period_clock - p1->time_spent_delay;
        if (elapsed_time > 0) {
            usleep(elapsed_time);
        }
        timeout(0);
        ch = getch();   //Get keybaord input
        switch(ch) 
        {
            case '0':
                p1->userinput = 0x0;
                break;
            case '1':
                p1->userinput  = 0x1;
                break;
            case '2':
                p1->userinput  = 0x2;
                break;
            case '3':
                p1->userinput  = 0x3;
                break;
            case '4':
                p1->userinput  = 0x4;
                break;
            case '5':
                p1->userinput  = 0x5;
                break;
            case '6':
                p1->userinput  = 0x6;
                break;
            case '7':
                p1->userinput  = 0x7;
                break;
            case '8':
                p1->userinput  = 0x8;
                break;
            case '9':
                p1->userinput  = 0x9;
                break;
            case 'a':
                p1->userinput  = 0xa;
                break;
            case 'b':
                p1->userinput  = 0xb;
                break;
            case 'c':
                p1->userinput  = 0xc;
                break;
            case 'd':
                p1->userinput  = 0xd;
                break;
            case 'e':
                p1->userinput  = 0xe;
                break;
            case 'f':
                p1->userinput  = 0xf;
                break;
            default :
                p1->userinput = 0xff;
                break;
        }
        wmove(win, 34, 5);
        waddch(win,(char)ch);
        wmove(win,1,1);
        wrefresh(win);

        flushinp();

        if(p1->delayFlag == 1) 
        {
            // If there was a delay trigger, loop until delay timer decrements to 0
            p1->delayTimer -= 1;
            if (p1->delayTimer == 0) {
                p1->delayFlag = 0;
                p1->programCounter += 2;
            }
            continue;
        }

        // If sound was called, keep playing buzz until timer ends
        if(p1->soundFlag == 1)
        {
            beep();
            p1->soundTimer -= 1;
            if(p1->soundTimer == 0)
            {
                p1->soundFlag = 0;
            }
        }
    
        if (opcode_most_significant_bit == 0x0)
        {
            // 0x0XXX instructions
            if(opcode_upper_half == 0x00 && 
                opcode_lower_half == 0xe0) 
            {
                // call clear screen
                 for(int x = 0; x < DISPLAY_RESOLUTION_HORIZONTAL; x ++)
                {
                    for(int y = 0; y < DISPLAY_RESOLUTION_VERTICAL; y ++)
                    {
                        p1->displayGrid[x][y] = 0;
                    }
                }
                werase(win);
                box(win, 0 , 0);
                wmove(win, 1, 1);
                wrefresh(win);
            }
            else if (opcode_upper_half == 0x00 && 
                opcode_lower_half == 0xee)
            {
                // Return from subroutine
                if(p1->stackSize == 0) {
                    // Nothing on stack to return from
                    perror("ERROR: Stack underflowed into memory bounds\n");
                    close_program(p1, randomData);
                    exit(1);
                }
                p1->stackPointer -= 2;

                // Grab the address on top of the stack
                uint16_t tempAddress = 0;
                tempAddress = p1->memory[p1->stackPointer] & 0x0f;
                tempAddress = tempAddress << 8;
                tempAddress = tempAddress | p1->memory[p1->stackPointer + 1];

                // Decrease the stack size and return the address as the new program counter
                p1->stackSize -= 1;
                p1->programCounter = tempAddress;
                continue;
            }
            else 
            {
                // call subroutine RCA, this can go into below 0x200 region
                // Current doesn't work as there no documentation of this region
                // Besides font stored here
            
                // Check stack size and ensure there is enough room in stack
                if(p1->stackSize > CHIP8_STACK_SIZE) {
                    perror("ERROR: Stack overflowed into memory bounds\n");
                    close_program(p1, randomData);
                    exit(1);
                }

                //Store program counter into stack
                p1->memory[p1->stackPointer] = ((p1->programCounter + 2) & 0xf00) >> 8;    
                p1->memory[p1->stackPointer + 1] = (p1->programCounter & 0xff);
                p1->stackPointer += 2;
                p1->stackSize += 1;

                if(opcode_NNN > 0xEA0) {
                    perror("ERROR: Program ran out of memory bounds\n");
                    close_program(p1, randomData);
                    exit(1);
                }

                p1->programCounter = opcode_NNN;
                continue;
            }
        }
        else if (opcode_most_significant_bit == 0x1) 
        {
            // 0x1XXX opcode, JMP command 

            if(opcode_NNN < 0x200 || opcode_NNN > 0xEA0) 
            {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            p1->programCounter = opcode_NNN;
            continue;
        }
        else if (opcode_most_significant_bit == 0x2) 
        {
            // 0x2XXX opcode, call subroutine
            
            // Check if stack is full
            if(p1->stackSize > CHIP8_STACK_SIZE)
            {
                perror("ERROR: Stack overflowed into memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            //Grab the current program counter, advance it by 1 step and store it in the stack.
            p1->memory[p1->stackPointer] = ((p1->programCounter + 2) & 0xf00) >> 8;    
            p1->memory[p1->stackPointer + 1] = ((p1->programCounter + 2) & 0xff);
            p1->stackPointer += 2;
            p1->stackSize += 1;

            if(opcode_NNN < 0x200 || opcode_NNN > 0xEA0) 
            {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            p1->programCounter = opcode_NNN;
            continue;
        }
        else if (opcode_most_significant_bit == 0x3) 
        {
            // 0x3NNN, Skip if Vx == NN
            if(p1->registers[opcode_Vx] == opcode_NN) 
            {
                p1->programCounter += 2;
            }
        }
        else if (opcode_most_significant_bit == 0x4) 
        {
            // Skip if Vx != NN
            if(p1->registers[opcode_Vx] != opcode_NN) {
                p1->programCounter += 2;
            }
        }
        else if (opcode_most_significant_bit == 0x5) {
            // Skip if Vx == Vy
            if(p1->registers[opcode_Vx] == p1->registers[opcode_Vy] 
                && (p1->memory[p1->programCounter + 1] & 0x0f) == 0) 
            {
                p1->programCounter += 2;
            }
        }
        else if (opcode_most_significant_bit == 0x6) 
        {
            //  Vx = NN
            p1->registers[opcode_Vx] = opcode_NN;
        }
        else if (opcode_most_significant_bit == 0x7) 
        {
            // Vx += NN
            p1->registers[opcode_Vx] += opcode_NN;
        }
        else if (opcode_most_significant_bit == 0x8) 
        {
            // 0x8XXX commands subsection
            if(opcode_least_significant_bit == 0x0) 
            {
                // Vx = Vy
                p1->registers[opcode_Vx] = p1->registers[opcode_Vy];
            }
            else if (opcode_least_significant_bit == 0x1) 
            {
                // Command Vx = Vx | Vy
                p1->registers[opcode_Vx] = p1->registers[opcode_Vx] | p1->registers[opcode_Vy];
            }
            else if (opcode_least_significant_bit == 0x2) 
            {
                // Commmand 	Vx=Vx & Vy
                p1->registers[opcode_Vx] = p1->registers[opcode_Vx] & p1->registers[opcode_Vy];
            }
            else if (opcode_least_significant_bit == 0x3) 
            {
                // Commmand 	Vx=Vx^Vy
                p1->registers[opcode_Vx] = p1->registers[opcode_Vx] ^ p1->registers[opcode_Vy];
            }
            else if (opcode_least_significant_bit == 0x4) {
                // Command Vx += Vy
                // Check if there will be a carry over
                uint16_t check_flag = (uint16_t)p1->registers[opcode_Vx] + (uint16_t)p1->registers[opcode_Vy];
                if (check_flag > 0xff) {
                    p1->registers[0xf] = 1;
                }
                else 
                {
                    p1->registers[0xf] = 0;
                }

                p1->registers[opcode_Vx] += p1->registers[opcode_Vy];
            }
            else if (opcode_least_significant_bit == 0x5) 
            {
                // Command Vx -= Vy
                // Check to see if there will be a negative value
                int16_t check_flag = (int16_t)p1->registers[opcode_Vx] - (int16_t)p1->registers[opcode_Vy];
                if (check_flag < 0) {
                     p1->registers[0xf] = 1;
                }
                else
                {
                    p1->registers[0xf] = 0;
                }
                p1->registers[opcode_Vx] -= p1->registers[opcode_Vy];
            }
            else if (opcode_least_significant_bit == 0x6) 
            {
                // Command Vx >>= 1
                p1->registers[0xf] = p1->registers[opcode_Vx] & 0x1;
                p1->registers[opcode_Vx] = p1->registers[opcode_Vx] >> 1;
            }
            else if (opcode_least_significant_bit == 0x7)
            {
                // Command Vx = Vy - Vx
                // Checks to see if there will be a underflow
                int16_t check_flag = (int16_t)p1->registers[opcode_Vy] - p1->registers[opcode_Vx] ;
                if (check_flag < 0) {
                     p1->registers[0xf] = 1;
                }
                else
                {
                    p1->registers[0xf] = 0;
                }

                p1->registers[opcode_Vx] = p1->registers[opcode_Vy] - p1->registers[opcode_Vx];
            }
            else if (opcode_least_significant_bit == 0xe) 
            {
                // Command Vx <<= 1
                p1->registers[0xf] = (p1->registers[opcode_Vx] & 0x80) >> 7;
                p1->registers[opcode_Vx] = p1->registers[opcode_Vx] << 1;
            }
        }
        else if (opcode_most_significant_bit == 0x9) 
        {
            // 0x9XXX
            if(opcode_least_significant_bit == 0x0) 
            {
                //Skip if Vx != Vy
                if(p1->registers[opcode_Vx] != p1->registers[opcode_Vy])
                {
                    p1->programCounter += 2;
                }
            }
        }
        else if (opcode_most_significant_bit == 0xa) 
        {
            // I = NNNN
            if(opcode_NNN < 0x200 || opcode_NNN > 0xEA0) 
            {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }
            // Store NNN into address register
            p1->addressRegister = opcode_NNN;
        }
        else if (opcode_most_significant_bit == 0xb) 
        {
            // PC = V0 + NNNN
            // Stores temp_Address to jump to
            uint16_t tempAddress = opcode_NNN;

            // Add V0 to NNNN
            tempAddress += p1->registers[0];

            //Check if Program counter is in restricted part of memory
            if(tempAddress < 0x200 || tempAddress > 0xEA0) 
            {
                perror("ERROR: Program ran out of memory bounds\n");
                close_program(p1, randomData);
                exit(1);
            }

            p1->programCounter = tempAddress;
            continue;
        }
        else if (opcode_most_significant_bit == 0xc) 
        {
            // Vx = Random() & NN
            read(randomData, &randbits, sizeof(randbits));
            p1->registers[opcode_Vx] = randbits & opcode_NN; 

        }
        else if (opcode_most_significant_bit == 0xd) 
        {
            //Draw DXYN
            uint8_t height = opcode_N; // Grab N from opcode
            uint16_t i_temp = p1->addressRegister;
            uint8_t xPixel = p1->registers[opcode_Vx];
            uint8_t yPixel = p1->registers[opcode_Vy];
            p1->registers[0xf] = 0;

            for(uint8_t y = 0; y < height && yPixel + y < DISPLAY_RESOLUTION_VERTICAL; y++) 
            {
                for(uint8_t x = 0; x < 8 && x + xPixel < DISPLAY_RESOLUTION_HORIZONTAL ; x++)
                {
                    if(p1->displayGrid[xPixel + x][yPixel + y] == (0x1 & ( p1->memory[i_temp + (2*y)] >> (7 - x))) && p1->registers[0xf] == 0)
                    {
                        // Check to see if a pixel will flip
                        p1->registers[0xf] = 1;
                    }
                    p1->displayGrid[xPixel + x][yPixel + y] = p1->displayGrid[xPixel + x][yPixel + y] ^ (0x1 & ( p1->memory[i_temp + (2*y)] >> (7 - x)));
                }
            }
            draw_display( p1,  win);
        }
        else if (opcode_most_significant_bit == 0xe) 
        {
            if(opcode_lower_half == 0x9e) 
            {
                // Skip if Vx == key()
                if(p1->registers[opcode_Vx] == p1->userinput ) 
                {
                    p1->programCounter += 2;
                }
            }
            else if(opcode_lower_half == 0xa1)
            {
                // Skip if Vx != Key()
                if(p1->registers[opcode_Vx] != p1->userinput) 
                {
                    p1->programCounter += 2;
                }
            }
            
        }
        else if (opcode_most_significant_bit == 0xf) 
        {
            // 0xfXXX
            if(opcode_lower_half == 0x07) 
            {
                // Vx = current delay value
                p1->registers[opcode_Vx] = p1->delayTimer;
            }
            else if(opcode_lower_half == 0x0a) 
            {
                // Get key press and store it in Vx, stop program until valid key is press
                // Input is in hex
                int valid_character = 0;
                timeout(-1);
                
                while(valid_character == 0) 
                {
                    ch = getch();   //Get keybaord input

                    switch(ch) 
                    {
                        case '0':
                            valid_character = 1;
                            p1->registers[opcode_Vx] = 0x0;
                            break;
                        case '1':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x1;
                            break;
                        case '2':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x2;
                            break;
                        case '3':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x3;
                            break;
                        case '4':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x4;
                            break;
                        case '5':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x5;
                            break;
                        case '6':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x6;
                            break;
                        case '7':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x7;
                            break;
                        case '8':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x8;
                            break;
                        case '9':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0x9;
                            break;
                        case 'a':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0xa;
                            break;
                        case 'b':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0xb;
                            break;
                        case 'c':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0xc;
                            break;
                        case 'd':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0xd;
                            break;
                        case 'e':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0xe;
                            break;
                        case 'f':
                            valid_character = 1;
                            p1->registers[opcode_Vx]  = 0xf;
                            break;
                        default :
                            break;
                    }
                }
                keypad(stdscr, TRUE);
                p1->userinput = p1->registers[opcode_Vx];
            }
            else if(opcode_lower_half == 0x15) 
            {
                // Set a delay based on the value of Vx
                time_begin = clock();
                p1->delayFlag = 1;
                p1->delayTimer = p1->registers[opcode_Vx];
                p1->time_spent_delay = 0;
                continue;
            }
            else if(opcode_lower_half == 0x18) 
            {
                // Set a sound to be played after a set amount of time
                // Value to be delay is Vx
                time_begin = clock();
                p1->soundFlag = 1;
                p1->soundTimer = p1->registers[opcode_Vx];
                p1->time_spent_sound = 0;
            }
            else if(opcode_lower_half == 0x1e)
            {
                // I += Vx

                // If there is a overflow, then set VF = 1
                uint32_t tempAddress = p1->addressRegister + p1->registers[opcode_Vx];
                if(tempAddress > 0xFFF) {
                    p1->registers[15] = 1;
                }
                else {
                    p1->registers[15] = 0;
                }

                p1->addressRegister += p1->registers[opcode_Vx];
            }
            else if(opcode_lower_half == 0x29) {
                // load sprite to Iregister
                //Register are 8 bit, we only want the last 4bit (hex) to represent the number being loaded
                uint16_t sprite_Location = (p1->registers[opcode_Vx] & 0xf) * 10;

                for(uint8_t i = 0; i < 10; i++) {
                    p1->memory[p1->addressRegister + i] = p1->memory[sprite_Location + i];
                }
                
            }
            else if(opcode_lower_half == 0x33) {
                // BCD
                p1->memory[p1->addressRegister] = p1->registers[opcode_Vx] / 100;
                p1->memory[p1->addressRegister + 1] = (p1->registers[opcode_Vx] % 100) / 10 ;
                p1->memory[p1->addressRegister + 2] = (p1->registers[opcode_Vx] % 10);
            }
            else if(opcode_lower_half== 0x55) {
                // REGDUMP
                for (uint8_t i = 0; i <= opcode_Vx; i += 1) {
                    // Load V0 to 
                    p1->memory[p1->addressRegister + i]  = p1->registers[i];
                }
            }
            else if(opcode_lower_half == 0x65) {
                // Regload
                for (uint8_t i = 0; i <= (opcode_Vx); i += 1) {
                    p1->registers[i] = p1->memory[p1->addressRegister + i];
                }
            }
            else if(opcode_lower_half == 0xff) {
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
    }

    close_program(p1, randomData);
    return 1;
}

chip8processor* init_chip8(void) {
    // Create processor 
    chip8processor* p1 = (chip8processor*)malloc(sizeof(chip8processor));

    //Zero out the processor to start from an empty state
    memset(p1->memory,0,sizeof(uint8_t)* CHIP8_MEMORY_LIMIT);
    memset(p1->registers,0,sizeof(uint8_t)*16);
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
    for(int x = 0; x < DISPLAY_RESOLUTION_HORIZONTAL; x ++)
    {
        for(int y = 0; y < DISPLAY_RESOLUTION_VERTICAL; y ++)
        {
            p1->displayGrid[x][y] = 0;
        }
    }

    // Load Font sprites into chip8
    // Load 0x0 sprites
    p1->memory[0x000] = 0xf;
    p1->memory[0x002] = 0x9;
    p1->memory[0x0004] = 0x9;
    p1->memory[0x0006] = 0x9;
    p1->memory[0x0008] = 0xF;

    // Load 0x1 sprite
    p1->memory[0x00a] = 0x2;
    p1->memory[0x00c] = 0x6;
    p1->memory[0x00e] = 0x2;
    p1->memory[0x010] = 0x2;
    p1->memory[0x012] = 0x7;

    // Load 0x2 sprite
    p1->memory[0x014] = 0xf;
    p1->memory[0x016] = 0x1;
    p1->memory[0x018] = 0xf;
    p1->memory[0x01a] = 0x8;
    p1->memory[0x01c] = 0xf;

    // Load 0x3 sprite
    p1->memory[0x01e] = 0xf;
    p1->memory[0x020] = 0x1;
    p1->memory[0x022] = 0xf;
    p1->memory[0x024] = 0x1;
    p1->memory[0x026] = 0xf;

    // Load 0x4 sprite
    p1->memory[0x028] = 0x9;
    p1->memory[0x02a] = 0x9;
    p1->memory[0x02c] = 0xf;
    p1->memory[0x02e] = 0x1;
    p1->memory[0x030] = 0x1;

    // Load 5 sprite
    p1->memory[0x032] = 0xf;
    p1->memory[0x034] = 0x8;
    p1->memory[0x036] = 0xf;
    p1->memory[0x038] = 0x1;
    p1->memory[0x03a] = 0xf;

    // Load 0x6 sprite
    p1->memory[0x03c] = 0xf;
    p1->memory[0x03e] = 0x8;
    p1->memory[0x040] = 0xf;
    p1->memory[0x042] = 0x9;
    p1->memory[0x044] = 0xf;

    // Load 0x7 sprite
    p1->memory[0x046] = 0xf;
    p1->memory[0x048] = 0x1;
    p1->memory[0x04a] = 0x2;
    p1->memory[0x04c] = 0x4;
    p1->memory[0x04e] = 0x4;

    // Load 0x8 sprite
    p1->memory[0x050] = 0xf;
    p1->memory[0x052] = 0x9;
    p1->memory[0x054] = 0xf;
    p1->memory[0x056] = 0x9;
    p1->memory[0x058] = 0xf;

    // Load 0x9 sprite
    p1->memory[0x05a] = 0xf;
    p1->memory[0x05c] = 0x9;
    p1->memory[0x05e] = 0xf;
    p1->memory[0x060] = 0x1;
    p1->memory[0x062] = 0xf;

    // Load 0xA sprite
    p1->memory[0x064] = 0xf;
    p1->memory[0x066] = 0x9;
    p1->memory[0x068] = 0xf;
    p1->memory[0x06a] = 0x9;
    p1->memory[0x06c] = 0x9;

    // Load 0xB sprite
    p1->memory[0x06e] = 0xe;
    p1->memory[0x070] = 0x9;
    p1->memory[0x072] = 0xe;
    p1->memory[0x074] = 0x9;
    p1->memory[0x076] = 0xe;

    // Load 0xC sprite
    p1->memory[0x0078] = 0xf;
    p1->memory[0x007a] = 0x8;
    p1->memory[0x007c] = 0x8;
    p1->memory[0x007e] = 0x8;
    p1->memory[0x0080] = 0xf;

    // Load 0xD sprite
    p1->memory[0x082] = 0xe;
    p1->memory[0x084] = 0x9;
    p1->memory[0x086] = 0x9;
    p1->memory[0x088] = 0x9;
    p1->memory[0x08a] = 0xe;

    // Load 0xE sprite
    p1->memory[0x08c] = 0xf;
    p1->memory[0x08e] = 0x8;
    p1->memory[0x090] = 0xf;
    p1->memory[0x092] = 0x8;
    p1->memory[0x094] = 0xf;

    // Load 0xF sprite
    p1->memory[0x096] = 0xf;
    p1->memory[0x098] = 0x8;
    p1->memory[0x09a] = 0xf;
    p1->memory[0x09c] = 0x8;
    p1->memory[0x09e] = 0x8;
    return p1;
}

void destory_chip(chip8processor* p1) {
    // Free memory
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
    char str[100];
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
    werase(win);
    box(win, 0 , 0);
    wmove(win, 1, 1);
    int y_axis = 1;
    for(int y = 0; y < DISPLAY_RESOLUTION_VERTICAL; y++) 
    {
        for(int x = 0; x < DISPLAY_RESOLUTION_HORIZONTAL; x++) 
        {
            if(p1->displayGrid[x][y]  == 1)
            {
                waddch(win,'#');
            }
            else {
                waddch(win,' ');
            }
        }
        y_axis += 1;
        wmove(win, y_axis, 1);
    }
    wrefresh(win);
}

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state)
{
    struct termios ttystate;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (state==1)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==0)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

}