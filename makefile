all	: emulator disassembler 

emulator : 
	gcc -Wall -o chip8emulator.out chip8emulator.c -lSDL2

emulatorDEBUG :
	gcc -Wall  -o chip8emulator.out chip8emulator.c -lSDL2 -D DEBUG -g

disassembler : 
	gcc -Wall  -o chip8disassembler.out chip8disassembler.c

test:
	gcc -Wall  -o ./notes/testSDL.out ./notes/testSDL.c -lSDL2

clean	: 
	rm *.out *.log
