// The tania VM
#include "tvm.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include "value.h"


static void resetStack(TVM* tvm){
    tvm->stackTop = tvm->stack;
    tvm->frameCount = 0;
}


TVM* initVM(){
    TVM* tvm = ALLOCATESTRUCT(TVM);
    if(tvm == NULL){
        fprintf(stderr, "error allocating tvm: not enough memory");
        exit(1);
    }

    resetStack(tvm);
    for (int i = 0; i < REGISTERS_NUM; i++) {
        tvm->registers[i] = NIL_VAL();
        tvm->allocatedRegisters[i] = false;  // All registers are initially free
    }

    tvm->last_allocated_register = INVALID_REGISTER;
    tvm->free_register_count = REGISTERS_NUM;
    // tvm->pc = 0;
    tvm->objects = NULL;
    initTable(&tvm->strings);
    initTable(&tvm->globals);

    return tvm;
}

static void push(TVM* tvm, Value value) {
    *tvm->stackTop = value;
    tvm->stackTop++;
}

static Value pop(TVM* tvm) {
    tvm->stackTop--;
    return *tvm->stackTop;
}

static Value peek(TVM* tvm, int distance) {
  return tvm->stackTop[-1 - distance];
}

void freeVM(TVM* tvm){
    freeTable(&tvm->strings);
    freeTable(&tvm->globals);
    FREE(TVM, tvm);
}

static void runtimeErr(TVM* tvm, const char* format, ...){
    fprintf(stderr, "\n");

    for(int i = tvm->frameCount - 1; i >=0; i--){
        CallFrame* frame = &tvm->frames[i];
        ObjFunction* f = frame->function;
        size_t inst = frame->pc - frame->function->program.code - 1;
        fprintf(stderr, "[line %d] in ", f->program.lines[inst]);
        if(f->name == NULL){
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", f->name->chars);
        }
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    fprintf(stderr, "\n");
    resetStack(tvm);
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

static bool call(TVM* tvm, ObjFunction* f, int agrCount){
    if(agrCount != f->arity){
        runtimeErr(tvm, "   on fn<%s> call expected %d arguments but got %d", f->name->chars, f->arity, agrCount);
        return false;
    }

    if(tvm->frameCount == FRAMES_NUM){
        runtimeErr(tvm, "   stack overflow");
        return false;
    }



    CallFrame* frame = &tvm->frames[tvm->frameCount++];
    frame->function = f;
    frame->pc = f->program.code;
    frame->slots = tvm->stackTop  - 1;

    return true;
}

static bool callValue(TVM* tvm, Value calee, uint8_t argCount){
    if(IS_OBJ(calee)){
        switch (OBJ_TYPE(calee))
        {
        case OBJ_FUNCTION: return  call(tvm, AS_FUNCTION(calee), argCount);
        default: break;
        }
    }   
    runtimeErr(tvm, "   tried calling non-function");
    return false;
}

static InterpretResult runVM(TVM* tvm){
    register CallFrame* frame = &tvm->frames[tvm->frameCount - 1];
    
    #define ibreak break
    #define GET_CONSTANT(index) (frame->function->program.constants.values[index])
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
        register Instruction i = NEXT_INSTRUCTION(frame);
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
            frame->function->program.constants.values[AS_INUMBER(tvm->registers[r])] = DNUMBER_VAL(-v.as.dnumber);
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

            if(isFalsey(tvm->registers[readConditionFrom])) frame->pc += offset;
            ibreak;
        }
        case OP_JUMP:{
            ido_uint32 offset = GET_JUMP_OFFSET(i);
            frame->pc += offset;
            ibreak;
        }
        case OP_LOOP:{
            ido_uint32 offset = GET_JUMP_OFFSET(i);
            frame->pc -= offset;
            ibreak;
        }
        case OP_CALL:{
            uint8_t argCount = DEC_CALL_ARGUMENT_COUNT(i);
            ido_uint32 rFunction = DEC_CALL_FUNCTION(i);

            ObjFunction* f = AS_FUNCTION(tvm->registers[rFunction]);

            // for(uint8_t i = rFunction+1; i <= argCount; i++){
            //     printValue(tvm->registers[i]);
            //     printf("\n");
            //     f->parameters[i].registerIndex = i;
            // }

            if(!callValue(tvm, tvm->registers[rFunction], argCount)){
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &tvm->frames[tvm->frameCount - 1];
            ibreak;
        }
        case OP_RETURN:{
            // pop result
            tvm->frameCount--;
            if(tvm->frameCount == 0){
                return INTERPRET_OK;
            }
            tvm->stackTop = frame->slots;
            //push
            frame = &tvm->frames[tvm->frameCount - 1];
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
    ObjFunction* function = compile(sc, p, tvm);
    if(function == NULL) return INTERPRET_COMPILE_ERROR;

    push(tvm, OBJ_VAL(function));
    call(tvm, function, 0);

    return runVM(tvm);
}
