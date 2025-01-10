#ifndef IDO_COMPILER
#define IDO_COMPILER

#include "token.h"
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "tvm.h"


#define LOCALS_NUM 200

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
} FunctionType;

typedef struct {
    Token name;
    int depth;
    uint8_t registerIndex;
} Local;

// typedef struct {
//     ido_uint32 start;
//     ido_uint32 end;
//     uint8_t registerIndex;
// } Intervals;

// typedef struct {
//     int capacity;
//     int count;
//     Intervals* intervals;
// } LiveInterval;

typedef struct {
    TVM* tvm;
    ObjFunction* function;
    FunctionType type;

    Local locals[LOCALS_NUM]; // What a strange limit...
    // LiveInterval liveIntervals; 
    int localCount;
    int scopeDepth;
} Compiler;

typedef void (*ParseFn)(Parser *p, Scanner *sc, Compiler* c, bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix; 
    Precedence precedence;
} ParseRule;

Compiler* initCompiler(TVM* tvm, Parser *p, FunctionType type);
void freeCompiler(Compiler* c);
ObjFunction* compile(Scanner* sc, Parser* p, TVM* tvm);

ido_uint32 ralloc(Compiler* c, Parser* p);
void freeR(Compiler* c, Parser* p, ido_uint32 r);
ido_uint32 getLastAllocatedRegister(Compiler* c);
void setLastAllocatedRegister(Compiler* c, ido_uint32 r);

// void initIntervalArray(LiveInterval* array);
// void writeIntervalArray(LiveInterval* array, ido_uint32 r, ido_uint32 start, ido_uint32 end);
// void freeIntervalArray(LiveInterval* array);

#endif
