#ifndef H_TOKEN
#define H_TOKEN

typedef enum {
    T_INT, // 1, 2, 3 ...
    T_FLOAT, // 1.5, 2.6, 3.3 ...
    T_STRING, // "String stuff"

    T_IF, // if ...
    T_ELSE, // else ...
    T_FALSE, // false ...
    T_TRUE, // true ...
    T_NIL, // nil ...

    T_FN, // fn ...
    T_RETURN,  // return ...

    T_PRINT, // print ...

    T_WHILE, // while ...
    T_FOR, // for ...

    T_LEFT_PAREN, // (
    T_RIGHT_PAREN, // )
    T_LEFT_BRACE, // {
    T_RIGHT_BRACE, // }

    T_AND, // && and
    T_OR, // || or
    T_BANG, // !
    T_EQUAL, // =
    T_EQUAL_EQUAL, // ==
    T_BANG_EQUAL, // !=
    T_LESS, // <
    T_GREATER, // >
    T_LESS_EQUAL, // <=
    T_GREATER_EQUAL, // >=

    T_PLUS, // +
    T_MINUS, // -
    T_STAR, // *
    T_SLASH, // /

    T_DOT, // .
    T_COMMA,  // ,
    T_SEMICOLON, // ;

    T_IDEN, // Identifiers (variables, constants etc)
    T_VAR,

    T_ERROR, // Special error token
    T_EOF // Special end of file token
} TokenType;

typedef struct {
    TokenType type;
    const char* start; // Starting position of the occurrance in the source code
    int line;
    int length;
} Token;

#endif