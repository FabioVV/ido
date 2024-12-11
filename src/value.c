#ifndef C_VALUE
#define C_VALUE

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "value.h"
#include "memory.h"
#include "value.h"
#include "object.h"

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
  case VAL_INUMBER: printf("%lu\n", AS_INUMBER(value)); break;
  case VAL_DNUMBER: printf("%f\n", AS_DNUMBER(value)); break;
  case VAL_NIL:     printf("nil\n"); break;
  case VAL_BOOL:    printf(AS_BOOL(value) ? "true\n" : "false\n"); break; 
  case VAL_OBJ:     printObject(value); break;
  default: return;
  }
}

bool valuesEqual(Value a, Value b){
  if(a.type != b.type) return false;
  switch(a.type){
  case VAL_BOOL:     return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NIL:      return true;
  case VAL_DNUMBER:  return AS_DNUMBER(a) == AS_DNUMBER(b);
  case VAL_INUMBER:  return AS_INUMBER(a) == AS_INUMBER(b);
  case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);    
  default:           return false;
  }
}

bool valuesGreaterEqual(Value a, Value b){
  if(a.type != b.type) return false;
  switch(a.type){
  case VAL_BOOL:     return AS_BOOL(a) >= AS_BOOL(b);
  case VAL_NIL:      return true;
  case VAL_DNUMBER:  return AS_DNUMBER(a) >= AS_DNUMBER(b);
  case VAL_INUMBER:  return AS_INUMBER(a) >= AS_INUMBER(b);
  default: return false; // Unreachable
  }
}

bool valuesLessEqual(Value a, Value b){
  if(a.type != b.type) return false;
  switch(a.type){
  case VAL_BOOL:       return AS_BOOL(a) <= AS_BOOL(b);
  case VAL_NIL:        return true;
  case VAL_DNUMBER:    return AS_DNUMBER(a) <= AS_DNUMBER(b);
  case VAL_INUMBER:    return AS_INUMBER(a) <= AS_INUMBER(b);
  default: return false; // Unreachable
  }
}

bool valuesNotEqual(Value a, Value b){
  if(a.type != b.type) return false;
  switch(a.type){
  case VAL_BOOL:       return AS_BOOL(a) != AS_BOOL(b);
  case VAL_NIL:        return true;
  case VAL_DNUMBER:    return AS_DNUMBER(a) != AS_DNUMBER(b);
  case VAL_INUMBER:    return AS_INUMBER(a) != AS_INUMBER(b);
  default: return false; // Unreachable
  }
}


#endif 
