#include "chip8.hpp"
#include <fstream>
#include <cstring> 
#include <random>
#include <map>
#include <iostream>
#include <chrono>

const uint16_t START_ADDRESS {0x200};
const unsigned int FONTSET_SIZE {80};
const uint16_t START_FONT_ADRESS {0};

uint8_t fontset[FONTSET_SIZE] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};



Chip8::Chip8() :
    randGen(rd())
{
    // Init Chip8 starting address
    programCounter = START_ADDRESS;

	// Load fonts into memory 
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[START_FONT_ADRESS + i] = fontset[i];
	}

    // create random number gen
    random_byte = std::uniform_int_distribution<uint8_t>(0, 255U);

    
    //create a map of all function
    funcMap[0x0] = &Chip8::opcode_0;
    funcMap[0x1] = &Chip8::op_1NNN;
    funcMap[0x2] = &Chip8::op_2NNN;
    funcMap[0x3] = &Chip8::op_3XNN;
    funcMap[0x4] = &Chip8::op_4XNN;
    funcMap[0x5] = &Chip8::op_5XY0;
    funcMap[0x6] = &Chip8::op_6XNN;
    funcMap[0x7] = &Chip8::op_7XNN;
    funcMap[0x8] = &Chip8::opcode_8;
    funcMap[0x9] = &Chip8::op_9XY0;
    funcMap[0xA] = &Chip8::op_ANNN;
    funcMap[0xB] = &Chip8::op_BNNN;
    funcMap[0xC] = &Chip8::op_CXNN;
    funcMap[0xD] = &Chip8::op_DXYN;
    funcMap[0xE] = &Chip8::opcode_E;
    funcMap[0xF] = &Chip8::opcode_F;
    
    //create a map of all function starting with 0
    funcMap0[0x0E0] =&Chip8::op_00E0;
    funcMap0[0x0EE] =&Chip8::op_00EE;
    funcMap0[0x0] =&Chip8::op_0NNN;

    //create a map of all function starting with 8
    funcMap8[0x0] = &Chip8::op_8XY0;
    funcMap8[0x1] = &Chip8::op_8XY1;
    funcMap8[0x2] = &Chip8::op_8XY2;
    funcMap8[0x3] = &Chip8::op_8XY3;
    funcMap8[0x4] = &Chip8::op_8XY4;
    funcMap8[0x5] = &Chip8::op_8XY5;
    funcMap8[0x6] = &Chip8::op_8XY6;
    funcMap8[0x7] = &Chip8::op_8XY7;
    funcMap8[0xE] = &Chip8::op_8XYE;

    //create a map of all function starting with E
    funcMapE[0x9E] = &Chip8::op_EX9E;
    funcMapE[0x07] = &Chip8::op_EXA1;

    //create a map of all function starting with F
    funcMapF[0x07] = &Chip8::op_FX07;
    funcMapF[0x0A] = &Chip8::op_FX0A;
    funcMapF[0x15] = &Chip8::op_FX15;
    funcMapF[0x18] = &Chip8::op_FX18;
    funcMapF[0x1E] = &Chip8::op_FX1E;
    funcMapF[0x29] = &Chip8::op_FX29;
    funcMapF[0x33] = &Chip8::op_FX33;
    funcMapF[0x55] = &Chip8::op_FX55;
    funcMapF[0x65] = &Chip8::op_FX65;
    // Test funmap
    funcMapF[0x1] = &Chip8::op_FXFF;

}

void Chip8::opcode_0()
{
    uint16_t lastThreeHex = this->get_opcode() & 0x0FFFu;

    if (lastThreeHex == 0x0E0 || lastThreeHex == 0x0EE) ((*this).*(funcMap0[ lastThreeHex]))();
    else ((*this).*(funcMap0[0]))();

}

void Chip8::opcode_8()
{
    uint8_t lastHex = get_opcode() & 0x000F;

    ((*this).*(funcMap8[lastHex]))();

}

void Chip8::opcode_E()
{
    uint8_t lastTwoHex = get_opcode() & 0x00FF;

    ((*this).*(funcMapE[lastTwoHex]))();

}

void Chip8::opcode_F()
{
    uint8_t lastTwoHex = get_opcode() & 0x00FF;

    ((*this).*(funcMapF[lastTwoHex]))();
}

int Chip8::load_program(char const* filename)
{
	// Open the file as a stream of binary and move the file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		// Get size of file and allocate a buffer to hold the contents
		std::streampos fileSize = file.tellg();
		char* buffer = new char[fileSize];

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, fileSize);
		file.close();

		// Load the ROM contents into the Chip8's memory, starting at 0x200
		for (int i = 0; i < fileSize; ++i)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}

		// Free the buffer
		delete[] buffer;

        // return sucess
        return 0;
	}
    else 
    {
        std::perror("ERROR: Unable to open file");
        return 1;
    }
}

void Chip8::set_memory(uint16_t _programCounter, uint8_t _memory_data) 
{
    this->memory[_programCounter] = _memory_data;
}

