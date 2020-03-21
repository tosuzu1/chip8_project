#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define OPCODE_SIZE_INBYTES 2

int main(int argc, char *argv[]) {
    //Check too she if a filename was given
    if (argc != 2) {
        fprintf(stderr,"Error: useage: decompile.o <filename>\n");
        return 1;
    }

    //Variables used
    FILE *programFile, *decompileFile;
    int fileSize = 0, programCounter = 0x200;
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

   printf("PC\t\tOP Code\t\tInstructions\n");

    //read from programfile until EOF
    while(!feof(programFile)) {
        //set the buffer to all zeros
        memset(buffer,0,OPCODE_SIZE_INBYTES);

        //Get the 2 byte opcode and put it into the buffer
        result = fread(buffer,1,OPCODE_SIZE_INBYTES, programFile);

        //if end of file, exit loop
        if(feof(programFile)) {
            break;
        }

        opcode = 0;
        opcode = buffer[0];
        opcode = opcode << 8;
        opcode = opcode | buffer[1];
        //printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);

        if(buffer[0] >> 4 == 0x0 ) {
            if((buffer[0] & 0xF == 0) && buffer[1] == 0xE0) {
                //Clears the screen.
                printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
            }
            else if((buffer[0] & 0xF == 0) && buffer[1] == 0xEE) {
                //return subroutine
                printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
            }
            else {
                // Call RCA 1802
                printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
            }
        }
        else if (buffer[0] >> 4 == 0x1) {
            //JUMP
            printf("%#06X\t\t%02X %02X\t\tjmp\t\t%#03X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
        }
        else if (buffer[0] >> 4 == 0x2) {
            //Call subroutine
            printf("%#06X\t\t%02X %02X\t\tcall\t\t%#05X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
        }
        else if (buffer[0] >> 4 == 0x3) {
            //if(Vx == NN)
            printf("%#06X\t\t%02X %02X\t\tskip.nel\tV%1X, %#02X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x4) {
            //if(Vx!=NN)
            printf("%#06X\t\t%02X %02X\t\tskip.eql\t\tV%1X, %#02X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x5) {
            //if(Vx==Vy)
            printf("%#06X\t\t%02X %02X\t\tskip.ne\t\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0));
        }
        else if (buffer[0] >> 4 == 0x6) {
            //Vx = NN
            printf("%#06X\t\t%02X %02X\t\tmov.l\t\tV%1X, %#04X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x7) {
            //Vx += NN
            printf("%#06X\t\t%02X %02X\t\tadd.l\t\tV%1X, %#04X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x8) {
            // math function to do
            printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x9) {
            //if(Vx!=Vy)
            printf("%#06X\t\t%02X %02X\t\tskip.eq\t\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0));
        }
        else if (buffer[0] >> 4 == 0xA) {
            //	I = NNN
            printf("%#06X\t\t%02X %02X\t\tmov.l\t\tI, %#03X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
        }
        else if (buffer[0] >> 4 == 0xB) {
            //PC=V0+NNN
            printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
        }
        else if (buffer[0] >> 4 == 0xC) {
            //Vx=rand()&NN
            printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
        }
        else if (buffer[0] >> 4 == 0xD) {
            //	draw(Vx,Vy,N)
            printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
        }
        else if (buffer[0] >> 4 == 0xE) {
            //KeyOp
            printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
        }
        else if (buffer[0] >> 4 == 0xF) {
            //timer and sound
            printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);
        }
        else {
            //unexpected behavior
            printf("WARNING UNEXPECTED BAVHIOR: at %d\n", programCounter);
        }
        programCounter++;
    }


    return 0;
}