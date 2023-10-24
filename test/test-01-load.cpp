#include <iostream>
#include "chip8.hpp"
#include <fstream>
#include <iomanip>

// This program is used to test if a program load correctly

int main (int argc, char *argv[])
{
    Chip8 newProcessor;

    newProcessor.load_program("../data/Clock.ch8");

    std::cout // show the 0x prefix
            << std::internal // fill between the prefix and the number
            << std::setfill('0'); // fill with 0s

    for(int i = 0; i < 20; i += 2)
    {
        std::cout << "0x" << std::setw(3) << std::hex <<  
        (newProcessor.get_programCounter() + i) << "\t\t";

        std::cout << "0x" << std::setw(4) << std::hex 
        
            << newProcessor.get_opcode(newProcessor.get_programCounter() + i)
            << std::endl;
    }

    
    return 0;
}