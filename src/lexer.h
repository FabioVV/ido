#ifndef H_LEXER
#define H_LEXER

#include "token.h"

typedef struct {
    const char* start; // Starting character
    const char* current; // Current character
    int line;
} Scanner;

Scanner* initScanner(const char* source);
void freeScanner(Scanner* sc);

Token scanToken(Scanner* sc);
Token errorToken(Scanner* sc, const char* message);
Token makeToken(Scanner* sc, TokenType type);

#endif
