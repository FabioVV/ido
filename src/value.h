#ifndef H_VALUE
#define H_VALUE

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
    VAL_BOOL,
    VAL_NIL, 
    VAL_INUMBER,
    VAL_DNUMBER,
    VAL_OBJ,
} ValueType;

typedef struct {
    ValueType type;
    union 
    {
        bool boolean;
        long inumber;
        double dnumber;
        Obj* obj
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
#define NIL_VAL()          ((Value){VAL_NIL, {.inumber = 0}})
#define OBJ_VAL(object)    ((Value){VAL_OBJ, {.obj = (Obj*)object}})

// IDO VALUE TO C VALUE
#define AS_BOOL(value)     ((value).as.boolean)
#define AS_INUMBER(value)  ((value).as.inumber)
#define AS_DNUMBER(value)  ((value).as.dnumber)
#define AS_OBJ(value)      ((value).as.obj)


// CHECK FOR TYPES
#define IS_BOOL(value)     ((value).type == VAL_BOOL)
#define IS_INUMBER(value)  ((value).type == VAL_INUMBER)
#define IS_DNUMBER(value)  ((value).type == VAL_DNUMBER)
#define IS_NUMBER(value)   (((value).type == VAL_DNUMBER) || ((value).type == VAL_INUMBER))
#define IS_NIL(value)      ((value).type == VAL_NIL)
#define IS_OBJ(value)      ((value).type == VAL_OBJ)

// Get type
#define GET_TYPE(val) ((val).type)

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

void printValue(Value value);

bool valuesEqual(Value a, Value b);
bool valuesLessEqual(Value a, Value b);
bool valuesGreaterEqual(Value a, Value b);
bool valuesNotEqual(Value a, Value b);
#endif 
