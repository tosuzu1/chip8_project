#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define OPCODE_SIZE_INBYTES 2

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Error: useage: decompile.o <filename>\n");
        return 1;
    }

    FILE *programFile, *decompileFile;
    int fileSize = 0;
    unsigned char * buffer; //stores op code
    buffer = (char*) malloc (sizeof(char)*OPCODE_SIZE_INBYTES);
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

    //TODO: implement a check to see if a filename already exist using access

    //TODO: name output file as fileinput name with appended tag, for now out as output.txt
    decompileFile = fopen("output.txt", "w");
    if(decompileFile == NULL) {
        fprintf(stderr,"Error: Cannot open file\n");
        return 1;
    }

    //read from programfile until EOF
    while(!feof(programFile)) {
        memset(buffer,0,OPCODE_SIZE_INBYTES);
        result = fread(buffer,1,OPCODE_SIZE_INBYTES, programFile);
        if(feof(programFile)) {
            break;
        }
        opcode = 0;
        opcode = buffer[0];
        opcode = opcode << 8;
        opcode = opcode | buffer[1];
        printf("DEBUG: opcode reads %#8x , %#8x, %d\n", buffer[0],buffer[1], opcode);

        if(buffer[0] >> 4 == 0x0 ) {
            if((buffer[0] & 0xF == 0) && buffer[1] == 0xE0) {
                //Clears the screen.
            }
            else if((buffer[0] & 0xF == 0) && buffer[1] == 0xEE) {
                //return subroutine
            }
            else {
                // Call RCA 1802
            }
        }
        else if (buffer[0] >> 4 == 0x1) {
            //JUMP
        }
        else if (buffer[0] >> 4 == 0x2) {
            //Call subroutine
        }
        else if (buffer[0] >> 4 == 0x3) {
            //if(Vx == NN)
        }
        else if (buffer[0] >> 4 == 0x4) {
            //if(Vx!=NN)
        }
        else if (buffer[0] >> 4 == 0x5) {
            //if(Vx==Vy)
        }
        else if (buffer[0] >> 4 == 0x6) {
            //Vx = NN
        }
        else if (buffer[0] >> 4 == 0x7) {
            //Vx += NN
        }
        else if (buffer[0] >> 4 == 0x8) {
            // math function to do
        }
        else if (buffer[0] >> 4 == 0x9) {
            //if(Vx!=Vy)
        }
        else if (buffer[0] >> 4 == 0xA) {
            //	I = NNN
        }
        else if (buffer[0] >> 4 == 0xB) {
            //PC=V0+NNN
        }
        else if (buffer[0] >> 4 == 0xC) {
            //Vx=rand()&NN
        }
        else if (buffer[0] >> 4 == 0xD) {
            //	draw(Vx,Vy,N)
        }
        else if (buffer[0] >> 4 == 0xE) {
            //KeyOp
        }
        else if (buffer[0] >> 4 == 0xF) {
            //timer and sound
        }
        else {
            //unexpected behavior
        }
    }


    return 0;
}