void Chip8::clear_memory() 
{
    // Iterate throuhgh memory and set to 0;
    for (uint16_t i = 0; i < CHIP8_MEMORY_LIMIT; i++)
    {
        this->memory[i] = 0;
    }
}

void Chip8::cycle() 
{
    uint16_t firstHex = (get_opcode() & 0xF000) >> 12;

    // increment program counter;
    programCounter += 2;

    // run the opcode base on firstHex
    ((*this).*(funcMap[firstHex]))();

    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) --soundTimer;
}

uint16_t Chip8::get_programCounter()
{
    return this->programCounter;
}

void Chip8::set_programCounter(uint16_t programCounter_) {
   this-> programCounter = programCounter_;
}

uint16_t Chip8::get_opcode()
{
    // process to get opcode, as the opcode is comprised of two part of the memory
    opcode = 0;
    opcode = memory[programCounter];
    opcode = opcode << 8;
    opcode = opcode | memory[programCounter + 1];

    return opcode;
}

uint16_t Chip8::get_opcode(uint16_t _programCounter)
{
    // process to get opcode, get op code for a specific adress
    opcode = 0;
    opcode = memory[_programCounter];
    opcode = opcode << 8;
    opcode = opcode | memory[_programCounter + 1];

    return opcode;
}



void Chip8::op_0NNN() 
{
    // syscall NNN
    // Get NNN from opcode
    uint16_t address = this->get_opcode() & 0x0FFFu;

    // Add the program to the stackprogramer and jmp to the new address
	stack[stackPointer] = programCounter;
	++stackPointer;
	programCounter = address;
}

void Chip8::op_00E0()
{
    // clr_disp
    // Clear the display by clearing the array
    memset(displayGrid, 0, sizeof(displayGrid));
}

void Chip8::op_00EE() 
{
    // return
    // return from subroutine
    --stackPointer;
    programCounter = stack[stackPointer];
}

void Chip8::op_1NNN()
{
    // JUMP NNN
    // Get NNN from opcode
    uint16_t address = get_opcode() & 0x0FFF;

    // JMP to address
	programCounter = address;
}

void Chip8::op_2NNN()
{
    // call NNN
     // Get NNN from opcode
    uint16_t address = get_opcode() & 0x0FFF;

    // Add the program to the stackprogramer and jmp to the new address
	stack[stackPointer] = programCounter;
	++stackPointer;
	programCounter = address;
}

void Chip8::op_3XNN()
{
    //skipel vx, l
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t val = get_opcode() & 0x00FF;

    if (v_x == val) programCounter += 2;
}

void Chip8::op_4XNN()
{
    //skipnel vx, l
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t val = get_opcode() & 0x00FF;

    if (v_x != val) programCounter += 2;
}

void Chip8::op_5XY0() 
{
    // skipeq vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    if (v_x == v_y) programCounter += 2;
}

void Chip8::op_6XNN()
{
    // mov vx, l
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t val = get_opcode() & 0x00FF;

    registers[v_x] = val;
}

void Chip8::op_7XNN()
{
    // addl vx, l
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t val = get_opcode() & 0x00FF;

    registers[v_x] += val;
}

void Chip8::op_8XY0() 
{
    // mov vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    registers[v_x] = registers[v_y];
}

void Chip8::op_8XY1() 
{
    // or vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    registers[v_x] |= registers[v_y];
}

void Chip8::op_8XY2() 
{
    // add vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    registers[v_x] &= registers[v_y];
}

void Chip8::op_8XY3() 
{
    // xor vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    registers[v_x] ^= registers[v_y];
}

void Chip8::op_8XY4()
{
    // add vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    u_int16_t sum = registers[v_x] + registers[v_y];

    // if overflow, set reg_f to 1
    (sum > 255) ?  registers[0xf] = 1 : registers[0xf] = 0;

    registers[v_x] += registers[v_y];
}

void Chip8::op_8XY5()
{
    // sub vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    // if overflow, set reg_f to 1
    (registers[v_y] > registers[v_x]) ?  registers[0xf] = 1 : registers[0xf] = 0;

    registers[v_x] -= registers[v_y];
}

void Chip8::op_8XY6()
{
    // shiftr vx
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    registers[0xf] = registers[v_x] & 0x1;

    registers[v_x] >>= 1;
}

void Chip8::op_8XY7()
{
    // subrev vx, vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    // if overflow, set reg_f to 1
    (registers[v_x] > registers[v_y]) ?  registers[0xf] = 1 : registers[0xf] = 0;

    registers[v_x] = registers[v_y] - registers[v_x];
}

void Chip8::op_8XYE() 
{
    // shiftl vx
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    registers[0xf] = (registers[v_x] & 0x80) >> 7;

    registers[v_x] <<= 1;
}

void Chip8::op_9XY0()
{
    //jmp_ne vx,vy
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;

    if (registers[v_x] != registers[v_y] ) programCounter += 2;
}

