# chip8_project
This repository is my journel into learning about the Chip8 programming 

DO 
The emulator shall be able to disassemble a Chip-8 ROM and produce source assembly listings.
	1) accept a input of file name {CHECK}
	2) read every 16 bit and output source code {check}
		If statement to check which opcode, need to interpt to assembly
		it seem there not offical assembly instruction and I could make up my own.

The emulator shall be capable of executing a Chip-8 ROM without crashing.

The emulator shall be able to accept user input via keyboard and produce a graphical display.

Useage.

TODO
	Makefile

gcc -o chip8disassemble chip8disassemble.c