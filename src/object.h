#ifndef IDO_OBJECT
#define IDO_OBJECT

#include "common.h"
#include "compiler.h"
#include "value.h"
#include "instruction.h"
#include "tvm.h"


#define OBJ_TYPE(value)    (AS_OBJ(value)->type)
#define IS_STRING(value)   isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)

#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
} ObjType; 

struct Obj {
    ObjType type;
    struct Obj* next;
}; 


// basically the same idea from locals in the compiler, the difference here is that
// i needed to do this because after the `locals` (read parameters) and the fuction
// had been compiled, i had no way of accesing those locals to initialize them with
// arguments, because once the compilations of the function was done, its `compiler`
// would be lost
typedef struct {
    Token name;
    uint8_t registerIndex;
} Parameter;

struct ObjFunction{
    Obj obj;
    int arity;
    int parametersCount;
    Parameter parameters[LOCALS_NUM];
    Program program;
    ObjString* name;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

ObjFunction* newFunction(TVM* tvm);
ObjString* copyString(TVM* tvm, const char* chars, int length);
ObjString* takeString(TVM* tvm, char* chars, int length);
void printObject(Value v);

static inline bool isObjType(Value v, ObjType t){
    return IS_OBJ(v) && AS_OBJ(v)->type == t;
}

#endif
