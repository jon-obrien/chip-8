 #include <stdint.h>
#include <stdio.h>
 #include <string.h>

#include "opcodes.h"
#include "cpu.h"

typedef void (*OpFn)(Cpu *cpu, uint16_t opcode);

static void op_unknown(Cpu *cpu, uint16_t opcode) {
    (void)cpu;
    printf("unknown opcode %04X\n", opcode);
}

static void op_0xxx(Cpu *cpu, uint16_t opcode) {
    switch (opcode) {
        case 0x00E0:
            memset(cpu->display, 0, sizeof cpu->display);
            break;
        default:
            op_unknown(cpu, opcode);
            break;
    }
}

static void op_1nnn(Cpu *cpu, uint16_t opcode) {
    cpu->pc = opcode & 0x0FFF;
}

static void op_6xnn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;
    cpu->V[x] = (uint8_t)(opcode & 0x00FF);
}

static void op_7xnn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;
    cpu->V[x] = (uint8_t)(cpu->V[x] + (opcode & 0x00FF));
}

static void op_annn(Cpu *cpu, uint16_t opcode) {
    cpu->I = opcode & 0x0FFF;
}

static void op_dxyn(Cpu *cpu, uint16_t opcode) {
    cpu->V[CHIP8_CARRY_FLAG_INDEX] = 0;

    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t n = opcode & 0xF;

    uint8_t display_x = cpu->V[x] % CHIP8_DISPLAY_WIDTH;
    uint8_t display_y = cpu->V[y] % CHIP8_DISPLAY_HEIGHT;

    for (uint8_t i=0; i < n; i++) {
        uint8_t pixel_y = display_y + i;
        if (pixel_y >= CHIP8_DISPLAY_HEIGHT) {
            break;
        }

        uint8_t sprite_data = cpu->memory[cpu->I + i];

        for (uint8_t j=0; j < 8; j++) {
            uint8_t pixel_x = display_x + j;
            if (pixel_x >= CHIP8_DISPLAY_WIDTH) {
                break;
            }

            uint8_t pixel = (sprite_data >> (7 - j)) & 1;

            if (pixel) {
                if (cpu->display[pixel_x][pixel_y] == 1) {
                    cpu->V[CHIP8_CARRY_FLAG_INDEX] = 1;
                }

                cpu->display[pixel_x][pixel_y] ^= 1;
            }
        }
    }
}

static const OpFn op_table[16] = {
    [0x0] = op_0xxx,
    [0x1] = op_1nnn,
    op_unknown,
    op_unknown,
    op_unknown,
    op_unknown,
    [0x6] = op_6xnn,
    [0x7] = op_7xnn,
    op_unknown,
    op_unknown,
    [0xA] = op_annn,
    op_unknown,
    op_unknown,
    [0xD] = op_dxyn,
    op_unknown,
    op_unknown,
};

void opcode_execute(Cpu *cpu, uint16_t opcode) {
    uint8_t first = (opcode >> 12) & 0xF;
    op_table[first](cpu, opcode);
}
