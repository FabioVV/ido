#ifndef C_LEXER
#define C_LEXER

#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "common.h"

Scanner initScanner(const char* source){
    Scanner sc;
    sc.start = source;
    sc.current = source;
    sc.line = 1;

    return sc;
}

Token errorToken(Scanner sc, const char* message){
    Token token;
    token.type = T_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = sc.line;
    return token;
}

Token makeToken(Scanner sc, TokenType type){
    Token token;
    token.type = type;
    token.start = sc.start;
    token.length = (int)strlen(sc.start - sc.current);
    token.line = sc.line;
    return token;
}

static bool isAtEnd(Scanner sc){
    return *sc.current == '\0';
}

static char advance(Scanner sc){
    sc.current++;
    return sc.current[-1];
}

static bool match(Scanner sc, char expected){
    if(isAtEnd(sc)) return false;
    if(*sc.current != expected) return false;
    
    sc.current++;
    return true;
}

#endif