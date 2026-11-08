#include "harness.h"

static uint8_t oversized_rom[CHIP8_RAM_SIZE];

static void test_init_sets_pc_and_stack(void) {
    Cpu cpu;
    cpu_init(&cpu);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START);
    CHECK_EQ(cpu.stack_ptr, 0);
}

static void test_init_clears_state(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[3] = 9;
    cpu.display[1][1] = 1;
    cpu.I = 0x123;
    cpu.delay_tmr = 4;

    cpu_init(&cpu);
    CHECK_EQ(cpu.V[3], 0);
    CHECK_EQ(cpu.display[1][1], 0);
    CHECK_EQ(cpu.I, 0);
    CHECK_EQ(cpu.delay_tmr, 0);
}

static void test_init_loads_font(void) {
    Cpu cpu;
    cpu_init(&cpu);
    CHECK_EQ(cpu.memory[CHIP8_FONT_START], 0xF0);
    CHECK_EQ(cpu.memory[CHIP8_FONT_START + 79], 0x80);
}

static void test_load_rom_copies_bytes(void) {
    Cpu cpu;
    const uint8_t rom[] = {0x12, 0x34};
    load_program(&cpu, rom, sizeof rom);
    CHECK_EQ(cpu.memory[CHIP8_ROM_START], 0x12);
    CHECK_EQ(cpu.memory[CHIP8_ROM_START + 1], 0x34);
}

static void test_load_rom_accepts_largest_size(void) {
    Cpu cpu;
    cpu_init(&cpu);
    CHECK(cpu_load_rom(&cpu, oversized_rom, CHIP8_RAM_SIZE - CHIP8_ROM_START));
}

static void test_load_rom_rejects_too_large(void) {
    Cpu cpu;
    cpu_init(&cpu);
    CHECK(!cpu_load_rom(&cpu, oversized_rom, CHIP8_RAM_SIZE - CHIP8_ROM_START + 1));
}

static void test_step_runs_last_opcode_in_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.pc = CHIP8_RAM_SIZE - 2;
    cpu.memory[CHIP8_RAM_SIZE - 2] = 0x6A;
    cpu.memory[CHIP8_RAM_SIZE - 1] = 0x01;
    cpu_step(&cpu);
    CHECK_EQ(cpu.V[0xA], 1);
    CHECK_EQ(cpu.pc, CHIP8_RAM_SIZE);
}

static void test_step_rejects_pc_past_end_of_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.pc = CHIP8_RAM_SIZE - 1;
    cpu_step(&cpu);
    CHECK_EQ(cpu.pc, CHIP8_RAM_SIZE - 1);
}

static void test_timers_count_down(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.delay_tmr = 2;
    cpu.sound_tmr = 1;

    cpu_tick_timers(&cpu);
    CHECK_EQ(cpu.delay_tmr, 1);
    CHECK_EQ(cpu.sound_tmr, 0);
}

static void test_timers_stop_at_zero(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.delay_tmr = 1;
    cpu.sound_tmr = 0;

    cpu_tick_timers(&cpu);
    cpu_tick_timers(&cpu);
    CHECK_EQ(cpu.delay_tmr, 0);
    CHECK_EQ(cpu.sound_tmr, 0);
}

static void test_stack_fills_and_empties(void) {
    Cpu cpu;
    cpu_init(&cpu);
    uint16_t value = 0;

    for (int i = 0; i < CHIP8_STACK_SIZE; i++) {
        CHECK(cpu_stack_push(&cpu, (uint16_t)i));
    }
    CHECK(!cpu_stack_push(&cpu, 0xFFFF));

    for (int i = CHIP8_STACK_SIZE - 1; i >= 0; i--) {
        CHECK(cpu_stack_pop(&cpu, &value));
        CHECK_EQ(value, i);
    }
    CHECK(!cpu_stack_pop(&cpu, &value));
}

static void test_model_sets_flags(void) {
    Cpu cpu;

    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    CHECK(cpu.flags.shift_uses_vy);
    CHECK(cpu.flags.loadstore_uses_i);
    CHECK(!cpu.flags.jump_uses_vx);
    CHECK(!cpu.flags.index_overflow_sets_vf);
    CHECK(!cpu.flags.key_beep);
    CHECK(cpu.flags.logic_resets_vf);

    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    CHECK(!cpu.flags.shift_uses_vy);
    CHECK(!cpu.flags.loadstore_uses_i);
    CHECK(cpu.flags.jump_uses_vx);
    CHECK(!cpu.flags.index_overflow_sets_vf);
    CHECK(!cpu.flags.key_beep);
    CHECK(!cpu.flags.logic_resets_vf);

    cpu_set_model(&cpu, MODEL_SCHIP_LEGACY);
    CHECK(!cpu.flags.shift_uses_vy);
    CHECK(!cpu.flags.loadstore_uses_i);
    CHECK(cpu.flags.jump_uses_vx);
    CHECK(!cpu.flags.logic_resets_vf);

    cpu_set_model(&cpu, MODEL_XO_CHIP);
    CHECK(!cpu.flags.shift_uses_vy);
    CHECK(cpu.flags.loadstore_uses_i);
    CHECK(!cpu.flags.jump_uses_vx);
    CHECK(!cpu.flags.logic_resets_vf);
}

static const TestCase cpu_tests[] = {
    {"init sets pc and stack",                 test_init_sets_pc_and_stack},
    {"init clears state",                      test_init_clears_state},
    {"init loads font",                        test_init_loads_font},
    {"load rom copies bytes",                  test_load_rom_copies_bytes},
    {"load rom accepts largest size",          test_load_rom_accepts_largest_size},
    {"load rom rejects too large",             test_load_rom_rejects_too_large},
    {"step runs last opcode in memory",        test_step_runs_last_opcode_in_memory},
    {"step rejects pc past end of memory",     test_step_rejects_pc_past_end_of_memory},
    {"timers count down",                      test_timers_count_down},
    {"timers stop at zero",                    test_timers_stop_at_zero},
    {"stack fills and empties",                test_stack_fills_and_empties},
    {"model sets flags",                       test_model_sets_flags},
    {NULL, NULL},
};

const TestSuite cpu_suite = {"cpu", cpu_tests};
