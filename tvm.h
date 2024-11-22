#ifndef H_TANIAVM
#define H_TANIAVM

#include "instruction.h"

#define REGISTERS_NUM 32 // Number of registers the Tania vm has

typedef enum {
  INTERPRET_OK,
  INTERPRET_HALT,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct{
    uint32_t registers[REGISTERS_NUM];
    Program* program; 
    uint8_t* pc;
    int32_t remainder;
} TVM;

void initVM(TVM* tvm);
void freeVM(TVM* tvm);
InterpretResult interpret(TVM* tvm);


#endif 