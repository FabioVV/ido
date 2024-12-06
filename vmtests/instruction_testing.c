#include <stdio.h>
#include <string.h> // for memset
#include <stdlib.h> // for UINT_MAX
#include <stdint.h> // for uint8_t

// Bunch of magic numbers i know
// Will fix it later


// #define ENC_CONSTANT(constantIndex) (OP_CONSTANT << 26) | (constantIndex & 0x1FFFFFF)
// #define DEC_CONSTANT(i)             (i & 0x1FFFFFF)

// uint32_t e = ENC_CONSTANT(11231);
// uint32_t d = DEC_CONSTANT(e);
// printf("%lu aaa", d);

#define IS32INT ((UINT_MAX >> 30) >= 3)

#if IS32INT
    typedef unsigned int ido_uint32;
#else
    typedef unsigned long ido_uint32;
#endif

#define NUM_REGS 8 // Only 8 registers, its for a simple test

/*
    With heavy inspirations from Lua, IDO instructions are also 32 bits in size
*/
typedef ido_uint32 Instruction;

typedef enum {
    OP_LOAD,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_RETURN,
    OP_ILLEGAL,
    OP_HALT
} opcode;

ido_uint32 registers[NUM_REGS];

int main(){
    memset(registers, 0, sizeof(registers));

    // Very simple macros to encode and decode stuff

    #define ENCODE_ILOAD(reg, val) \
        ((OP_LOAD << 26) | ((reg & 0xFF) << 18) | (val))

    #define GET_OPCODE(inst) \
        ((inst >> 26) & 0x3F)

    #define GET_OPR_1(inst) \
        ((inst >> 18) & 0xFF)

    // I named ..._LARGE_CONSTANT, because if you are making a real VM you would ideally want to store stuff
    // like numbers and strings in an constant pool. You would then use the index of the obj in the constant pool
    // to encode in the instruction, but here iam simply passing the full number directly to the bytecode instruction
    // you could do that in a real VM, but problems would arise as soon as the number becomes too big to fit in the instruction
    #define GET_OPR_LARGE_CONSTANT(inst) \
        (inst & 0x3FFFF)


    #define ENCODE_ADD(dstr, ra, rb) \
        ((OP_ADD << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))
    
    #define ENCODE_SUB(dstr, ra, rb) \
        ((OP_SUB << 26) | ((dstr & 0xFF) << 18) | ((ra & 0xFF) << 10) | (rb))

    #define GET_REG_DSTR(inst) \
        ((inst >> 18) & 0xFF)

    #define GET_REG_A(inst) \
        ((inst >> 10) & 0xFF)

    #define GET_REG_B(inst) \
        ((inst & 0x1FF))



    Instruction f1 = ENCODE_ILOAD(2, 25);
    Instruction f2 = ENCODE_ILOAD(3, 25);
    Instruction f3 = ENCODE_ADD(1, 2, 3);

    Instruction f4 = ENCODE_ILOAD(2, 3);
    Instruction f5 = ENCODE_ILOAD(3, 5);
    Instruction f6 = ENCODE_SUB(1, 2, 3);

    Instruction instructions[] = {f1, f2, f3, f4, f5, f6};

    printf("Bytecode:\n");
    for (int i = 0; i < 6; i++) {
        Instruction ins = instructions[i];
        printf("Instruction %d: 0x%08X\n", i, ins);
    }

    for(int i = 0; i < 6; i++){
        Instruction inst = instructions[i];
        opcode op = GET_OPCODE(inst);

        switch (op)
        {
        case OP_LOAD:{
            uint8_t registr = GET_OPR_1(inst);
            ido_uint32 val = GET_OPR_LARGE_CONSTANT(inst);
            registers[registr] = val;

            printf("%i: LOAD %lu R%i\n", i, val, registr);

            break;
        }
        case OP_ADD:{

            uint8_t dstr = GET_REG_DSTR(inst);
            uint8_t ra = GET_REG_A(inst);
            uint8_t rb = GET_REG_B(inst);
            ido_uint32 result = registers[ra] + registers[rb];

            registers[dstr] = result;

            printf("%i: ADD R%i R%i R%i -> (R1 = %i)\n", i, dstr, ra, rb, result);

            break;
        }
        case OP_SUB:{

            uint8_t dstr = GET_REG_DSTR(inst);
            uint8_t ra = GET_REG_A(inst);
            uint8_t rb = GET_REG_B(inst);
            ido_uint32 result = registers[ra] - registers[rb];

            registers[dstr] = result;

            printf("%i: SUB R%i R%i R%i -> (R1 = %i)\n", i, dstr, ra, rb, result);

            break;
        }
        case OP_HALT:
            return 0;
        default:
            printf("UNKNOW OPERATION");
            break;
        }

    }
    
    #undef IS32INT
    #undef NUM_REGS
    #undef ENCODE_ILOAD
    #undef GET_OPCODE
    #undef GET_REG_DSTR
    #undef ENCODE_ADD
    #undef GET_REG_A
    #undef GET_REG_B

    return 0;
}