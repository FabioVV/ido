#ifndef C_MEMORY
#define C_MEMORY

#include "memory.h"
#include "object.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize){
    if(newSize == 0) {
        free(pointer);
        return NULL;
    }
    
    void* result = realloc(pointer, newSize);
    if(result == NULL) exit(1);
    return result;
}

static void freeObject(Obj* object){
    switch (object->type)
    {
    case OBJ_STRING:{
        ObjString* string = (ObjString*)object;
        FREE_ARRAY(char, string->chars, string->length+1);
        FREE(ObjString, object);
        break;
    }
    default: return;
    }
}

void freeObjects(TVM* tvm){
  Obj* object = tvm->objects;

  while (object != NULL) {
    Obj* next = object->next;
    freeObject(object);
    object = next;
  }
}   


#endif