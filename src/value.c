#ifndef C_VALUE
#define C_VALUE

#include <stdio.h>
#include "common.h"
#include "value.h"
#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array){
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value){
    if(array->capacity < array->count + 1){
        int oldCap = array->capacity;
        array->capacity = GROW_CAPACITY(oldCap);
        array->values = GROW_ARRAY(Value, array->values, oldCap, array->capacity);
    }


    array->values[array->count] = value;
    array->count++;

}

void freeValueArray(ValueArray* array){
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value){
    switch (value.type)
    {
    case VAL_INUMBER:{
        printf("%lu", AS_INUMBER(value));
        break;

    }
    case VAL_DNUMBER:{
        printf("%f", AS_DNUMBER(value));
        break;
    }
        
    default: return;
    }
}

#endif 
