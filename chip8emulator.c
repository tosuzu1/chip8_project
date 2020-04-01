#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DISPLAY_RESOLUTION_HORIZONTAL 64
#define DISPLAY_RESOLUTION_VERTICAL 32
#define OPCODE_SIZE_INBYTES 2

struct chip8processor { 
    uint16_t memory[4096]; 
    uint8_t registers[16];
    uint16_t stack[12];
    uint8_t stackSize;
    uint8_t delayTimer;
    uint8_t soundTimer;
    unsigned char userinput;
    uint16_t programCounter;
    uint8_t stackPointer;
    uint8_t addressPointer;
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
    uint16_t opcode;
    size_t result;

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
    memset(p1.memory,0,sizeof(uint16_t)*4096);
    memset(p1.registers,0,sizeof(uint8_t)*16);
    memset(p1.stack,0,sizeof(uint8_t)*12);
    p1.stackSize = 0;
    p1.delayTimer = 0;
    p1.soundTimer = 0;
    p1.userinput = 0;
    p1.programCounter = 0x200;
    p1.stackPointer = 0;
    p1.addressPointer = 0;

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

        // Convert buffer to a integer value and load it into the chip8 memory
        opcode = 0;
        opcode = buffer[0];
        opcode = opcode << 8;
        opcode = opcode | buffer[1];

        p1.memory[p1.programCounter] = opcode;
        p1.programCounter++;

        // If the program being loaded goes pass limit of the memory, throw a error;
        if(p1.programCounter > 0xF00) {
            perror("Error: Program being loaded excedes the memory of the processor\n");
        }
    }

    // Reset program counter to base
    p1.programCounter = 0x200;

    //Debug
    for(unsigned int i = p1.programCounter; p1.memory[i] != 0; i++) {
        printf("DEBUG: opcode at memory[%#5X]\t%#6X\n", i, p1.memory[i]);
    }


    return 1;
}