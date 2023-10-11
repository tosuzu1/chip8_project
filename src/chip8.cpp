#include "chip8.hpp"
#include <fstream>


const uint16_t START_ADDRESS {0x200};

Chip8::Chip8()
{
    this->programCounter = START_ADDRESS;
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
			this->memory[START_ADDRESS + i] = buffer[i];
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
    // process to get opcode, as the opcode is comprised of two part of the memory
    opcode = 0;
    opcode = memory[_programCounter];
    opcode = opcode << 8;
    opcode = opcode | memory[_programCounter + 1];

    return opcode;
}