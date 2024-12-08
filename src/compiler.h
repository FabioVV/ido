#ifndef H_COMPILER
#define H_COMPILER

#include "tvm.h"
#include "token.h"
#include "common.h"
#include "lexer.h"

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
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;

    TVM* tvm;
} Parser;

typedef void (*ParseFn)(Parser *p, Scanner *sc);

typedef struct {
    ParseFn prefix;
    ParseFn infix; 
    Precedence precedence;
} ParseRule;

bool compile(const char* source, Program* program, TVM* tvm);
Parser* initParser(TVM* tvm);

#endif
