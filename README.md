# chip8_project
This repository is my journel into learning about the Chip8 programming 

# DO 
The emulator shall be able to disassemble a Chip-8 ROM and produce source assembly listings.
1. accept a input of file name 
2. read every 16 bit and output source code 
	1. statement to check which opcode, need to interpt to assembly
	2. seem there not offical assembly instruction and I could make up my own.

The emulator shall be capable of executing a Chip-8 ROM without crashing.
	Emulator works now with all code without grahpics

The emulator shall be able to accept user input via keyboard and produce a graphical display.

Requires SDL, 
sudo apt-get install libsdl2-2.0 //Debian systems
See SDL webpage to install for your system https://wiki.libsdl.org/Installation

Useage.

* make all 
* make emulator 
* make disassembler
* make clean 
* make emulatorDEBUG

USEAGE
./chip8disassembler	<rom file>
./chip8emulator		<rom file>

*When debug is used, log of chip8 state will go to debug.log
