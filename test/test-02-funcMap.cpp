#include <iostream>
#include "chip8.hpp"
#include <fstream>
#include <iomanip>


int main (int argc, char *argv[])
{
    // This program is used to test function mapping.
    Chip8 newProcessor;

    newProcessor.load_program("/home/hongf/Desktop/chip8/data/Clock.ch8");

    newProcessor.callHello();

    return 0;
}