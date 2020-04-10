all	: emulator disassembler

emulator : 
	gcc -o chip8emulator.out chip8emulator.c

disassembler : 
	gcc -o chip8disassembler.out chip8disassembler.c

clean	: 
	rm chip8disassembler.out chip8emulator.out
