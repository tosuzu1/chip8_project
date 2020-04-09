#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DISPLAY_RESOLUTION_HORIZONTAL 64
#define DISPLAY_RESOLUTION_VERTICAL 32
#define OPCODE_SIZE_INBYTES 2
#define CHIP8_MEMORY_LIMIT 4096
#define CHIP8_STACK_SIZE 16

typedef struct chip8processor { 
    uint8_t memory[CHIP8_MEMORY_LIMIT]; 
    uint8_t registers[16];
    // uint16_t stack[CHIP8_STACK_SIZE];        //Stack is now inside of memory
    uint16_t stackSize;
    uint8_t delayTimer;
    uint8_t soundTimer;
    unsigned char userinput;
    uint16_t programCounter;
    uint16_t stackPointer;
    uint16_t addressRegister;
} chip8processor;   

chip8processor* init_chip8(void) {
    chip8processor* p1 = (chip8processor*)malloc(sizeof(chip8processor));
    memset(p1->memory,0,sizeof(uint8_t)* CHIP8_MEMORY_LIMIT);
    memset(p1->registers,0,sizeof(uint8_t)*16);
    // memset(p1->stack,0,sizeof(uint16_t)*CHIP8_STACK_SIZE);
    p1->stackSize = 0;
    p1->delayTimer = 0;
    p1->soundTimer = 0;
    p1->userinput = 0;
    p1->programCounter = 0x200;
    p1->stackPointer = 0xea0;
    p1->addressRegister = 0;
    return p1;
}

void debug_chip8_state(chip8processor* p1) {
    printf("\nDEBUG: PRCESSOR STATE\n");
    printf("Reg: \n");
    for(int cnt = 0; cnt < 16; cnt += 1) {
        printf("REG %d = %x\n", cnt, p1->registers[cnt]);
    }
    printf("program counter: %x\n", p1->programCounter);
    printf("Address Reg %x\n", p1->addressRegister);
    printf("\n");
}

