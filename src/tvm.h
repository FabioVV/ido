#ifndef H_TANIAVM
#define H_TANIAVM

#include "instruction.h"

#define REGISTERS_NUM 32

typedef enum {
  INTERPRET_OK,
  INTERPRET_HALT,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct{
  ido_uint32 registers[REGISTERS_NUM];
  Program* program; 
  ido_uint32* pc;
} TVM;

void initVM(TVM* tvm);
void freeVM(TVM* tvm);
InterpretResult interpret(TVM* tvm, const char* source);


#endif 