#ifndef H_TOKEN
#define H_TOKEN

typedef enum {
    T_INT, // 1, 2, 3 ...
    T_FLOAT, // 1.5, 2.6, 3.3 ...

    T_LEFT_PAREN, // (
    T_RIGHT_PAREN, // )

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