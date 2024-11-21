#ifndef H_INSTRUCTION
#define H_INSTRUCTION

#include "common.h"

typedef enum {
    OP_HLT, // Halts the Tania vm
    OP_IGL, // Ilegal op found
    OP_LOAD, // Loads a value into a register 

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
} Opcode;

typedef struct {
    uint8_t* code;
    int capacity;
    int count;
    int* lines;
} Program;

void initProgram(Program* prog);
void writeToProgram(Program* prog, uint8_t bytecode, int line);
void freeProgram(Program* prog);

#endif