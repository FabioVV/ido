#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "value.h"
#include "tvm.h"

#define ALLOCATE_OBJ(tvm, type, objectType) \
    (type*)allocateObj(tvm, sizeof(type), objectType)\


static Obj* allocateObj(TVM* tvm, size_t size, ObjType type){
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = tvm->objects;
    tvm->objects = object;
    
    return object;
}

static ObjString* allocateString(TVM* tvm, char* chars, int length){
    ObjString* string = ALLOCATE_OBJ(tvm, ObjString, OBJ_STRING);
    string->chars = chars;
    string->length = length;
    return string;
}

ObjString* takeString(TVM* tvm, char* chars, int length){
    return allocateString(tvm, chars, length);
}

ObjString* copyString(TVM* tvm, const char* chars, int length){
    char* heapChars = ALLOCATE(char, length+1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(tvm, heapChars, length);
}

void printObject(Value v){
    switch (OBJ_TYPE(v))
    {
    case OBJ_STRING:
        printf("%s\n", AS_CSTRING(v));
        break;
    default: return;
    }
}