int main(int argc, char *argv[]) {
    //Check too she if a filename was given
    if (argc != 2) {
        fprintf(stderr,"Error: useage: chip8emulator <filename>\n");
        return 1;
    }

    //Variables used
    FILE *programFile;
    long int fileSize;
    unsigned char * buffer = (char*) malloc (sizeof(char)*OPCODE_SIZE_INBYTES); //stores op code
    size_t result;
    srand(time(0));     // Used for random number generator but is vunable to timing attacks
    // TODO use /dev/random         this makes it non portal to non-unix OS so look into it

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

    chip8processor* p1 = init_chip8();

    //Load rom into ram
    while(!feof(programFile)) {
        //set the buffer to all zeros
        memset(buffer,0,OPCODE_SIZE_INBYTES);

        //Get the 2 byte opcode and put it into the buffer
        result = fread(buffer,1,OPCODE_SIZE_INBYTES, programFile);

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
        }
    }

    // Reset program counter to base and closes the file
    p1->programCounter = 0x200;
    fclose(programFile);

    //Debug
    for(unsigned int i = p1->programCounter; p1->memory[i] != 0; i += 2) {
        printf("DEBUG: opcode at memory[%#5X]\t%#5X %02X\n", i, p1->memory[i],p1->memory[i+1]);
    }

    while(0 == 0) {
        debug_chip8_state(p1);

        if ((p1->memory[p1->programCounter] >> 4) == 0x0){
            //0x00 intructsions
            if((p1->memory[p1->programCounter] & 0xf) == 0x0 && 
            (p1->memory[p1->programCounter + 1] >> 4) == 0x0) {
                //call clear screen
            }
            else if ((p1->memory[p1->programCounter] & 0xf) == 0 && 
            (p1->memory[p1->programCounter + 1] >> 4) == 0xe) {
                if(p1->stackSize == 0) {
                    perror("ERROR: Stack underflowed into memory bounds\n");
                }

                uint16_t tempAddress = 0;
                tempAddress = p1->memory[p1->stackPointer] & 0x0f;
                tempAddress = tempAddress << 16;
                tempAddress = tempAddress | p1->memory[p1->stackPointer + 1];

                p1->stackPointer -= 2;
                p1->stackSize -= 1;

                p1->programCounter = tempAddress;
                continue;
            }
            else {
                // call subroutine RCA, this can go into below 0x200 region
            
                //Store program counter into stack
                if(p1->stackSize > CHIP8_STACK_SIZE) {
                    perror("ERROR: Stack overflowed into memory bounds\n");
                }
                p1->memory[p1->stackPointer] = ((p1->programCounter + 2) & 0xf00) >> 8;    //grab the programcounter's first 4 bits
                p1->memory[p1->stackPointer + 1] = (p1->programCounter & 0xff);
                p1->stackPointer += 2;
                p1->stackSize += 1;

                uint16_t tempAddress = 0;
                tempAddress = p1->memory[p1->programCounter] & 0x0f;
                tempAddress = tempAddress << 16;
                tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

                if(tempAddress > 0xEA0) {
                    perror("ERROR: Program ran out of memory bounds\n");
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
            tempAddress = tempAddress << 16;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
            }

            p1->programCounter = tempAddress;
            continue;
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x2) {
            // call subroutine
            
            //Store program counter into stack
            if(p1->stackSize > CHIP8_STACK_SIZE) {
                perror("ERROR: Stack overflowed into memory bounds\n");
            }
            p1->memory[p1->stackPointer] = ((p1->programCounter + 2) & 0xf00) >> 8;    //grab the programcounter's first 4 bits
            p1->memory[p1->stackPointer + 1] = (p1->programCounter & 0xff);
            p1->stackPointer += 2;
            p1->stackSize += 1;

            uint16_t tempAddress = 0;
            tempAddress = p1->memory[p1->programCounter] & 0x0f;
            tempAddress = tempAddress << 16;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
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
            if(p1->registers[p1->memory[p1->programCounter] & 0xf] == p1->registers[p1->memory[p1->programCounter + 1] & 0xf0] &&
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
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter + 1] & 0xf0];
            }
            // Command Vx = Vx | Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x1) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter + 1] & 0xf0] | p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Commmand 	Vx=Vx&Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x2) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter + 1] & 0xf0] & p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Commmand 	Vx=Vx^Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x3) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter + 1] & 0xf0] ^ p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Command Vx += Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x4) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] += p1->registers[p1->memory[p1->programCounter + 1] & 0xf0];
            }
            // Command Vx -= Vy
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x5) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] -= p1->registers[p1->memory[p1->programCounter + 1] & 0xf0];
            }
            // Command Vx >>= 1
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x6) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter] & 0x0f] >> 1;
            }
            // Command Vx = Vy - Vx
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0x7) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter + 1] & 0xf0] - p1->registers[p1->memory[p1->programCounter] & 0x0f];
            }
            // Command Vx <<= 1
            else if ((p1->memory[p1->programCounter + 1] & 0x0f) == 0xe) {
                p1->registers[p1->memory[p1->programCounter] & 0x0f] = p1->registers[p1->memory[p1->programCounter] & 0x0f] << 1;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0x9) {
            // Skip if Vx != Vy
            if(p1->registers[p1->memory[p1->programCounter] & 0xf] != p1->registers[p1->memory[p1->programCounter + 1] & 0xf0] &&
            (p1->memory[p1->programCounter + 1] & 0x0f) == 0) {
                p1->programCounter += 2;
            }
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xa) {
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1->memory[p1->programCounter] & 0x0f;
            tempAddress = tempAddress << 16;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
            }

            p1->addressRegister = tempAddress;
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xb) {
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1->memory[p1->programCounter] & 0x0f;
            tempAddress = tempAddress << 16;
            tempAddress = tempAddress | p1->memory[p1->programCounter + 1]; 

            tempAddress += p1->registers[0];
            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
            }

            p1->programCounter = tempAddress;
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xc) {
            p1->registers[p1->memory[p1->programCounter] | 0xf ] = (rand() % 255) & p1->memory[p1->programCounter + 1];
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xd) {
            //Draw
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xe) {
            //get key press
            
        }
        else if ((p1->memory[p1->programCounter] >> 4) == 0xf) {
            if(p1->memory[p1->programCounter + 1] == 0x07) {
                p1->registers[p1->memory[p1->programCounter] & 0xf] = p1->delayTimer;
            }
            else if(p1->memory[p1->programCounter + 1] == 0x0a) {
                
            }
            else if(p1->memory[p1->programCounter + 1] == 0x15) {
                //delay
            }
            else if(p1->memory[p1->programCounter + 1] == 0x18) {
                //sound timer
            }
            else if(p1->memory[p1->programCounter + 1] == 0x1e) {
                p1->addressRegister += p1->registers[p1->memory[p1->programCounter] & 0x0f];

                uint16_t tempAddress = p1->addressRegister + p1->registers[p1->memory[p1->programCounter] & 0x0f];

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
                exit(1);
            }
            else {
                printf("DEBUG: opcode at 0xfxxx instruction\n");
            }
        }
        else {
            perror("ERROR: Unintended behavio\n");
        }

        // Increment programer
        p1->programCounter += 2;
        
    }

    return 1;
}