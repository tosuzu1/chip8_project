#pragma once

#include <cstdint>
#include <random>
#include <map>

const unsigned int CHIP8_MEMORY_LIMIT { 4096};
const unsigned int DISPLAY_RESOLUTION_HORIZONTAL {64};
const unsigned int DISPLAY_RESOLUTION_VERTICAL {32};
const unsigned int REGISTER_SIZE {16};
const unsigned int STACK_SIZE{16};


class Chip8 
{      
  public:
    uint8_t memory[CHIP8_MEMORY_LIMIT]{}; 
    uint8_t registers[REGISTER_SIZE]{};
    uint8_t addressRegister{};
    uint16_t programCounter{};
    uint8_t stackPointer{};
    uint16_t stack[STACK_SIZE]{};
    uint8_t soundTimer{};
    uint8_t delayTimer{};
    uint16_t opcode{}; 
    uint8_t keyboard[16]{};
    uint8_t displayGrid[DISPLAY_RESOLUTION_HORIZONTAL][DISPLAY_RESOLUTION_VERTICAL];
  

    //Contructor
    Chip8();

    int load_program(char const* filename);
    void set_memory(uint16_t _programCounter, uint8_t _memory_data);
    void clear_memory();
    void cycle(); 
    uint16_t get_programCounter();
    void set_programCounter(uint16_t _programCounter);
    uint16_t get_opcode();
    uint16_t get_opcode(uint16_t _programCounter);
    void set_keyboard(uint16_t _key);
    uint16_t get_keyboard();
    uint8_t get_processerInfo();
    void callHello();
    std::map<int, void (Chip8::*)()> funcMap;
    std::map<int, void (Chip8::*)()> funcMap0;
    std::map<int, void (Chip8::*)()> funcMap8;
    std::map<int, void (Chip8::*)()> funcMapE;
    std::map<int, void (Chip8::*)()> funcMapF;
    
    

   

  private:
    std::random_device rd;
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t>  random_byte;

    void op_0NNN();
    void op_00E0();
    void op_00EE();
    void op_1NNN();
    void op_2NNN();
    void op_3XNN();
    void op_4XNN();
    void op_5XY0();
    void op_6XNN();
    void op_7XNN();
    void op_8XY0();
    void op_8XY1();
    void op_8XY2();
    void op_8XY3();
    void op_8XY4();
    void op_8XY5();
    void op_8XY6();
    void op_8XY7();
    void op_8XYE();
    void op_9XY0();
    void op_ANNN();
    void op_BNNN();
    void op_CXNN();
    void op_DXYN();
    void op_EX9E();
    void op_EXA1();
    void op_FX07();
    void op_FX0A();
    void op_FX15();
    void op_FX18();
    void op_FX1E();
    void op_FX29();
    void op_FX33();
    void op_FX55();
    void op_FX65();
    void op_FXFF();
    void opcode_0();
    void opcode_8();
    void opcode_E();
    void opcode_F();
};
