#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DISPLAY_RESOLUTION_HORIZONTAL 64
#define DISPLAY_RESOLUTION_VERTICAL 32
#define OPCODE_SIZE_INBYTES 2
#define CHIP8_MEMORY_LIMIT 4096
#define CHIP8_STACK_SIZE 12

struct chip8processor { 
    uint8_t memory[CHIP8_MEMORY_LIMIT]; 
    uint8_t registers[16];
    uint16_t stack[CHIP8_STACK_SIZE];
    uint16_t stackSize;
    uint8_t delayTimer;
    uint8_t soundTimer;
    unsigned char userinput;
    uint16_t programCounter;
    uint16_t stackPointer;
    uint16_t addressRegister;
} chip8processor;   

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

    //Initialize chip8 processor
    struct chip8processor p1;
    memset(p1.memory,0,sizeof(uint8_t)* CHIP8_MEMORY_LIMIT);
    memset(p1.registers,0,sizeof(uint8_t)*16);
    memset(p1.stack,0,sizeof(uint16_t)*CHIP8_STACK_SIZE);
    p1.stackSize = 0;
    p1.delayTimer = 0;
    p1.soundTimer = 0;
    p1.userinput = 0;
    p1.programCounter = 0x200;
    p1.stackPointer = 0;
    p1.addressRegister = 0;

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

        p1.memory[p1.programCounter] = buffer[0];
        p1.programCounter++;
        p1.memory[p1.programCounter] = buffer[1];
        p1.programCounter++;

        // If the program being loaded goes pass limit of the memory, throw a error;
        if(p1.programCounter > 0xF00) {
            perror("Error: Program being loaded excedes the memory of the processor\n");
        }
    }

    // Reset program counter to base and closes the file
    p1.programCounter = 0x200;
    fclose(programFile);

    //Debug
    for(unsigned int i = p1.programCounter; p1.memory[i] != 0; i += 2) {
        printf("DEBUG: opcode at memory[%#5X]\t%#5X %02X\n", i, p1.memory[i],p1.memory[i+1]);
    }

    while(0) {
        if ((p1.memory[p1.programCounter] >> 4) == 0x0){
            //0x00 intructsions
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x1) {
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1.memory[p1.programCounter] & 0x0f;
            tempAddress = tempAddress << 16;
            tempAddress = tempAddress | p1.memory[p1.programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
            }

            p1.programCounter = tempAddress;
            continue;
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x2) {
            // call subroutine
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x3) {
            // Skip if Vx == NN
            if(p1.registers[p1.memory[p1.programCounter] & 0xf] == p1.memory[p1.programCounter + 1]) {
                p1.programCounter += 2;
            }
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x4) {
            // Skip if Vx != NN
            if(p1.registers[p1.memory[p1.programCounter] & 0xf] != p1.memory[p1.programCounter + 1]) {
                p1.programCounter += 2;
            }
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x5) {
            // Skip if Vx == Vy
            if(p1.registers[p1.memory[p1.programCounter] & 0xf] == p1.registers[p1.memory[p1.programCounter + 1] & 0xf0] &&
            (p1.memory[p1.programCounter + 1] & 0x0f) == 0) {
                p1.programCounter += 2;
            }
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x6) {
           //  Vx = NN
            p1.registers[p1.memory[p1.programCounter] & 0xf] = p1.memory[p1.programCounter + 1];
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x7) {
            // Vx += NN
            p1.registers[p1.memory[p1.programCounter] & 0xf] += p1.memory[p1.programCounter + 1];
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x8) {
            
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0x9) {
            // Skip if Vx != Vy
            if(p1.registers[p1.memory[p1.programCounter] & 0xf] != p1.registers[p1.memory[p1.programCounter + 1] & 0xf0] &&
            (p1.memory[p1.programCounter + 1] & 0x0f) == 0) {
                p1.programCounter += 2;
            }
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0xa) {
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1.memory[p1.programCounter] & 0x0f;
            tempAddress = tempAddress << 16;
            tempAddress = tempAddress | p1.memory[p1.programCounter + 1]; 

            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
            }

            p1.addressRegister = tempAddress;
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0xb) {
            //Convert to a single variable
            uint16_t tempAddress = 0;
            tempAddress = p1.memory[p1.programCounter] & 0x0f;
            tempAddress = tempAddress << 16;
            tempAddress = tempAddress | p1.memory[p1.programCounter + 1]; 

            tempAddress += p1.registers[0];
            if(tempAddress < 0x200 || tempAddress > 0xEA0) {
                perror("ERROR: Program ran out of memory bounds\n");
            }

            p1.programCounter = tempAddress;
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0xc) {
            p1.registers[p1.memory[p1.programCounter] | 0xf ] = (rand() % 255) & p1.memory[p1.programCounter + 1];
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0xd) {
            //Draw
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0xe) {
            //get key press
            
        }
        else if ((p1.memory[p1.programCounter] >> 4) == 0xf) {
            if(p1.memory[p1.programCounter + 1] == 0x07) {
                p1.registers[p1.memory[p1.programCounter] & 0xf] = p1.delayTimer;
            }
            else if(p1.memory[p1.programCounter + 1] == 0x0a) {
                
            }
            else if(p1.memory[p1.programCounter + 1] == 0x15) {
                //delay
            }
            else if(p1.memory[p1.programCounter + 1] == 0x18) {
                //sound timer
            }
            else if(p1.memory[p1.programCounter + 1] == 0x1e) {

            
            }
            else if(p1.memory[p1.programCounter + 1] == 0x29) {
                //Sprites
            }
            else if(p1.memory[p1.programCounter + 1] == 0x33) {
                // BCD
            }
            else if(p1.memory[p1.programCounter + 1] == 0x55) {
                // REGDUMP
            }
            else if(p1.memory[p1.programCounter + 1] == 0x65) {
                // Regload
            }
            else {
                printf("DEBUG: opcode at 0xfxxx instruction\n");
            }
        }
        else {
            perror("ERROR: Unintended behavio\n");
        }

        // Increment programer
        p1.programCounter += 2;
    }

    return 1;
}