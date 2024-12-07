#ifndef C_COMPILER
#define C_COMPILER

#include <stdio.h>
#include "tvm.h"
#include "lexer.h"
#include "compiler.h"
#include "token.h"
#include "value.h"
#include "instruction.h"


static ParseRule* getRule(TokenType t);
static void parsePrecedence(Parser *p, Scanner *sc, Precedence prec);
static void expression(Parser *p, Scanner *sc);

Program* compilingProgram;

Parser* initParser(TVM* tvm){
    Parser* p = malloc(sizeof(Parser));
    if(p == NULL) exit(1);
    p->tvm = tvm;
    return p;
}

static Program* currentProgram(){
    return compilingProgram;
}

static void errorAt(Parser* p, Token* token, const char* message){
    if(p->panicMode) return;
    p->panicMode = true;

    fprintf(stderr, "[line %d] error", token->line);

    if(token->type == T_EOF){
        fprintf(stderr, " at end");
    } else if(token->type == T_ERROR){

    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
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

static void emitReturn(Parser* p){
    writeToProgram(currentProgram(), ENC_RETURN(), p->previous.line);
}

static void endCompilation(Parser* p){
    emitReturn(p);
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

static ido_uint32 createConstant(Parser* p, Value v){
    ido_uint32 constantIndex = addConstant(currentProgram(), v);
    if(constantIndex > UINT32_MAX){
        error(p, "too many constants in one program");
        return 0;
    }

    return constantIndex;
}

static ido_uint32 emitConstant(Parser *p, Value v){
    ido_uint32 constantIndex = createConstant(p, v);
    ido_uint32 r = allocR(p->tvm); // Allocate a free register
    setLastAllocatedRegister(p->tvm, r);
    writeToProgram(currentProgram(), ENC_CONSTANT(constantIndex, r), p->previous.line);
    return constantIndex;
}

static void parsePrecedence(Parser *p, Scanner *sc, Precedence prec){
    advance(p, sc);
    ParseFn prefixRule = getRule(p->previous.type)->prefix;

    if(prefixRule == NULL){
        error(p, "expect expression");
        return;
    }

    ido_uint32 lR = prefixRule(p, sc, 0);

    while(prec <= getRule(p->current.type)->precedence){
        advance(p, sc);
        ParseFn infixRule = getRule(p->previous.type)->infix;
        infixRule(p, sc, lR);
    }
}

static ido_uint32 number(Parser *p, Scanner *sc, ido_uint32 lR){
    double value = strtod(p->previous.start, NULL);
    Value v = DNUMBER_VAL(value);
    ido_uint32 constantIndex = emitConstant(p, v);
}

static ido_uint32 unary(Parser *p, Scanner *sc, ido_uint32 lR){
    TokenType opType = p->previous.type;
    parsePrecedence(p, sc, PREC_UNARY);

    switch (opType)
    {
    case T_MINUS: 
        // emit bytecode for unary negation here
        break;
    
    default: return 0;
    }
}

static void expression(Parser *p, Scanner *sc){
    parsePrecedence(p, sc, PREC_ASSIGNMENT);
}

static ido_uint32 grouping(Parser *p, Scanner *sc, ido_uint32 lR){
    expression(p, sc);
    consume(p, sc, T_RIGHT_PAREN, "expect ')' after expression");
}

static ido_uint32 binary(Parser *p, Scanner *sc, ido_uint32 lR){
    TokenType opType = p->previous.type;
    ParseRule* rule = getRule(opType);

    ido_uint32 leftR = (getLastRegisterResult(p->tvm) != -1) ? getLastRegisterResult(p->tvm) : getLastAllocatedRegister(p->tvm);
    parsePrecedence(p, sc, (Precedence)rule->precedence+1);

    ido_uint32 rightR = getLastAllocatedRegister(p->tvm);
    ido_uint32 resultR = allocR(p->tvm);


    switch (opType)
    {
    case T_PLUS:
        writeToProgram(currentProgram(), ENC_ADD(resultR, leftR, rightR), p->previous.line);
        break;
    case T_MINUS:
        writeToProgram(currentProgram(), ENC_SUB(resultR, leftR, rightR), p->previous.line);
        break;
    case T_STAR:
        writeToProgram(currentProgram(), ENC_MUL(resultR, leftR, rightR), p->previous.line);
        break;
    case T_SLASH:
        writeToProgram(currentProgram(), ENC_DIV(resultR, leftR, rightR), p->previous.line);
        break;
    default: return 0;
    }

    // freeR(p->tvm, lR);

    freeR(p->tvm, leftR);
    freeR(p->tvm, rightR);
    // freeR(p->tvm, resultR);

    setLastRegisterResult(p->tvm, resultR);
    setLastAllocatedRegister(p->tvm, resultR);

    return resultR;
}

ParseRule rules[] = {
  [T_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [T_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [T_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [T_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [T_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [T_DOT]           = {NULL,     NULL,   PREC_NONE},
  [T_MINUS]         = {unary,    binary, PREC_TERM},
  [T_PLUS]          = {NULL,     binary, PREC_TERM},
  [T_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [T_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [T_STAR]          = {NULL,     binary, PREC_FACTOR},
  [T_BANG]          = {NULL,     NULL,   PREC_NONE},
  [T_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [T_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [T_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [T_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [T_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [T_LESS]          = {NULL,     NULL,   PREC_NONE},
  [T_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [T_IDEN]          = {NULL,     NULL,   PREC_NONE},
  [T_STRING]        = {NULL,     NULL,   PREC_NONE},
  [T_FLOAT]         = {number,   NULL,   PREC_NONE},
  [T_INT]           = {number,   NULL,   PREC_NONE},
  [T_AND]           = {NULL,     NULL,   PREC_NONE},
  [T_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [T_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [T_FOR]           = {NULL,     NULL,   PREC_NONE},
  [T_FN]            = {NULL,     NULL,   PREC_NONE},
  [T_IF]            = {NULL,     NULL,   PREC_NONE},
  [T_NIL]           = {NULL,     NULL,   PREC_NONE},
  [T_OR]            = {NULL,     NULL,   PREC_NONE},
  [T_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [T_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [T_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [T_VAR]           = {NULL,     NULL,   PREC_NONE},
  [T_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [T_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [T_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static ParseRule* getRule(TokenType t){
    return &rules[t];
}

bool compile(const char* source, Program* program, TVM* tvm){
    Scanner* sc = initScanner(source);
    Parser* p = initParser(tvm);
    compilingProgram = program;

    p->panicMode = false;
    p->hadError = false;

    advance(p, sc);
    expression(p, sc);
    consume(p, sc, T_EOF, "expect end of expression");

    endCompilation(p);
    return !p->hadError;
}

#endif
