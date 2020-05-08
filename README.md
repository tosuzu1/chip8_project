# chip8_project
This repository is my journel into learning about the Chip8 programming 

# DO 
The emulator shall be able to disassemble a Chip-8 ROM and produce source assembly listings.
1. accept a input of file name {CHECK}
2. read every 16 bit and output source code {check}
	1. statement to check which opcode, need to interpt to assembly
	2. seem there not offical assembly instruction and I could make up my own.

The emulator shall be capable of executing a Chip-8 ROM without crashing.
	Emulator works now with all code without grahpics

The emulator shall be able to accept user input via keyboard and produce a graphical display.

# Useage.

* make all  				/Builds both emulator and disassembler
* make emulator				/builds emulator only
* make disassembler			/builds disassembler only
* make clean				/remove output files

./chip8disassembler	<rom file>	/disassembles target rom
./chip8emulator		<rom file>	/Emulates target rom
