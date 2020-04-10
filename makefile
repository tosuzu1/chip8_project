all	: emulator disassembler ncurse

emulator : 
	gcc -o chip8emulator.out chip8emulator.c -lncurses

disassembler : 
	gcc -o chip8disassembler.out chip8disassembler.c

ncurse:
	gcc -o ./notes/test_ncurse.out ./notes/test_ncurse.c -lncurses

clean	: 
	rm chip8disassembler.out chip8emulator.out