void Chip8::op_ANNN()
{
    //movl addr, l
    // get address from op code
    uint16_t address = get_opcode() & 0x0FFF;

    // load address into address pointer
    addressRegister = address;
}

void Chip8::op_BNNN()
{
    //Jumps to the address NNN plus V0.
    // get address from op code
    uint16_t address = get_opcode() & 0x0FFF;

    programCounter = address + registers[0];
}

void Chip8::op_CXNN()
{
    // rand vx
    // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t val = get_opcode() & 0x00FF;

    registers[v_x] = random_byte(randGen) & val;
}

void Chip8::op_DXYN() 
{
    // desp, vx, vy
    // decode data from opcode
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t v_y = (get_opcode() & 0x00F0) >> 4;
    uint8_t height = (get_opcode() & 0x000Fu);

    //Wrap if going beyond boundaries
    unsigned int xPos = registers[v_x] % DISPLAY_RESOLUTION_HORIZONTAL;
    unsigned int yPos = registers[v_y] % DISPLAY_RESOLUTION_VERTICAL;
    
    // set register 0xf 0, set to 1 if Xor
    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row)
    {
        // store the row of pixel 
        uint8_t spriteByte = memory[addressRegister + row];

        for(unsigned int col = 0; col < 8; ++col)
        {
            // get the pixel we're working with
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            // get the display pixel 
            uint8_t *displayPixel = &displayGrid[xPos + col][yPos + col];

            // if we have to draw something on screen
            if (spritePixel)
            {
                // if the display pixel was on
                if (*displayPixel == 1)
                {
                    // both display and draw were 1 so there a collision
                    registers[0xF] = 1;
                }

                // Xor the displayPixel
                *displayPixel ^= 1;
            }
        }
    }

}

void Chip8::op_EX9E() 
{
    // jumpeq vx, key
    // get vx from opcode
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;

    if (keyboard[v_x]) programCounter += 2;

}

void Chip8::op_EXA1()
{
    //jumpneq vx, key
    // get vx from opcode
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;

    if (!keyboard[v_x])programCounter += 2;
}

void Chip8::op_FX07()
{
    //mov vx , delay
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;

    registers[v_x] = delayTimer;
}

void Chip8::op_FX0A()
{
    //getKey
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;

    // continously check all key to see if a key was preessed,
    // if no key, go back to previous programcounter and check again 
    if (keyboard[0]) registers[v_x] = 0;
    else if (keyboard[1]) registers[v_x] = 1;
    else if (keyboard[2]) registers[v_x] = 2;
    else if (keyboard[3]) registers[v_x] = 3;
    else if (keyboard[4]) registers[v_x] = 4;
    else if (keyboard[5]) registers[v_x] = 5;
    else if (keyboard[6]) registers[v_x] = 6;
    else if (keyboard[7]) registers[v_x] = 7;
    else if (keyboard[8]) registers[v_x] = 1;
    else if (keyboard[9]) registers[v_x] = 1;
    else if (keyboard[10]) registers[v_x] = 10;
    else if (keyboard[11]) registers[v_x] = 11;
    else if (keyboard[12]) registers[v_x] = 12;
    else if (keyboard[13]) registers[v_x] = 13;
    else if (keyboard[14]) registers[v_x] = 14;
    else if (keyboard[15]) registers[v_x] = 15;
    else programCounter -= 2;


}

void Chip8::op_FX15()
{
    //mov delay, vx
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;

    delayTimer = registers[v_x];
}

void Chip8::op_FX18()
{
    //mov sound vx
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;

    soundTimer = registers[v_x];
}

void Chip8::op_FX1E()
{
    // add I, Vx
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;

    addressRegister += registers[v_x];
}

void Chip8::op_FX29()
{
    // ld I , spriteAdd[vx]
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t val = registers[v_x];

    // set addres to the Font specified by Vx
    addressRegister = START_FONT_ADRESS + (5 * val);
}

void Chip8::op_FX33()
{
    //BCD stores binary-coded decimal
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    uint8_t val = registers[v_x];

    // Ones-place
	memory[addressRegister + 2] = val % 10;
	val /= 10;

	// Tens-place
	memory[addressRegister + 1] = val % 10;
	val /= 10;

	// Hundreds-place
	memory[addressRegister] = val % 10;

}

void Chip8::op_FX55()
{
    //Reg_dump vx, I
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    //get val of Vx
    uint8_t val_vx = registers[v_x];

    for(int i = 0; i <= val_vx; i++) 
    {
        memory[addressRegister + i] = registers[i];
    }
}

void Chip8::op_FX65()
{
    //Reg_load vx, I
    uint8_t v_x = (get_opcode() & 0x0F00) >> 8;
    //get val of Vx
    uint8_t val = registers[v_x];

    for(int i = 0; i <= val; i++) 
    {
        registers[i] = memory[addressRegister + i];
    }
}

void Chip8::op_FXFF() 
{
    std::cout << "Hello world\n" ;
}

void Chip8::callHello()
{
    ((*this).*(funcMapF[1]))();
}