// The tania VM
#ifndef C_TANIAVM
#define C_TANIAVM

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "tvm.h"
#include "instruction.h"
#include "compiler.h"
#include "value.h"
#include "object.h"
#include "memory.h"


void initVM(TVM* tvm){
    for (int i = 0; i < REGISTERS_NUM; i++) {
        tvm->registers[i] = NIL_VAL();
        tvm->free_registers[i] = i;  // All registers are initially free (may change)
    }

    tvm->last_allocated_register = INVALID_REGISTER; // Initialize with an invalid register
    tvm->last_result_register = INVALID_REGISTER; // Initialize with an invalid register
    tvm->free_register_count = REGISTERS_NUM;
    tvm->pc = 0;
    tvm->objects = NULL;
    initTable(&tvm->strings);
}

void freeVM(TVM* tvm){
    freeTable(&tvm->strings);
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

static bool isFalsey(Value v){
    return IS_NIL(v) || (IS_BOOL(v) && !AS_BOOL(v));
}

static inline void concatenate(TVM* tvm, Value rA, Value rB, ido_uint32 dstR){
    ObjString* b = AS_STRING(rB);
    ObjString* a = AS_STRING(rA);

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(tvm, chars, length);
    tvm->registers[dstR] = OBJ_VAL(result);
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

    #define GET_REGISTER_VALUE(target, source)\
    do {\
        switch (GET_TYPE(source)) {\
            case VAL_INUMBER:\
                target = GET_CONSTANT(AS_INUMBER(source));\
                break;\
            case VAL_NIL:\
                target = NIL_VAL();\
                break;\
            case VAL_DNUMBER:\
            case VAL_BOOL:\
                target = source;\
                break;\
            default: target = NIL_VAL();\
        }\
    } while (false)
    
    #define BINARY_OP(vType, op) \
        do { \
            ido_uint32 rD = DEC_REGISTER_DEST(i);\
            Value rA;\
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);\
            Value rB;\
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);\
            if(!IS_NUMBER(rA) || !IS_NUMBER(rB)){\
                runtimeErr(tvm, "matherr: operands must be numbers");\
                return INTERPRET_RUNTIME_ERROR;\
            }\
            tvm->registers[rD] = vType(rA.as.dnumber op rB.as.dnumber);\
            printValue(tvm->registers[rD]);\
            \
            freeR(tvm, DEC_REGISTER_RA(i));\
            freeR(tvm, DEC_REGISTER_RB(i));\
            \
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
        case OP_ADD:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);
            
            if(IS_STRING(rA) && IS_STRING(rB)){
                concatenate(tvm, rA, rB, rD);
            } else if(IS_NUMBER(rA) && IS_NUMBER(rB)){
                tvm->registers[rD] = DNUMBER_VAL(rA.as.dnumber + rB.as.dnumber);
            } else {
                runtimeErr(tvm, "error: operands must be either numbers or strings");
                return INTERPRET_RUNTIME_ERROR;
            }
            
            freeR(tvm, DEC_REGISTER_RA(i));
            freeR(tvm, DEC_REGISTER_RB(i));

            printValue(tvm->registers[rD]);
            ibreak;
        }
        case OP_SUB:{BINARY_OP(DNUMBER_VAL, -); ibreak;}
        case OP_MUL:{BINARY_OP(DNUMBER_VAL, *); ibreak;}
        case OP_DIV:{BINARY_OP(DNUMBER_VAL, /); ibreak;}
        case OP_NEG:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            Value v = GET_CONSTANT(AS_INUMBER(tvm->registers[r]));
            tvm->program->constants.values[AS_INUMBER(tvm->registers[r])] = DNUMBER_VAL(-v.as.dnumber);
            ibreak;
        }
        case OP_TRUE:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            tvm->registers[r] = BOOL_VAL(true);
            ibreak;
        }
        case OP_FALSE:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            tvm->registers[r] = BOOL_VAL(false);
            ibreak;
        }
        case OP_NIL:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            tvm->registers[r] = NIL_VAL();
            ibreak;
        }
        case OP_NOT:{
            ido_uint32 r = DEC_REGISTER_DEST(i);
            Value v = BOOL_VAL(isFalsey(tvm->registers[r]));
            tvm->registers[r] = v;
            printValue(tvm->registers[r]);
            ibreak;
        }
        case OP_GREATER:{BINARY_OP(BOOL_VAL, >); ibreak;}
        case OP_LESS:{BINARY_OP(BOOL_VAL, <); ibreak;}

        case OP_GREATER_EQUAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);

            tvm->registers[rD] = BOOL_VAL(valuesGreaterEqual(rA, rB));

            freeR(tvm, DEC_REGISTER_RA(i));
            freeR(tvm, DEC_REGISTER_RB(i));
            ibreak;
        }
        case OP_LESS_EQUAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);

            tvm->registers[rD] = BOOL_VAL(valuesLessEqual(rA, rB));
            freeR(tvm, DEC_REGISTER_RA(i));
            freeR(tvm, DEC_REGISTER_RB(i));
            ibreak;
        }
        case OP_EQUAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);

            tvm->registers[rD] = BOOL_VAL(valuesEqual(rA, rB));
            freeR(tvm, DEC_REGISTER_RA(i));
            freeR(tvm, DEC_REGISTER_RB(i));
            ibreak;
        }
        case OP_BANG_EQUAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);

            tvm->registers[rD] = BOOL_VAL(valuesNotEqual(rA, rB));
            freeR(tvm, DEC_REGISTER_RA(i));
            freeR(tvm, DEC_REGISTER_RB(i));
            ibreak;
        }
        case OP_RETURN:{
            printf("Last RD result: ");
            printValue(tvm->registers[getLastAllocatedRegister(tvm)]);
            printf("Registers after return: %i\n", tvm->free_register_count);
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
    #undef GET_CONSTANT
    #undef BINARY_OP
    #undef GET_REGISTER_VALUE
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
