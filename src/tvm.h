#ifndef IDO_TANIAVM
#define IDO_TANIAVM

typedef struct ObjFunction ObjFunction;

#include <stdio.h>
#include "instruction.h"
#include "table.h"
#include "parser.h"

#define REGISTERS_NUM 256
#define INVALID_REGISTER ((ido_uint32)-1)
#define IS_REGISTER_VALID(r) ((r) != INVALID_REGISTER)
#define IS_REGISTER_FREE(r)  ((r) >= 0 && (r) <= REGISTERS_NUM)

#define FRAMES_NUM 2024 
#define STACK_NUM 2048 

typedef enum {
  INTERPRET_OK,
  INTERPRET_HALT,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
  INTERPRET_REGISTER_ERROR,
} InterpretResult;

typedef struct {
  ObjFunction* function;
  Instruction* pc;
} CallFrame;

typedef struct{
  Value registers[REGISTERS_NUM];
  bool allocatedRegisters[REGISTERS_NUM];
  ido_uint32 free_register_count;
  ido_uint32 last_allocated_register;

  CallFrame frames[FRAMES_NUM];
  int frameCount;

  Value stack[STACK_NUM];
  Value* stackTop;

  // Program* program; 
  // ido_uint32* pc;
  Table strings;
  Table globals;
  Obj* objects;
} TVM;

TVM* initVM();
void freeVM(TVM* tvm);
InterpretResult interpret(TVM* tvm, Scanner* sc, Parser* p);



#endif 
