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
    FILE *programFile;
    int  programCounter = 0x200;
    unsigned char * buffer = (unsigned char*) malloc (sizeof(char)*OPCODE_SIZE_INBYTES); //stores op code
    uint16_t opcode;
    //size_t result;

    programFile = fopen(argv[1], "r");

    //Check to see if the file can be opened
    if(programFile == NULL) {
        fprintf(stderr,"Error: Cannot open file or file not found\n");
        return 1;
    }

   printf("PC\t\tOP Code\t\tInstructions\n");

    //read from programfile until EOF
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

        opcode = 0;
        opcode = buffer[0];
        opcode = opcode << 8;
        opcode = opcode | buffer[1];
        //printf("%#06X\t\t%02X %02X\t\tTESTCODE\n",programCounter, buffer[0],buffer[1]);

        if(buffer[0] >> 4 == 0x0 ) {
            if(((buffer[0] & 0xF) == 0) && buffer[1] == 0xE0) {
                //Clears the screen.
                printf("%#06X\t\t%02X %02X\t\tclrs\n",programCounter, buffer[0],buffer[1]);
            }
            else if(((buffer[0] & 0xF) == 0) && buffer[1] == 0xEE) {
                //return subroutine
                printf("%#06X\t\t%02X %02X\t\trts\n",programCounter, buffer[0],buffer[1]);
            }
            else {
                // Call RCA 1802 programs
                printf("%#06X\t\t%02X %02X\t\tcall\t%#05X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
            }
        }
        else if (buffer[0] >> 4 == 0x1) {
            //JUMP
            printf("%#06X\t\t%02X %02X\t\tjmp\t%#05X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
        }
        else if (buffer[0] >> 4 == 0x2) {
            //Call subroutine
            printf("%#06X\t\t%02X %02X\t\tcall\t%#05X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
        }
        else if (buffer[0] >> 4 == 0x3) {
            //if(Vx == NN)
            printf("%#06X\t\t%02X %02X\t\tskip.el\tV%1X, %#04X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x4) {
            //if(Vx!=NN)
            printf("%#06X\t\t%02X %02X\t\tskip.nel\tV%1X, %#04X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x5) {
            //if(Vx==Vy)
            printf("%#06X\t\t%02X %02X\t\tskip.eq\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4);
        }
        else if (buffer[0] >> 4 == 0x6) {
            //Vx = NN
            printf("%#06X\t\t%02X %02X\t\tmov.l\tV%1X, %#04X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x7) {
            //Vx += NN
            printf("%#06X\t\t%02X %02X\t\tadd.l\tV%1X, %#04X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),buffer[1]);
        }
        else if (buffer[0] >> 4 == 0x8) {
            // math function to do
            if((buffer[1] & 0x0F) == 0) {
                //vx = vy
                printf("%#06X\t\t%02X %02X\t\tmov\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x01) {
                //vx = vy|vy
                printf("%#06X\t\t%02X %02X\t\tor\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x02) {
                //vx = vy&vy
                printf("%#06X\t\t%02X %02X\t\tand\tV%1X, V%1X\n",programCounter,buffer[0],buffer[1], (buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x03) {
                //vx = vy^vy
                printf("%#06X\t\t%02X %02X\t\txor\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x04) {
                //vx += vy
                printf("%#06X\t\t%02X %02X\t\tadd\tV%1X, V%1X\n",programCounter,buffer[0],buffer[1], (buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x05) {
                //vx -= vy
                printf("%#06X\t\t%02X %02X\t\tsub\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x06) {
                //Vx>>=1
                printf("%#06X\t\t%02X %02X\t\tshift.r\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x07) {
                //Vx=Vy-Vx
                printf("%#06X\t\t%02X %02X\t\tsub.rev\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4);
            }
            else if((buffer[1] & 0x0F) == 0x0E) {
                //Vx=Vy-Vx
                printf("%#06X\t\t%02X %02X\t\tshift.l\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4);
            }
            else {
                //Undefined at
                //printf("%#06X\t\t%02X %02X\t\tERROR1\n",programCounter, buffer[0],buffer[1]);
                printf("%#06X\t\t%02X %02X\t\tERROR1\t%d\n",programCounter, buffer[0],buffer[1],buffer[1] & 0x0F);
            }
        }
        else if (buffer[0] >> 4 == 0x9) {
            //if(Vx!=Vy)
            printf("%#06X\t\t%02X %02X\t\tskip.eq\tV%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4 >> 4);
        }
        else if (buffer[0] >> 4 == 0xA) {
            //	I = NNN
            printf("%#06X\t\t%02X %02X\t\tmov.l\tI, %#03X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
        }
        else if (buffer[0] >> 4 == 0xB) {
            //PC=V0+NNN
            printf("%#06X\t\t%02X %02X\t\tjump.add\tV0, %#05X\n",programCounter, buffer[0],buffer[1],(opcode & 0xFFF));
        }
        else if (buffer[0] >> 4 == 0xC) {
            //Vx=rand()&NN
            printf("%#06X\t\t%02X %02X\t\trand\tV%1X, %#04X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(opcode & 0xFF));
        }
        else if (buffer[0] >> 4 == 0xD) {
            //	draw(Vx,Vy,N)
            printf("%#06X\t\t%02X %02X\t\tdraw\tV%1X, V%1X, V%1X\n",programCounter, buffer[0],buffer[1],(buffer[0] & 0xF),(buffer[1] & 0xF0) >> 4,(buffer[1] & 0xF));
        }
        else if (buffer[0] >> 4 == 0xE) {
            if(buffer[1] == 0x9E) {
                //skip if key press is equal to vx
                printf("%#06X\t\t%02X %02X\t\tskip.keyeq\tV%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
            else if(buffer[1] == 0xA1) {
                //dkip if key press is not equal to vx
                printf("%#06X\t\t%02X %02X\t\tskip.keyne\tV%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
            else {
                //Undefined at
                printf("%#06X\t\t%02X %02X\t\tERROR2\n",programCounter, buffer[0],buffer[1]);
            }
        }
        else if (buffer[0] >> 4 == 0xF) {
            //timer and sound
            if(buffer[1] == 0x07) {
                //set vx to delay timer
                printf("%#06X\t\t%02X %02X\t\tgetdelay\tV%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
            else if(buffer[1] == 0x0A) {
                //get key press and store into vx
                printf("%#06X\t\t%02X %02X\t\tkey.op\tV%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
            else if(buffer[1] == 0x15) {
                //set delay timer to vx
                printf("%#06X\t\t%02X %02X\t\ttimer\tV%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
            else if(buffer[1] == 0x18) {
                //set sound timer to vx
                printf("%#06X\t\t%02X %02X\t\tsound\t\tV%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
            else if(buffer[1] == 0x1E) {
                //increment I by vx
                printf("%#06X\t\t%02X %02X\t\tadd\t\tI, V%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
             else if(buffer[1] == 0x29) {
                //I=sprite_addr[Vx]
                printf("%#06X\t\t%02X %02X\t\tsprite\tI, V%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
             else if(buffer[1] == 0x33) {
                //	set_BCD(Vx)
                printf("%#06X\t\t%02X %02X\t\tmov.bcd\t\tI, V%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
              else if(buffer[1] == 0x55) {
                //reg_dump at I
                printf("%#06X\t\t%02X %02X\t\tstore\t\tI, V%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
              else if(buffer[1] == 0x65) {
                //reg_load at I
                printf("%#06X\t\t%02X %02X\t\tload\t\tI, V%1X\n",programCounter, buffer[0],buffer[1],buffer[0] & 0xF);
            }
            else {
                //Undefined at
                printf("%#06X\t\t%02X %02X\t\tERROR3\n",programCounter, buffer[0],buffer[1]);
            }
        }
        else {
            //unexpected behavior
            printf("%#06X\t\t%02X %02X\t\tERROR\n",programCounter, buffer[0],buffer[1]);
        }
        programCounter += 2;
    }

    free(buffer);
    return 0;
}