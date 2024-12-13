#ifndef H_COMPILER
#define H_COMPILER

#include "tvm.h"
#include "token.h"
#include "common.h"
#include "lexer.h"

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

typedef struct {
    Token name;
    int depth;
} Local;

typedef struct {
    Local locals[LOCALS_NUM];// What a strange limit...
    int localCount;
    int scopeDepth;
} Compiler;

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;

    TVM* tvm;
} Parser;

typedef void (*ParseFn)(Parser *p, Scanner *sc, Compiler* c, bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix; 
    Precedence precedence;
} ParseRule;

Compiler* initCompiler();
void freeCompiler(Compiler* compiler);
bool compile(const char* source, Program* program, TVM* tvm);
Parser* initParser(TVM* tvm);

#endif
