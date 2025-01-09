#include "butil.h"
#include "instruction.h"

typedef struct{
    char* opname;
} bytecodeInfo;

void printBytecodeSimple(Program* prog){
    ido_uint32* pc = prog->code;

    bytecodeInfo opnames[] = { // expand these later
        // [OP_LOAD]    = {"OP_LOAD"},
        [OP_CONSTANT]    = {"OP_CONSTANT"},
        [OP_GET_FROM_STACK ] = {"OP_GET_FROM_STACK "},
        [OP_SET_FROM_STACK ] = {"OP_SET_FROM_STACK "},
        [OP_PRINT]    = {"OP_PRINT"},
        [OP_DEFINE_GLOBAL]    = {"OP_DEFINE_GLOBAL"},
        [OP_GET_GLOBAL]    = {"OP_GET_GLOBAL"},
        [OP_SET_GLOBAL]    = {"OP_SET_GLOBAL"},
        [OP_GET_LOCAL]    = {"OP_GET_LOCAL"},
        [OP_SET_LOCAL]    = {"OP_SET_LOCAL"},
        [OP_PUSH]    = {"OP_PUSH"},
        [OP_ADD]    = {"OP_ADD"},
        [OP_SUB]    = {"OP_SUB"},
        [OP_MUL]    = {"OP_MUL"},
        [OP_DIV]    = {"OP_DIV"},
        [OP_NEG]    = {"OP_NEG"},
        [OP_TRUE]    = {"OP_TRUE"},
        [OP_FALSE]    = {"OP_FALSE"},
        [OP_NIL]    = {"OP_NIL"},
        [OP_NOT]    = {"OP_NOT"},
        [OP_GREATER]    = {"OP_GREATER"},
        [OP_LESS]    = {"OP_LESS"},
        [OP_EQUAL]    = {"OP_EQUAL"},
        [OP_GREATER_EQUAL]    = {"OP_GREATER_EQUAL"},
        [OP_LESS_EQUAL]    = {"OP_LESS_EQUAL"},
        [OP_BANG_EQUAL]    = {"OP_BANG_EQUAL"},
        [OP_JUMP_IF_FALSE]    = {"OP_JUMP_IF_FALSE"},
        [OP_JUMP]    = {"OP_JUMP"},
        [OP_LOOP]    = {"OP_LOOP"},
        [OP_RETURN]    = {"OP_RETURN"},
        [OP_CALL]    = {"OP_CALL"},
        [OP_HLT]    = {"OP_HLT"},
    };

    Instruction i;
    int index = 0;
    while(index < prog->count && (i = *pc++)){
        Opcode o = GET_OPCODE(i);

        printf("0x%08X ", i);
        printf("%s\n", opnames[o].opname);
        index++;
    }

}
