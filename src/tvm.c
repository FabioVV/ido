// The tania VM
#ifndef C_TANIAVM
#define C_TANIAVM

#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include "tvm.h"
#include "instruction.h"
#include "compiler.h"
#include "value.h"


void initVM(TVM* tvm){
    for (int i = 0; i < REGISTERS_NUM; i++) {
        tvm->registers[i] = NIL_VAL();
        tvm->free_registers[i] = i;  // All registers are initially free (may change)
    }

    tvm->last_allocated_register = INVALID_REGISTER; // Initialize with an invalid register
    tvm->last_result_register = INVALID_REGISTER; // Initialize with an invalid register
    tvm->free_register_count = REGISTERS_NUM;
    tvm->pc = 0;
}

void freeVM(TVM* tvm){

}

static void runtimeErr(TVM* tvm, const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t inst = tvm->pc - tvm->program->code - 1;
    int line = tvm->program->lines[inst];
    fprintf(stderr, "[line %d] in script \n", line);
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
    if(tvm->free_register_count == 0){
        fprintf(stderr, "registererr: no free registers for op\n");
        exit(1);
    }
    return tvm->free_registers[--tvm->free_register_count];
}

void freeR(TVM* tvm, ido_uint32 r){
    if(!IS_REGISTER_FREE(r)){
        fprintf(stderr, "registererr: attempted to free invalid register %u\n", r);
        exit(1);
    }
    tvm->free_registers[tvm->free_register_count++] = r;
}

static InterpretResult runVM(TVM* tvm){

    #define ibreak break
    #define GET_CONSTANT(index) (tvm->program->constants.values[index])
    #define BINARY_OP(op) \
        do { \
            ido_uint32 rD = DEC_REGISTER_DEST(i);\
            Value rA = !IS_DNUMBER(tvm->registers[DEC_REGISTER_RA(i)]) ? GET_CONSTANT(AS_INUMBER(tvm->registers[DEC_REGISTER_RA(i)])): tvm->registers[DEC_REGISTER_RA(i)];\
            Value rB = !IS_DNUMBER(tvm->registers[DEC_REGISTER_RB(i)]) ? GET_CONSTANT(AS_INUMBER(tvm->registers[DEC_REGISTER_RB(i)])): tvm->registers[DEC_REGISTER_RB(i)];\
            if(!IS_NUMBER(rA) || !IS_NUMBER(rB)){\
                runtimeErr(tvm, "matherr: operands must be numbers");\
                return INTERPRET_RUNTIME_ERROR;\
            }\
            tvm->registers[rD] = DNUMBER_VAL(rA.as.dnumber op rB.as.dnumber);\
            printf("result: %f\n", AS_DNUMBER(tvm->registers[rD]));\
            setLastRegisterResult(tvm, INVALID_REGISTER);\
        } while(false)\

    for(;;){
        register Instruction i = NEXT_INSTRUCTION(tvm);

        switch (GET_OPCODE(i))
        {
        case OP_CONSTANT:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            ido_uint32 constantIndex = DEC_CONSTANT_INDEX(i);
            tvm->registers[r] = INUMBER_VAL(constantIndex);
            ibreak;
        }
        case OP_ADD:{BINARY_OP(+); ibreak;}
        case OP_SUB:{BINARY_OP(-); ibreak;}
        case OP_MUL:{BINARY_OP(*); ibreak;}
        case OP_DIV:{BINARY_OP(/); ibreak;}
        case OP_NEG:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            Value v = GET_CONSTANT(AS_INUMBER(tvm->registers[r]));
            tvm->program->constants.values[AS_INUMBER(tvm->registers[r])] = DNUMBER_VAL(-v.as.dnumber);
            ibreak;
        }
        case OP_TRUE:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            tvm->registers[r] = BOOL_VAL(true);
            setLastRegisterResult(tvm, INVALID_REGISTER);
            ibreak;
        }
        case OP_FALSE:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            tvm->registers[r] = BOOL_VAL(false);
            setLastRegisterResult(tvm, INVALID_REGISTER);
            ibreak;
        }
        case OP_NIL:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            tvm->registers[r] = NIL_VAL();
            setLastRegisterResult(tvm, INVALID_REGISTER);
            ibreak;
        }
        case OP_RETURN:{
            return INTERPRET_OK;
            ibreak;
        }
        case OP_HLT:
            printf("HALTING...\n");
            return INTERPRET_HALT;
        default:
            runtimeErr(tvm, "opcode: something has gone wrong. unrecognized code of operation");
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
