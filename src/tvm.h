#ifndef H_TANIAVM
#define H_TANIAVM

#include <stdio.h>
#include "instruction.h"
#include "table.h"

#define REGISTERS_NUM 256
#define INVALID_REGISTER ((ido_uint32)-1)
#define IS_REGISTER_VALID(r) ((r) != INVALID_REGISTER)
#define IS_REGISTER_FREE(r)  ((r) >= 0 && (r) < REGISTERS_NUM)

typedef enum {
  INTERPRET_OK,
  INTERPRET_HALT,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
  INTERPRET_REGISTER_ERROR,
} InterpretResult;

typedef struct{
  Value registers[REGISTERS_NUM];
  ido_uint32 free_registers[REGISTERS_NUM];
  ido_uint32 free_register_count;
  ido_uint32 last_allocated_register;
  ido_uint32 last_result_register;

  Program* program; 
  ido_uint32* pc;
  Table strings;
  Table globals;
  Obj* objects;
} TVM;

void initVM(TVM* tvm);
void freeVM(TVM* tvm);
ido_uint32 allocR(TVM* tvm);
void freeR(TVM* tvm, ido_uint32 r);
ido_uint32 getLastAllocatedRegister(TVM* tvm);
ido_uint32 getLastRegisterResult(TVM* tvm);
void setLastAllocatedRegister(TVM* tvm, ido_uint32 r);
void setLastRegisterResult(TVM* tvm, ido_uint32 r);
InterpretResult interpret(TVM* tvm, const char* source);


#endif 
