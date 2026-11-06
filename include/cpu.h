#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CHIP8_RAM_SIZE 4096
#define CHIP8_CPU_SPEED 700
#define CHIP8_FRAMES_PER_SECOND 60
#define CHIP8_INSTRUCTIONS_PER_CYCLE (CHIP8_CPU_SPEED / CHIP8_FRAMES_PER_SECOND)

#define CHIP8_FONT_START 0x050
#define CHIP8_ROM_START 0x200
#define CHIP8_CARRY_FLAG_INDEX 0xF

#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32

typedef struct {
    uint8_t memory[CHIP8_RAM_SIZE];
    uint16_t pc;
    uint8_t display[CHIP8_DISPLAY_WIDTH][CHIP8_DISPLAY_HEIGHT];
    uint16_t I;
    uint16_t stack[16];
    uint8_t stack_ptr;
    uint8_t delay_tmr;
    uint8_t sound_tmr;
    uint8_t V[16];
} Cpu;

void cpu_init(Cpu *cpu);
bool cpu_load_rom(Cpu *cpu, const uint8_t *data, size_t len);
void cpu_step(Cpu *cpu);

#endif
