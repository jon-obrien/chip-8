#include <string.h>

#include "harness.h"

static void sprite_at_i(Cpu *cpu, const uint8_t *rows, size_t count) {
    cpu->I = 0x300;
    memcpy(&cpu->memory[0x300], rows, count);
}

static void test_00e0_clears_screen(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.display[5][5] = 1;
    cpu.display[63][31] = 1;
    run_opcode(&cpu, 0x00E0);
    const char *rows[] = {""};
    CHECK_SCREEN(&cpu, rows);
}

static void test_dxyn_draws_sprite(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    run_opcode(&cpu, 0xD011);
    const char *rows[] = {"#"};
    CHECK_SCREEN(&cpu, rows);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_dxyn_font_zero_is_drawn(void) {
    Cpu cpu;
    cpu_init(&cpu);
    run_opcode(&cpu, 0xF029);
    run_opcode(&cpu, 0xD015);
    const char *rows[] = {
        "####",
        "#..#",
        "#..#",
        "#..#",
        "####",
    };
    CHECK_SCREEN(&cpu, rows);
}

static void test_dxyn_draw_twice_erases_and_sets_vf(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    run_opcode(&cpu, 0xD011);
    run_opcode(&cpu, 0xD011);
    const char *rows[] = {""};
    CHECK_SCREEN(&cpu, rows);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_dxyn_vf_clears_on_draw_without_collision(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    run_opcode(&cpu, 0xD011);
    run_opcode(&cpu, 0xD011);
    CHECK_EQ(cpu.V[0xF], 1);

    cpu.V[0] = 10;
    run_opcode(&cpu, 0xD011);
    CHECK_EQ(cpu.V[0xF], 0);
    const char *rows[] = {"..........#"};
    CHECK_SCREEN(&cpu, rows);
}

static void test_dxyn_partial_overlap_xors_and_sets_vf(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.memory[0x300] = 0xF0;
    cpu.memory[0x301] = 0xFF;

    cpu.I = 0x300;
    run_opcode(&cpu, 0xD011);

    cpu.I = 0x301;
    cpu.V[0] = 2;
    run_opcode(&cpu, 0xD011);

    CHECK_EQ(cpu.V[0xF], 1);
    const char *rows[] = {"##..######"};
    CHECK_SCREEN(&cpu, rows);
}

static void test_dxyn_blank_row_keeps_pixels(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    run_opcode(&cpu, 0xD011);

    cpu.memory[0x300] = 0x00;
    cpu.pc = CHIP8_ROM_START;
    run_opcode(&cpu, 0xD011);
    const char *rows[] = {"#"};
    CHECK_SCREEN(&cpu, rows);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_dxyn_clips_at_right_edge(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0xFF};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    cpu.V[0] = 60;
    cpu.V[1] = 0;
    run_opcode(&cpu, 0xD011);
    CHECK_EQ(cpu.display[60][0], 1);
    CHECK_EQ(cpu.display[63][0], 1);
    CHECK_EQ(cpu.display[0][0], 0);
}

static void test_dxyn_clips_at_bottom_edge(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    cpu.V[0] = 0;
    cpu.V[1] = 30;
    run_opcode(&cpu, 0xD015);
    CHECK_EQ(cpu.display[0][30], 1);
    CHECK_EQ(cpu.display[7][31], 1);
    CHECK_EQ(cpu.display[0][0], 0);
}

static void test_dxyn_wraps_start_x(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    cpu.V[0] = 70;
    cpu.V[1] = 0;
    run_opcode(&cpu, 0xD011);
    CHECK_EQ(cpu.display[6][0], 1);
}

static void test_dxyn_wraps_start_y(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    cpu.V[0] = 0;
    cpu.V[1] = 33;
    run_opcode(&cpu, 0xD011);
    CHECK_EQ(cpu.display[0][1], 1);
}

static void test_dxyn_x_register_f_is_read_as_coordinate(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    cpu.V[0xF] = 10;
    cpu.V[1] = 2;
    run_opcode(&cpu, 0xDF11);
    CHECK_EQ(cpu.display[10][2], 1);
    CHECK_EQ(cpu.display[0][2], 0);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_dxyn_edge_zero_rows_draws_nothing(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0xFF};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    run_opcode(&cpu, 0xD010);
    const char *rows[] = {""};
    CHECK_SCREEN(&cpu, rows);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_dxyn_does_not_change_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    const uint8_t sprite[] = {0x80};
    sprite_at_i(&cpu, sprite, sizeof sprite);
    run_opcode(&cpu, 0xD011);
    CHECK_EQ(cpu.memory[0x300], 0x80);
}

static void test_dxyn_rejects_rows_past_end_of_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.I = 0xFFF;
    cpu.memory[0xFFF] = 0xFF;
    run_opcode(&cpu, 0xD012);
    CHECK_EQ(cpu.display[0][0], 1);
    CHECK_EQ(cpu.display[0][1], 0);
}

static const TestCase display_tests[] = {
    {"00E0 clears screen",                         test_00e0_clears_screen},
    {"DXYN draws sprite",                          test_dxyn_draws_sprite},
    {"DXYN draws font zero",                       test_dxyn_font_zero_is_drawn},
    {"DXYN draw twice erases and sets VF",         test_dxyn_draw_twice_erases_and_sets_vf},
    {"DXYN VF clears on draw without collision",   test_dxyn_vf_clears_on_draw_without_collision},
    {"DXYN partial overlap XORs and sets VF",      test_dxyn_partial_overlap_xors_and_sets_vf},
    {"DXYN blank row keeps pixels",                test_dxyn_blank_row_keeps_pixels},
    {"DXYN clips at right edge",                   test_dxyn_clips_at_right_edge},
    {"DXYN clips at bottom edge",                  test_dxyn_clips_at_bottom_edge},
    {"DXYN wraps start X",                         test_dxyn_wraps_start_x},
    {"DXYN wraps start Y",                         test_dxyn_wraps_start_y},
    {"DXYN reads X register F as coordinate",      test_dxyn_x_register_f_is_read_as_coordinate},
    {"DXYN edge zero rows draws nothing",          test_dxyn_edge_zero_rows_draws_nothing},
    {"DXYN does not change memory",                test_dxyn_does_not_change_memory},
    {"DXYN rejects rows past end of memory",       test_dxyn_rejects_rows_past_end_of_memory},
    {NULL, NULL},
};

const TestSuite display_suite = {"display", display_tests};
