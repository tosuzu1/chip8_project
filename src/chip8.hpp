#include <cstdint>

#define CHIP8_MEMORY_LIMIT 4096
#define CHIP8_STACKSIZE 12

class Chip8 
{      
  private:
    // Everything in here should be public
    // The user should be free to hack and mess aroudn with the processor

  public:
    uint8_t memory[CHIP8_MEMORY_LIMIT]{}; 
    uint8_t registers[16]{};
    uint8_t addressRegister{};
    uint16_t programCounter{};
    uint8_t stackPointer{};
    uint16_t stack[16]{};
    uint8_t soundTimer{};
    uint8_t delayTimer{};
    uint16_t opcode{}; 
    uint16_t keyboard{};

    void op_0NNN_syscall();
    void op_00E0_clrs();
    void op_00EE_rts();
    void op_1NNN_jmp();
    void op_2NNN_call();
    void op_3XNN_skipel();
    void op_4NNN_skipnel();
    void op_5XY0_skipeq();
    void op_6XNN_movl();
    void op_7XNN_addl();
    void op_8XY0_or();
    void op_8XY1_and();
    void op_8XY3_xor();
    void op_8XY4_add();
    void op_8XY5_sub();
    void op_8XY6_shiftr();
    void op_8XY7_subrev();
    void op_8XYE_shiftl();
    void op_9XY0_skipne();
    void op_ANNN_regil();
    void op_BNNN_jmpadd();
    void op_CXNN_randor();
    void op_DXYN_draw();
    void op_EX9E_skipeqkey();
    void op_EXA1_skipnekey();
    void op_FX07_movtimer();
    void op_FX0A_getkey();
    void op_FX15_timer();
    void op_FX18_sound();
    void op_FX29_addrei();
    void op_FX33_bcd();
    void op_FX55_regDump();
    void op_FX65_regLoad();

    //Contructor
    Chip8();

    int load_program(char const* filename);
    void set_memory(uint16_t _programCounter, uint8_t _memory_data);
    void clear_memory();
    int cycle(); 
    uint16_t get_programCounter();
    void set_programCounter(uint16_t _programCounter);
    uint16_t get_opcode();
    uint16_t get_opcode(uint16_t _programCounter);
    void set_keyboard(uint16_t _key);
    uint16_t get_keyboard();
    uint8_t get_processerInfo();
};