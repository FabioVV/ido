#ifndef IDO_PARSER
#define IDO_PARSER

#include "stdbool.h"
#include "token.h"
#include "lexer.h"


typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;


Parser* initParser();
void freeParser(Parser* p);

#endif
