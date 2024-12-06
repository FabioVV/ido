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
    for (int i = 0; i < REGISTERS_NUM; i++) {
        tvm->free_registers[i] = i;  // All registers are initially free (may change)
        tvm->used_registers[i] = -1; // None are in use
    }
    tvm->last_allocated_register = -1; // Initialize with an invalid register
    tvm->last_result_register = -1; // Initialize with an invalid register
    
    tvm->pc = 0;
}

void freeVM(TVM* tvm){

}

ido_uint32 getLastAllocatedRegister(TVM* tvm){
    if(tvm->last_allocated_register == -1){
        fprintf(stderr, "registererr: no register allocated for eval.\n");
        exit(1);
    }
    return tvm->last_allocated_register;
}

void setLastAllocatedRegister(TVM* tvm, ido_uint32 r){
    tvm->last_allocated_register = r;
}

ido_uint32 getLastRegisterResult(TVM* tvm){
    return tvm->last_result_register;
}

void setLastRegisterResult(TVM* tvm, ido_uint32 r){
    tvm->last_result_register = r;
}

ido_uint32 allocR(TVM* tvm){
    for(int i = 0; i < REGISTERS_NUM; i++){
        if(tvm->free_registers[i]){
            tvm->free_registers[i] = false;
            return i;
        }
    }
    fprintf(stderr, "registererr: no free registers for op\n");
    exit(1);
}

ido_uint32 freeR(TVM* tvm, ido_uint32 r){
    if(r < REGISTERS_NUM){
        tvm->free_registers[r] = true;
    } else {
        fprintf(stderr, "registererr: attempted to free invalid register %u\n", r);
    }
}

static InterpretResult runVM(TVM* tvm){

    #define ibreak break

    #define GET_CONSTANT(index) (tvm->program->constants.values[index])

    for(;;){
        // if(tvm->pc > (tvm->program->code)){
        //     return INTERPRET_OK;
        // }

        register Instruction i = NEXT_INSTRUCTION(tvm);

        switch (GET_OPCODE(i))
        {
        case OP_CONSTANT:{
            ido_uint32 r = DEC_REGISTER_C(i);
            ido_uint32 constantIndex = DEC_CONSTANT_INDEX(i); 
            tvm->registers[r] = INUMBER_VAL(constantIndex);
            // printf("constant ");
            // printValue(GET_CONSTANT(constantIndex));
            // printf(" loaded in R%i\n", r);
            ibreak;
        }
        case OP_ADD:{
            // printf("ADD OP: \n");
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            Value rA = GET_CONSTANT(AS_INUMBER(tvm->registers[DEC_REGISTER_RA(i)]));
            Value rB = GET_CONSTANT(AS_INUMBER(tvm->registers[DEC_REGISTER_RB(i)]));

            printf("%i %i %i", rD, DEC_REGISTER_RA(i), DEC_REGISTER_RB(i));

            tvm->registers[rD] = DNUMBER_VAL(rA.as.dnumber + rB.as.dnumber);
            printf("result = %f\n", AS_DNUMBER(tvm->registers[rD]));

            ibreak;
        }
        case OP_RETURN:{
            return INTERPRET_OK;
            ibreak;
        }
        case OP_HLT:
            printf("HALTING APPLICATION\n");
            return INTERPRET_HALT;
        case OP_ILG:
            printf("ILLEGAL\n");
            return INTERPRET_RUNTIME_ERROR;
        default:
            printf("something has gone wrong. unrecognized opcode");
            return INTERPRET_RUNTIME_ERROR;
        }

    }

    #undef ibreak
}

InterpretResult interpret(TVM* tvm, const char* source){
    Program program;
    initProgram(&program);

    if(!compile(source, &program, tvm)){
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
