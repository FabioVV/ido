#ifndef H_VALUE
#define H_VALUE

#include "common.h"

typedef enum {
    VAL_BOOL,
    VAL_NIL, 
    VAL_INUMBER,
    VAL_DNUMBER,
} ValueType;

typedef struct {
    ValueType type;
    union 
    {
        bool boolean;
        long inumber;
        double dnumber;
    } as;
} Value;

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

// C VALUE TO IDO VALUE
#define BOOL_VAL(value)    ((Value){VAL_BOOL, {.boolean = value}})
#define INUMBER_VAL(value) ((Value){VAL_INUMBER, {.inumber = value}})
#define DNUMBER_VAL(value) ((Value){VAL_DNUMBER, {.dnumber = value}})
#define NIL_VAL()     ((Value){VAL_NIL, {.inumber = 0}})

// IDO VALUE TO C VALUE
#define AS_BOOL(value)     ((value).as.boolean)
#define AS_INUMBER(value)  ((value).as.inumber)
#define AS_DNUMBER(value)  ((value).as.dnumber)

// CHECK FOR TYPES
#define IS_BOOL(value)     ((value).type == VAL_BOOL)
#define IS_INUMBER(value)  ((value).type == VAL_INUMBER)
#define IS_DNUMBER(value)  ((value).type == VAL_DNUMBER)
#define IS_NIL(value)      ((value).type == VAL_NIL)


void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

void printValue(Value value);
#endif 
