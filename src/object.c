#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "value.h"
#include "tvm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObj(sizeof(type), objectType)\


static Obj* allocateObj(size_t size, ObjType type){
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    
    return object;
}

static ObjString* allocateString(const char* chars, int length){
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->chars = chars;
    string->length = length;
    return string;
}

ObjString* takeString(char* chars, int length){
    return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length){
    char* heapChars = ALLOCATE(char, length+1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

void printObject(Value v){
    switch (OBJ_TYPE(v))
    {
    case OBJ_STRING:
        printf("%s", AS_CSTRING(v));
        break;
    default: return;
    }
}