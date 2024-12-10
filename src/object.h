#ifndef H_OBJECT
#define H_OBJECT

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)    (AS_OBJ(value)->type)
#define IS_STRING(value)   isObjType(value, OBJ_STRING)

#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
} ObjType; 

struct Obj {
    ObjType type;
    struct Obj* next;
}; 

struct ObjString {
    Obj obj;
    int length;
    char* chars;
};

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);
void printObject(Value v);

static inline bool isObjType(Value v, ObjType t){
    return IS_OBJ(v) && AS_OBJ(v)->type == t;
}

#endif