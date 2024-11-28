#ifndef C_INSTRUCTION
#define C_INSTRUCTION

#include "instruction.h"
#include "memory.h"

void initProgram(Program* prog){
    prog->capacity = 0;
    prog->count = 0;
    prog->lines = NULL;
    prog->code = NULL;
    initValueArray(&prog->constants);
}

void writeToProgram(Program* prog, ido_uint32 bytecode, int line){
    if(prog->capacity < prog->count + 1){
        int oldCap = prog->capacity;
        prog->capacity = GROW_CAPACITY(oldCap);
        prog->code = GROW_ARRAY(ido_uint32, prog->code, oldCap, prog->capacity);
        prog->lines = GROW_ARRAY(int, prog->lines, oldCap, prog->capacity);
    }

    prog->code[prog->count] = bytecode;
    prog->lines[prog->count] = line;

    prog->count++;
}

void freeProgram(Program* prog){
    FREE_ARRAY(ido_uint32, prog->code, prog->capacity);
    FREE_ARRAY(int, prog->lines, prog->capacity);
    freeValueArray(&prog->constants);
    initProgram(prog);
}

int addConstant(Program* prog, Value value){
    writeValueArray(&prog->constants, value);
    return prog->constants.count - 1;
}


#endif