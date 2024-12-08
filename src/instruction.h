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

    OP_NEG,

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

#define ENC_CONSTANT(cIndex, r)     (OP_CONSTANT << 26) | (r << 18) | (cIndex & 0x1FFFF)
#define DEC_CONSTANT_INDEX(i)       (i & 0x1FFFF)
#define DEC_REGISTER_C(i)           ((i >> 18) & 0xFF)

#define DEC_REGISTER_DEST(i)         ((i >> 18) & 0xFF)
#define DEC_REGISTER_RA(i)           ((i >> 10) & 0xFF)
#define DEC_REGISTER_RB(i)           ((i & 0x1FF))
#define ENC_ADD(dstr, ra, rb)        ((OP_ADD << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_MUL(dstr, ra, rb)        ((OP_MUL << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_SUB(dstr, ra, rb)        ((OP_SUB << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_DIV(dstr, ra, rb)        ((OP_DIV << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))

#define ENC_NEG(r)                   ((OP_NEG << 26) | ((r & 0xFF) << 18))
#define DEC_NEG(r)                   ((i >> 18) & 0xFF)


#define ENC_RETURN()                 (OP_RETURN << 26) & 0xFC000000

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
