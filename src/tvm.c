// The tania VM
#include "tvm.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"


TVM* initVM(){
    TVM* tvm = ALLOCATESTRUCT(TVM);
    if(tvm == NULL){
        fprintf(stderr, "error allocating tvm: not enough memory");
        exit(1);
    }
    for (int i = 0; i < REGISTERS_NUM; i++) {
        tvm->registers[i] = NIL_VAL();
        tvm->allocatedRegisters[i] = false;  // All registers are initially free (may change)
    }

    tvm->last_allocated_register = INVALID_REGISTER; // Initialize with an invalid register
    tvm->free_register_count = REGISTERS_NUM;
    tvm->pc = 0;
    tvm->objects = NULL;
    initTable(&tvm->strings);
    initTable(&tvm->globals);

    return tvm;
}

void freeVM(TVM* tvm){
    freeTable(&tvm->strings);
    freeTable(&tvm->globals);
    FREE(TVM, tvm);
}

static void runtimeErr(TVM* tvm, const char* format, ...){

    size_t inst = tvm->pc - tvm->program->code - 1;
    int line = tvm->program->lines[inst];
    fprintf(stderr, "[line %d] in script \n", line);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);


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


static InterpretResult runVM(TVM* tvm){
    #define ibreak break

    #define GET_CONSTANT(index) (tvm->program->constants.values[index])
    #define READ_STRING(value) AS_STRING(value)
    
    #define GET_REGISTER_VALUE(target, source)\
        do {\
            switch (GET_TYPE(source)) {\
                case VAL_INUMBER:\
                    target = GET_CONSTANT(AS_INUMBER(source));\
                    break;\
                case VAL_NIL:\
                    target = NIL_VAL();\
                    break;\
                case VAL_OBJ:\
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
                runtimeErr(tvm, "   operands must be numbers");\
                return INTERPRET_RUNTIME_ERROR;\
            }\
            tvm->registers[rD] = vType(rA.as.dnumber op rB.as.dnumber);\
        } while(false)\

    for(;;){
        register Instruction i = NEXT_INSTRUCTION(tvm);
        Opcode OP = GET_OPCODE(i);
        switch (OP)
        {
        case OP_CONSTANT:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            ido_uint32 constantIndex = DEC_CONSTANT_INDEX(i);
            tvm->registers[rD] = INUMBER_VAL(constantIndex);
            ibreak;
        }
        case OP_PRINT:{
            ido_uint32 rIndex = DEC_GET_GLOBAL_CINDEX(i);
            Value v;
            GET_REGISTER_VALUE(v, tvm->registers[rIndex]);
            printValue(v);
            printf("\n");
            ibreak;
        }
        case OP_DEFINE_GLOBAL:{
            ido_uint32 rReadFrom = DEC_REGISTER_DEST(i);
            ido_uint32 constantIndex = DEC_CONSTANT_INDEX(i);

            ObjString* name = READ_STRING(GET_CONSTANT(constantIndex));
            Value v;
            GET_REGISTER_VALUE(v, tvm->registers[rReadFrom]);
            tableSet(&tvm->globals, name, v);

            ibreak;
        }
        case OP_GET_GLOBAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            ido_uint32 constantIndex = DEC_CONSTANT_INDEX(i);
            ObjString* name = READ_STRING(GET_CONSTANT(constantIndex));
            Value v;
            if(!tableGet(&tvm->globals, name, &v)){
                runtimeErr(tvm, "   undefined var '%s'", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            tvm->registers[rD] = v;
            ibreak;
        }  
        case OP_SET_GLOBAL:{
            ido_uint32 rReadFrom = DEC_REGISTER_DEST(i);
            ido_uint32 constantIndex = DEC_CONSTANT_INDEX(i);

            ObjString* name = READ_STRING(GET_CONSTANT(constantIndex));
            Value v;
            GET_REGISTER_VALUE(v, tvm->registers[rReadFrom]);

            if(tableSet(&tvm->globals, name, v)){
                tableDelete(&tvm->globals, name);
                runtimeErr(tvm, "   undefined var '%s'", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            ibreak;
        } 
        case OP_GET_LOCAL :{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            ido_uint32 rIndex = DEC_CONSTANT_INDEX(i);
            tvm->registers[rD] = tvm->registers[rIndex];           
            ibreak;
        }
        case OP_SET_LOCAL:{
            ido_uint32 rReadFrom = DEC_REGISTER_DEST(i);
            ido_uint32 rIndex = DEC_CONSTANT_INDEX(i); // Need better handling aswell, maybe a dec_register_index

            tvm->registers[rIndex] = tvm->registers[rReadFrom];
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
                runtimeErr(tvm, "   operands must be either numbers or strings");
                return INTERPRET_RUNTIME_ERROR;
            }

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

            ibreak;
        }
        case OP_LESS_EQUAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);

            tvm->registers[rD] = BOOL_VAL(valuesLessEqual(rA, rB));

            ibreak;
        }
        case OP_EQUAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);

            tvm->registers[rD] = BOOL_VAL(valuesEqual(rA, rB));

            ibreak;
        }
        case OP_BANG_EQUAL:{
            ido_uint32 rD = DEC_REGISTER_DEST(i);
            
            Value rA;
            GET_REGISTER_VALUE(rA, tvm->registers[DEC_REGISTER_RA(i)]);
            Value rB;
            GET_REGISTER_VALUE(rB, tvm->registers[DEC_REGISTER_RB(i)]);

            tvm->registers[rD] = BOOL_VAL(valuesNotEqual(rA, rB));

            ibreak;
        }
        case OP_JUMP_IF_FALSE:{
            ido_uint32 offset = GET_JUMP_OFFSET(i);
            ido_uint32 readConditionFrom = DEC_REGISTER_DEST(i);

            if(isFalsey(tvm->registers[readConditionFrom])) tvm->pc += offset;
            ibreak;
        }
        case OP_JUMP:{
            ido_uint32 offset = GET_JUMP_OFFSET(i);
            tvm->pc += offset;
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
            runtimeErr(tvm, "   something has gone wrong. unrecognized code of operation");
            return INTERPRET_RUNTIME_ERROR;
        }

    }

    #undef ibreak
    #undef GET_CONSTANT
    #undef READ_STRING
    #undef BINARY_OP
    #undef GET_REGISTER_VALUE
}

InterpretResult interpret(TVM* tvm, Scanner* sc, Parser* p){
    Program program;
    initProgram(&program);

    if(!compile(&program, sc, p, tvm)){
        freeProgram(&program);
        return INTERPRET_COMPILE_ERROR;
    }

    tvm->program = &program;
    tvm->pc = tvm->program->code;

    InterpretResult resultVM = runVM(tvm);

    freeProgram(&program);
    return resultVM;
}
