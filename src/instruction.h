#ifndef H_INSTRUCTION
#define H_INSTRUCTION

#include "common.h"
#include "value.h"
#include "idoconf.h"

typedef enum {
    OP_LOAD, 
    OP_CONSTANT,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_RETURN,
    OP_HLT, // Halts the vm
    OP_ILG, // ilegal op found
} Opcode;

#if IS32INT
    typedef unsigned int ido_uint32;
#else
    typedef unsigned long ido_uint32;
#endif

// Instructions handling
#define GET_OPCODE(i)               ((i >> 26) & 0x3F)
#define NEXT_INSTRUCTION(tvm)       (*tvm->pc++)
#define ENC_CONSTANT(constantIndex) (OP_CONSTANT << 26) | (constantIndex & 0x1FFFFFF)
#define DEC_CONSTANT(i)             (i & 0x1FFFFFF)

// Instructions handling

typedef ido_uint32 Instruction;

typedef struct {
    Instruction* code;
    ValueArray constants;
    int capacity;
    int count;
    int* lines;
} Program;

void initProgram(Program* prog);
void writeToProgram(Program* prog, ido_uint32 instruction, int line);
void freeProgram(Program* prog);
int addConstant(Program* prog, Value value);

#endif