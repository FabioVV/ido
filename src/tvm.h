#ifndef H_TANIAVM
#define H_TANIAVM

#include "instruction.h"

#define REGISTERS_NUM 256

typedef enum {
  INTERPRET_OK,
  INTERPRET_HALT,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct{
  Value registers[REGISTERS_NUM];
  ido_uint32 free_registers[REGISTERS_NUM];
  ido_uint32 used_registers[REGISTERS_NUM];
  ido_uint32 last_allocated_register;

  Program* program; 
  ido_uint32* pc;
} TVM;

void initVM(TVM* tvm);
void freeVM(TVM* tvm);
ido_uint32 allocR(TVM* tvm);
ido_uint32 freeR(TVM* tvm, ido_uint32 r);
ido_uint32 getLastAllocatedRegister(TVM* tvm);
void setLastAllocatedRegister(TVM* tvm, ido_uint32 r);
InterpretResult interpret(TVM* tvm, const char* source);


#endif 