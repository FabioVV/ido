#ifndef H_TANIAVM
#define H_TANIAVM

#include <stdio.h>
#include "instruction.h"
#include "table.h"
#include "parser.h"

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

  ido_uint32 allocated_registers_debug[REGISTERS_NUM];
  ido_uint32 freed_registers_debug[REGISTERS_NUM];


  Program* program; 
  ido_uint32* pc;
  Table strings;
  Table globals;
  Obj* objects;
} TVM;

TVM* initVM();
void freeVM(TVM* tvm);
ido_uint32 allocR(TVM* tvm);
void freeR(TVM* tvm, ido_uint32 r);
ido_uint32 getLastAllocatedRegister(TVM* tvm);
void setLastAllocatedRegister(TVM* tvm, ido_uint32 r);
InterpretResult interpret(TVM* tvm, Scanner* sc, Parser* p);


#endif 
