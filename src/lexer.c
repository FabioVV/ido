#ifndef C_LEXER
#define C_LEXER

#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "common.h"
#include "memory.h"

Scanner* initScanner(const char* source){
    Scanner* sc = malloc(sizeof(Scanner));
    if(sc == NULL) exit(1);
    sc->start = source;
    sc->current = source;
    sc->line = 1;

    return sc;
}

void freeScanner(Scanner* sc){
    FREE(Scanner, sc);
}

Token errorToken(Scanner* sc, const char* message){
    Token token;
    token.type = T_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = sc->line;
    return token;
}

Token makeToken(Scanner* sc, TokenType type){
    Token token;
    token.type = type;
    token.start = sc->start;
    token.length = (int)(sc->current - sc->start);
    token.line = sc->line;
    return token;
}

static bool isDigit(char c){
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}


static bool isAtEnd(Scanner* sc){
    return *sc->current == '\0';
}

static char advance(Scanner* sc){
    sc->current++;
    return sc->current[-1];
}

static bool match(Scanner* sc, char expected){
    if(isAtEnd(sc)) return false;
    if(*sc->current != expected) return false;

    sc->current++;
    return true;
}

static char peek(Scanner* sc){
    return *sc->current;
}

static char peekNext(Scanner* sc){
    if(isAtEnd(sc)) return '\0';
    return sc->current[1];
}

static void skipWhitespaceAndComments(Scanner* sc){
    for(;;){
        char c = peek(sc);
        switch (c)
        {
        case ' ':
        case '\t':
        case '\r':
            advance(sc);
            break;
        case '\n':
            sc->line++;
            advance(sc);
            break;
        case '/':
            if(peekNext(sc) == '/'){
                while(peek(sc) != '\n' && !isAtEnd(sc)) advance(sc);
            } else {return;}
            break;
        default:
            return;
        }
    }
}

static Token string(Scanner* sc){
    while(peek(sc) != '"' && !isAtEnd(sc)){
        if(peek(sc) == '\n') sc->line++;
        advance(sc);
    }

    if(isAtEnd(sc)) return errorToken(sc, "unterminated string");

    advance(sc); // Closing "
    return makeToken(sc, T_STRING);
}

static Token numeric(Scanner* sc){
    while(isDigit(peek(sc))) advance(sc);

    bool isFractional = false;
    if(peek(sc) == '.' && isDigit(peekNext(sc))){
        advance(sc);
        while(isDigit(peek(sc))) advance(sc);
        isFractional = true;
    }
    
    return makeToken(sc, isFractional ? T_FLOAT : T_INT);
}

static TokenType checkKeyword(Scanner* sc, int start, int length, const char* rest, TokenType type){
    if(sc->current - sc->start == start + length && memcmp(sc->start + start, rest , length) == 0){
        return type;
    }

    return T_IDEN;
}

static TokenType identifierType(Scanner* sc){
    switch (*sc->start)
    {
    case 'a': return checkKeyword(sc, 1, 2, "nd", T_AND);
    case 'e': return checkKeyword(sc, 1, 3, "lse", T_ELSE);
    case 'i': return checkKeyword(sc, 1, 1, "F", T_IF);
    case 'f':
        if(sc->current - sc->start > 1){
            switch (*(sc->start + 1))
            {
            case 'a': return checkKeyword(sc, 2, 3, "lse", T_FALSE);
            case 'o': return checkKeyword(sc, 2, 1, "r", T_FOR);
            case 'n': return T_FN;
            } 
        }  
        break;
    case 'n': return checkKeyword(sc, 1, 2, "il", T_NIL);
    case 'o': return checkKeyword(sc, 1, 1, "r", T_OR);
    case 'p': return checkKeyword(sc, 1, 4, "rint", T_PRINT);
    case 'r': return checkKeyword(sc, 1, 5, "eturn", T_RETURN);
    case 't': 
        if(sc->current - sc->start > 1){
            switch (*(sc->start + 1))
            {
            case 'r': return checkKeyword(sc, 2, 2, "ue", T_TRUE);
            } 
        }  
        break;
    case 'v': return checkKeyword(sc, 1, 2, "ar", T_VAR);
    case 'w': return checkKeyword(sc, 1, 4, "hile", T_WHILE);

    }

    return T_IDEN;
}

static Token identifier(Scanner* sc){
    while(isAlpha(peek(sc))) advance(sc);
    return makeToken(sc, identifierType(sc));
}

Token scanToken(Scanner* sc){
    skipWhitespaceAndComments(sc);

    sc->start = sc->current;
    if(isAtEnd(sc)) return makeToken(sc, T_EOF);

    char c = advance(sc);

    if(isAlpha(c)) return identifier(sc); // Keywords, variables, etc...
    if(isDigit(c)) return numeric(sc); // Numeric chars

    switch (c)
    {
    case '(': return makeToken(sc, T_LEFT_PAREN);
    case ')': return makeToken(sc, T_RIGHT_PAREN);
    case '{': return makeToken(sc, T_LEFT_BRACE);
    case '}': return makeToken(sc, T_RIGHT_BRACE);
    case ';': return makeToken(sc, T_SEMICOLON);
    case ',': return makeToken(sc, T_COMMA);
    case '.': return makeToken(sc, T_DOT);
    case '-': return makeToken(sc, T_MINUS);
    case '+': return makeToken(sc, T_PLUS);
    case '/': return makeToken(sc, T_SLASH);
    case '*': return makeToken(sc, T_STAR);
    case '!': return makeToken(sc, match(sc, '=') ? T_BANG_EQUAL : T_BANG);
    case '=': return makeToken(sc, match(sc, '=') ? T_EQUAL_EQUAL : T_EQUAL);
    case '<': return makeToken(sc, match(sc, '=') ? T_LESS_EQUAL : T_LESS);
    case '>': return makeToken(sc, match(sc, '=') ? T_GREATER_EQUAL : T_GREATER);
    case '"': return string(sc);
    }


    return errorToken(sc, "unexpected character");
}

#endif