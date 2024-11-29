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
    tvm->pc = 0;
}

void freeVM(TVM* tvm){

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
            Value v = GET_CONSTANT(DEC_CONSTANT(i));
            printValue(v);
            printf("\n");
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
