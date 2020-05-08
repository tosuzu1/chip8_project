all	: emulator disassembler 

emulator : 
	gcc -Wall -o chip8emulator.out chip8emulator.c -lncurses 

emulatorDEBUG :
	gcc -Wall  -o chip8emulator.out chip8emulator.c -lncurses -D DEBUG

disassembler : 
	gcc -Wall  -o chip8disassembler.out chip8disassembler.c

ncurse:
	gcc -Wall  -o ./notes/test_ncurse.out ./notes/test_ncurse.c -lncurses

clean	: 
	rm *.out *.log
