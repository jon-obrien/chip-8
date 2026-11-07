#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opcodes.h"
#include "cpu.h"

typedef void (*OpFn)(Cpu *cpu, uint16_t opcode);

static void op_unknown(Cpu *cpu, uint16_t opcode) {
    (void)cpu;
    fprintf(stderr, "unknown opcode %04X\n", opcode);
}

static void op_0xxx(Cpu *cpu, uint16_t opcode) {
    switch (opcode) {
        case 0x00E0:
            memset(cpu->display, 0, sizeof cpu->display);
            break;
        case 0x00EE: {
            uint16_t value;
            if (!cpu_stack_pop(cpu, &value)) {
                fprintf(stderr, "stack underflow at pc %04X\n", cpu->pc);
                break;
            }

            cpu->pc = value;
            break;
        }
        default:
            op_unknown(cpu, opcode);
            break;
    }
}

static void op_1nnn(Cpu *cpu, uint16_t opcode) {
    cpu->pc = opcode & 0x0FFF;
}

static void op_2nnn(Cpu *cpu, uint16_t opcode) {
    if (!(cpu_stack_push(cpu, cpu->pc))) {
        fprintf(stderr, "stack underflow at pc %04X\n", cpu->pc);
        return;
    }

    cpu->pc = opcode & 0x0FFF;
}

static void op_3xnn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;

    if (cpu->V[x] == (uint8_t)(opcode & 0x00FF)) {
        cpu->pc += 2;
    }
}

static void op_4xnn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;

    if (cpu->V[x] != (uint8_t)(opcode & 0x00FF)) {
        cpu->pc += 2;
    }
}

static void op_5xy0(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;

    if (cpu->V[x] == cpu->V[y]) {
        cpu->pc += 2;
    }
}

static void op_6xnn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;
    cpu->V[x] = (uint8_t)(opcode & 0x00FF);
}

static void op_7xnn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;
    cpu->V[x] = (uint8_t)(cpu->V[x] + (opcode & 0x00FF));
}

static void op_8xyn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t n = opcode & 0xF;

    switch (n) {
        case 0x0:
            cpu->V[x] = cpu->V[y];
            break;
        case 0x1:
            cpu->V[x] |= cpu->V[y];
            break;
        case 0x2:
            cpu->V[x] &= cpu->V[y];
            break;
        case 0x3:
            cpu->V[x] ^= cpu->V[y];
            break;
        case 0x4:{
            uint16_t sum = (cpu->V[x] + cpu->V[y]);

            cpu->V[x] = (uint8_t)sum;
            if (sum > 255){
                cpu->V[CHIP8_CARRY_FLAG_INDEX] = 1;
            } else {
                cpu->V[CHIP8_CARRY_FLAG_INDEX] = 0;
            }

            break;
        }
        case 0x5: {
            uint8_t no_borrow = 0;
            if (cpu->V[x] >= cpu->V[y]){
                no_borrow = 1;
            }

            cpu->V[x] -= (cpu->V[y]);
            cpu->V[CHIP8_CARRY_FLAG_INDEX] = no_borrow;

            break;
        }
        case 0x6: {
            uint8_t value = cpu->V[x];

            if (cpu->shift_uses_vy) {
                value = cpu->V[y];
            }

            uint8_t shifted_out = value & 0x1;

            cpu->V[x] = (uint8_t)(value >> 1);
            cpu->V[CHIP8_CARRY_FLAG_INDEX] = shifted_out;

            break;
        }
        case 0x7: {
            uint8_t no_borrow = 0;
            if (cpu->V[y] >= cpu->V[x]){
                no_borrow = 1;
            }

            cpu->V[x] = (uint8_t)(cpu->V[y] - cpu->V[x]);
            cpu->V[CHIP8_CARRY_FLAG_INDEX] = no_borrow;

            break;
        }
        case 0xE: {
            uint8_t value = cpu->V[x];

            if (cpu->shift_uses_vy) {
                value = cpu->V[y];
            }

            uint8_t shifted_out = (value >> 7) & 0x1;

            cpu->V[x] = (uint8_t)(value << 1);
            cpu->V[CHIP8_CARRY_FLAG_INDEX] = shifted_out;

            break;
        }
        default:
            op_unknown(cpu, opcode);
            break;
    }
}

static void op_9xy0(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;

    if (cpu->V[x] != cpu->V[y]) {
        cpu->pc += 2;
    }
}

static void op_annn(Cpu *cpu, uint16_t opcode) {
    cpu->I = opcode & 0x0FFF;
}

static void op_cxnn(Cpu *cpu, uint16_t opcode) {
    uint8_t x = (opcode >> 8) & 0xF;

    cpu->V[x] = (uint8_t)(rand() & (opcode & 0x00FF));
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
    [0x2] = op_2nnn,
    [0x3] = op_3xnn,
    [0x4] = op_4xnn,
    [0x5] = op_5xy0,
    [0x6] = op_6xnn,
    [0x7] = op_7xnn,
    [0x8] = op_8xyn,
    [0x9] = op_9xy0,
    [0xA] = op_annn,
    op_unknown,
    [0xC] = op_cxnn,
    [0xD] = op_dxyn,
    op_unknown,
    op_unknown,
};

void opcode_execute(Cpu *cpu, uint16_t opcode) {
    uint8_t first = (opcode >> 12) & 0xF;
    op_table[first](cpu, opcode);
}
