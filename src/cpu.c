#include <string.h>
#include <stdio.h>

#include "cpu.h"

void cpu_init(Cpu *cpu) {
    memset(cpu, 0, sizeof *cpu);
}

bool cpu_load_rom(Cpu *cpu, const uint8_t *data, size_t len) {
    if (len > CHIP8_RAM_SIZE - CHIP8_ROM_START) {
        fprintf(stderr, "rom too large\n");
        return false;
    }

    memcpy(&cpu->memory[CHIP8_ROM_START], data, len);

    return true;
}
