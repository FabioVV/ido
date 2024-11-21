// The tania VM
#ifndef C_TANIAVM
#define C_TANIAVM

#include <string.h>
#include <stdio.h>
#include "tvm.h"
#include "instruction.h"
#include "memory.h"

void initVM(TVM* tvm){
    memset(tvm->registers, 0, sizeof(tvm->registers));  
    initProgram(&tvm->program);
    tvm->pc = 0;
}

void freeVM(TVM* tvm){
    freeProgram(&tvm->program);
    free(tvm);
}

Opcode decode_op(TVM* tvm){
    Opcode op = (Opcode)tvm->program.code[tvm->pc];
    tvm->pc += 1;
    return op;
}

uint16_t next_16_bits(TVM* tvm){
    uint16_t val = (uint16_t)(tvm->program.code[tvm->pc] << 8) | (uint16_t)(tvm->program.code[tvm->pc + 1]);
    tvm->pc += 2;
    return val;
}

int runVM(TVM* tvm){
    #define DECODE_OPCODE(tvm) \
        decode_op(tvm)
    
    #define NEXT_8_BITS(tvm) (tvm->program.code[tvm->pc++])
    #define NEXT_16_BITS(tvm) next_16_bits(tvm)


    for(;;){
        if(tvm->pc > tvm->program.capacity){
            printf("Program count is greater than program capacity");
            exit(1);
        }

        switch (DECODE_OPCODE(tvm))
        {
        case OP_LOAD:{
            uint8_t _register = NEXT_8_BITS(tvm);
            uint16_t number = NEXT_16_BITS(tvm);
            tvm->registers[_register] = (uint32_t)number;
            continue;
        }
        case OP_ADD:{
            uint32_t _register1 = tvm->registers[NEXT_8_BITS(tvm)];
            uint32_t _register2 = tvm->registers[NEXT_8_BITS(tvm)];
            tvm->registers[NEXT_8_BITS(tvm)] = _register1 + _register2;

            continue;
        }
        case OP_SUB:{
            uint32_t _register1 = tvm->registers[NEXT_8_BITS(tvm)];
            uint32_t _register2 = tvm->registers[NEXT_8_BITS(tvm)];
            tvm->registers[NEXT_8_BITS(tvm)] = _register1 - _register2;

            continue;
        }

        case OP_MUL:{
            uint32_t _register1 = tvm->registers[NEXT_8_BITS(tvm)];
            uint32_t _register2 = tvm->registers[NEXT_8_BITS(tvm)];
            tvm->registers[NEXT_8_BITS(tvm)] = _register1 * _register2;

            continue;
        }
        case OP_DIV:{
            uint32_t _register1 = tvm->registers[NEXT_8_BITS(tvm)];
            uint32_t _register2 = tvm->registers[NEXT_8_BITS(tvm)];
            tvm->registers[NEXT_8_BITS(tvm)] = _register1 / _register2;
            tvm->remainder = _register1 % _register2;
            continue;
        }
        case OP_HLT:
            printf("HALTING APPLICATION\n");
            return 0;
        case OP_IGL:
            printf("ILLEGAL\n");
            return 1;
        default:
            printf("something has gone wrong. unrecognized opcode");
            exit(1);
        }

    }
    #undef DECODE_OPCODE
    #undef NEXT_8_BITS
    #undef NEXT_16_BITS

}

#endif 
