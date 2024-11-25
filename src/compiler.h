#ifndef H_COMPILER
#define H_COMPILER

#include "tvm.h"
#include "token.h"
#include "common.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;

} Parser;

bool compile(const char* source, Program* program);
Parser* initParser();

#endif