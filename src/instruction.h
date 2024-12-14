#ifndef H_INSTRUCTION
#define H_INSTRUCTION

#include "common.h"
#include "value.h"
#include "idoconf.h"

typedef enum {
    OP_LOAD, 
    OP_CONSTANT,

    OP_PRINT,

    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_NEG,

    OP_TRUE,
    OP_FALSE,
    OP_NIL,

    OP_NOT,

    OP_GREATER,
    OP_LESS,
    OP_EQUAL,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_BANG_EQUAL,

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
// #define DEC_REGISTER_C(i)           ((i >> 18) & 0xFF)

#define DEC_REGISTER_DEST(i)         ((i >> 18) & 0xFF)
#define DEC_REGISTER_RA(i)           ((i >> 10) & 0xFF)
#define DEC_REGISTER_RB(i)           ((i & 0x1FF))

#define ENC_ADD(dstr, ra, rb)        ((OP_ADD << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_MUL(dstr, ra, rb)        ((OP_MUL << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_SUB(dstr, ra, rb)        ((OP_SUB << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_DIV(dstr, ra, rb)        ((OP_DIV << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))

#define ENC_NEG(r)                   ((OP_NEG << 26) | ((r & 0xFF) << 18))
// #define DEC_NEG(r)                   ((i >> 18) & 0xFF)

#define ENC_TRUE(r)                  (OP_TRUE   << 26) | (r << 18)
#define ENC_FALSE(r)                 (OP_FALSE  << 26) | (r << 18)
#define ENC_NIL(r)                     (OP_NIL    << 26) | (r << 18)

#define ENC_NOT(r)                       (OP_NOT            << 26) | (r << 18)
#define ENC_GREATER(dstr, ra, rb)        ((OP_GREATER       << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_GREATER_EQUAL(dstr, ra, rb)  ((OP_GREATER_EQUAL << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_LESS(dstr, ra, rb)           ((OP_LESS          << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_LESS_EQUAL(dstr, ra, rb)     ((OP_LESS_EQUAL    << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_EQUAL(dstr, ra, rb)          ((OP_EQUAL         << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
#define ENC_BANG_EQUAL(dstr, ra, rb)     ((OP_BANG_EQUAL    << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))



#define ENC_DEFINE_GLOBAL(rReadFrom, cIndex)       (OP_DEFINE_GLOBAL << 26) | (rReadFrom << 18) | (cIndex & 0x1FFFF) // TODO: look at possibility to remove this instruction an use SET_GLOBAL instead
#define ENC_GET_GLOBAL(cIndex, r)       (OP_GET_GLOBAL << 26)  | (r << 18) | (cIndex & 0x1FFFF)
#define ENC_SET_GLOBAL(rReadFrom, cIndex)       (OP_SET_GLOBAL << 26) | (rReadFrom << 18) | (cIndex & 0x1FFFF)
#define DEC_GET_GLOBAL_CINDEX(instruction)     ((instruction) & 0x03FFFFFF)

#define ENC_GET_LOCAL(cIndex, r)       (OP_GET_LOCAL << 26)  | (r << 18) | (cIndex & 0x1FFFF)
#define ENC_SET_LOCAL(rReadFrom, rIndex)            (OP_SET_LOCAL << 26)  | (rReadFrom << 18) | (rIndex & 0x1FFFF)



#define ENC_PRINT(rIndex)            (OP_PRINT << 26) | (rIndex & 0x03FFFFFF)
#define ENC_RETURN                   (OP_RETURN << 26) 
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
