#ifndef H_TANIAVM
#define H_TANIAVM

#include "instruction.h"

#define REGISTERS_NUM 32

typedef struct{
    uint32_t registers[REGISTERS_NUM];
    Program program; 
    size_t pc;
    int32_t remainder;
} TVM;

void initVM(TVM* tvm);
int runVM(TVM* tvm);
void freeVM(TVM* tvm);

#endif 