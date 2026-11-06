#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CHIP8_RAM_SIZE 4096
#define CHIP8_ROM_START 0x200
#define CHIP8_FONT_START 0x050

typedef struct {
    uint8_t memory[CHIP8_RAM_SIZE];
    uint16_t pc;
} Cpu;

void cpu_init(Cpu *cpu);
bool cpu_load_rom(Cpu *cpu, const uint8_t *data, size_t len);
void cpu_step(Cpu *cpu);

#endif
