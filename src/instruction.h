#ifndef H_INSTRUCTION
#define H_INSTRUCTION

#include "common.h"
#include "value.h"
#include "idoconf.h"

typedef enum {
    OP_LOAD, // Loads a value into a register 
    OP_CONSTANT,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,


    OP_HLT, // Halts the Tania vm
    OP_IGL, // ilegal op found
} Opcode;

#if IS32INT
    typedef unsigned int ido_uint32;
#else
    typedef unsigned long ido_uint32;
#endif

typedef ido_uint32 Instruction;

typedef struct {
    Instruction* code;
    ValueArray constants;
    int capacity;
    int count;
    int* lines;
} Program;

void initProgram(Program* prog);
void writeToProgram(Program* prog, uint8_t bytecode, int line);
void freeProgram(Program* prog);
int addConstant(Program* prog, Value value);

#endif