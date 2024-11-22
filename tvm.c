// The tania VM
#ifndef C_TANIAVM
#define C_TANIAVM

#include <string.h>
#include <stdio.h>
#include "tvm.h"
#include "instruction.h"
#include "memory.h"
#include "compiler.h"

void initVM(TVM* tvm){
    memset(tvm->registers, 0, sizeof(tvm->registers));  
    // initProgram(&tvm->program);
    tvm->pc = 0;
}

void freeVM(TVM* tvm){

}


static uint16_t next_16_bits(TVM* tvm){
    tvm->pc += 2;
    uint16_t val = (uint16_t)(tvm->pc[-2] << 8 | tvm->pc[-1]);
    return val;
}

static InterpretResult runVM(TVM* tvm){

    #define NEXT_BYTE() (*tvm->pc++)
    #define NEXT_16_BITS(tvm) next_16_bits(tvm) 

    for(;;){
        switch (NEXT_BYTE())
        {
        case OP_LOAD:{
            uint8_t _register = NEXT_BYTE();
            uint16_t number = NEXT_16_BITS(tvm);
            tvm->registers[_register] = (uint32_t)number;
            continue;
        }
        case OP_ADD:{
            uint32_t _register1 = tvm->registers[NEXT_BYTE()];
            uint32_t _register2 = tvm->registers[NEXT_BYTE()];
            tvm->registers[NEXT_BYTE()] = _register1 + _register2;
            continue;
        }
        case OP_SUB:{
            uint32_t _register1 = tvm->registers[NEXT_BYTE()];
            uint32_t _register2 = tvm->registers[NEXT_BYTE()];
            tvm->registers[NEXT_BYTE()] = _register1 - _register2;
            continue;
        }

        case OP_MUL:{
            uint32_t _register1 = tvm->registers[NEXT_BYTE()];
            uint32_t _register2 = tvm->registers[NEXT_BYTE()];
            tvm->registers[NEXT_BYTE()] = _register1 * _register2;
            continue;
        }
        case OP_DIV:{
            uint32_t _register1 = tvm->registers[NEXT_BYTE()];
            uint32_t _register2 = tvm->registers[NEXT_BYTE()];
            tvm->registers[NEXT_BYTE()] = _register1 / _register2;
            tvm->remainder = _register1 % _register2;
            continue;
        }
        case OP_HLT:
            printf("HALTING APPLICATION\n");
            return INTERPRET_HALT;
        case OP_IGL:
            printf("ILLEGAL\n");
            return INTERPRET_RUNTIME_ERROR;
        default:
            printf("something has gone wrong. unrecognized opcode");
            return INTERPRET_RUNTIME_ERROR;
        }

    }
    #undef DECODE_OPCODE
    #undef NEXT_BYTE
    #undef NEXT_16_BITS
}

InterpretResult interpret(TVM* tvm, const char* source){
    Program program;
    initProgram(&program);

    if(!compile(source, &program)){
        freeProgram(&program);
        return INTERPRET_COMPILE_ERROR;
    }

    tvm->program = &program;
    tvm->pc = tvm->program->code;

    InterpretResult resultVM = runVM(tvm);

    freeProgram(&program);
    return resultVM;
}


#endif 
