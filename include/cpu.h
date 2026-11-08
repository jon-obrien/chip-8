#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CHIP8_RAM_SIZE 4096
#define CHIP8_CPU_SPEED 700
#define CHIP8_FRAMES_PER_SECOND 60

#define CHIP8_STACK_SIZE 16

#define CHIP8_FONT_START 0x050
#define CHIP8_ROM_START 0x200
#define CHIP8_CARRY_FLAG_INDEX 0xF

#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32

typedef enum {
    MODEL_CHIP8,
    MODEL_SCHIP_MODERN,
    MODEL_SCHIP_LEGACY,
    MODEL_XO_CHIP,
} Chip8Model;

typedef struct {
    bool shift_uses_vy;
    bool loadstore_uses_i;
    bool jump_uses_vx;
    bool index_overflow_sets_vf;
    bool key_beep;
    bool logic_resets_vf;
} Chip8Flags;

typedef struct {
    bool keys[4][4];
    bool key_state[4][4];
    bool waiting;
    uint8_t wait_key;
} Keypad;

typedef struct {
    uint8_t memory[CHIP8_RAM_SIZE];
    uint16_t pc;
    uint8_t display[CHIP8_DISPLAY_WIDTH][CHIP8_DISPLAY_HEIGHT];
    uint16_t I;
    uint16_t stack[CHIP8_STACK_SIZE];
    uint8_t stack_ptr;
    uint8_t delay_tmr;
    uint8_t sound_tmr;
    uint8_t V[16];

    Keypad keypad;
    unsigned cycle_remainder;
    Chip8Flags flags;
} Cpu;

void cpu_init(Cpu *cpu);
void cpu_run_frame(Cpu *cpu);
bool cpu_load_rom(Cpu *cpu, const uint8_t *data, size_t len);
void cpu_step(Cpu *cpu);
void cpu_tick_timers(Cpu *cpu);

void cpu_set_model(Cpu *cpu, Chip8Model model);

bool cpu_stack_push(Cpu *cpu, uint16_t value);
bool cpu_stack_pop(Cpu *cpu, uint16_t *value);

#endif
