#include <string.h>
#include <stdio.h>

#include "cpu.h"
#include "opcodes.h"

static const uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void cpu_init(Cpu *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memcpy(&cpu->memory[CHIP8_FONT_START], font, sizeof font);
    cpu->pc = CHIP8_ROM_START;
    cpu->shift_uses_vy = true;
    cpu->loadstore_uses_i = true;
    cpu->jump_uses_vx = false;
    cpu->index_overflow_sets_vf = false;
}

bool cpu_load_rom(Cpu *cpu, const uint8_t *data, size_t len) {
    if (len > CHIP8_RAM_SIZE - CHIP8_ROM_START) {
        fprintf(stderr, "rom too large\n");
        return false;
    }

    memcpy(&cpu->memory[CHIP8_ROM_START], data, len);

    return true;
}

void cpu_step(Cpu *cpu) {
    uint16_t opcode = (uint16_t)((cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1]);
    cpu->pc += 2;
    opcode_execute(cpu, opcode);
}

void cpu_tick_timers(Cpu *cpu) {
    if (cpu->delay_tmr > 0) {
        cpu->delay_tmr--;
    }
    if (cpu->sound_tmr > 0) {
        cpu->sound_tmr--;
    }
}

bool cpu_stack_push(Cpu *cpu, uint16_t value) {
    if (cpu->stack_ptr >= CHIP8_STACK_SIZE) {
        return false;
    }

    cpu->stack[cpu->stack_ptr] = value;
    cpu->stack_ptr += 1;

    return true;
}

bool cpu_stack_pop(Cpu *cpu, uint16_t *value) {
    if (cpu->stack_ptr == 0) {
        return false;
    }

    cpu->stack_ptr -= 1;
    *value = cpu->stack[cpu->stack_ptr];

    return true;
}
