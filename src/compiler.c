#ifndef C_COMPILER
#define C_COMPILER

#include <stdio.h>
#include "tvm.h"
#include "lexer.h"
#include "compiler.h"

Parser* initParser(){
    Parser* p = malloc(sizeof(Parser));
    if(p == NULL) exit(1);
    return p;
}

static void errorAt(Parser* p, Token* token, const char* message){
    if(p->panicMode) return;
    p->panicMode = true;

    fprintf(stderr, "[line %d] error", token->line);

    if(token->type == T_EOF){
        fprintf(stderr, " at end");
    } else if(token->type == T_ERROR){

    } else {
        fprintf(stderr, "at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    
    p->hadError = true;
}

static void error(Parser* p, const char* message){
    errorAt(p, &p->previous, message);
}

static void errorAtCurrent(Parser* p, const char* message){
    errorAt(p, &p->current, message);
}

static void advance(Parser* p, Scanner* sc){
    p->previous = p->current;
    for(;;){
        p->current = scanToken(sc);
        if(p->current.type != T_ERROR) break;
        errorAtCurrent(p, p->current.start);
    }
}

static void consume(Parser* p, Scanner* sc, TokenType type, const char* message){
    if(p->current.type == type){
        advance(p, sc);
        return;
    }

    errorAtCurrent(p, message);
}

bool compile(const char* source, Program* program){
    Scanner* sc = initScanner(source);
    Parser* p = initParser();
    p->panicMode = false;
    p->hadError = false;


    advance(p, sc);
    expression();
    consume(p, sc, T_EOF, "expect end of expression");
    return !p->hadError;
}

#endif


