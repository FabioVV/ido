#ifndef H_COMPILER
#define H_COMPILER

#include "tvm.h"
#include "token.h"
#include "common.h"

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

typedef void (*ParseFn)(Parser *p, Scanner *sc);

typedef struct {
    ParseFn prefix;
    ParseFn infix; 
    Precedence precedence;
} ParseRule;

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

bool compile(const char* source, Program* program);
Parser* initParser();

#endif