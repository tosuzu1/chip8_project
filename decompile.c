#include <stdio.h>
#include <stdlib.h>
#define OPCODE_SIZE_INBYTES 2

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Error: useage: decompile.o <filename>\n");
        return 1;
    }

    FILE *programFile, *decompileFile;
    int fileSize = 0;
    char * buffer; //stores op code
    buffer = (char*) malloc (sizeof(char)*OPCODE_SIZE_INBYTES);
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
        result = fread(buffer,1,OPCODE_SIZE_INBYTES, programFile);
        printf("DEBUG: opcode reads %x , %x\n", buffer[0],buffer[1]);
    }


    return 0;
}