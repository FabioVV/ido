#ifndef C_INSTRUCTION
#define C_INSTRUCTION

#include "instruction.h"
#include "memory.h"

void initProgram(Program* prog){
    prog->capacity = 0;
    prog->count = 0;
    prog->lines = NULL;
    prog->code = NULL;
}

void writeToProgram(Program* prog, uint8_t bytecode, int line){
    if(prog->capacity < prog->count + 1){
        int oldCap = prog->capacity;
        prog->capacity = GROW_CAPACITY(oldCap);
        prog->code = GROW_ARRAY(uint8_t, prog->code, oldCap, prog->capacity);
        prog->lines = GROW_ARRAY(int, prog->lines, oldCap, prog->capacity);
    }

    prog->code[prog->count] = bytecode;
    prog->lines[prog->count] = line;

    prog->count++;
}

void freeProgram(Program* prog){
    FREE_ARRAY(uint8_t, prog->code, prog->capacity);
    FREE_ARRAY(int, prog->lines, prog->capacity);
    
    initProgram(prog);
}


#endif