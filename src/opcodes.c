 #include <stdio.h>

#include "opcodes.h"

typedef void (*OpFn)(Cpu *cpu, uint16_t opcode);

static void op_unknown(Cpu *cpu, uint16_t opcode) {
    (void)cpu;
    printf("unknown opcode %04X\n", opcode);
}

static const OpFn op_table[16] = {
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
};

void opcode_execute(Cpu *cpu, uint16_t opcode) {
    uint8_t first = (opcode >> 12) & 0xF;
    op_table[first](cpu, opcode);
}
