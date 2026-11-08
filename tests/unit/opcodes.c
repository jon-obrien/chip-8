#include "harness.h"
#include "keypad.h"

static void test_1nnn_jumps(void) {
    Cpu cpu;
    cpu_init(&cpu);
    run_opcode(&cpu, 0x1300);
    CHECK_EQ(cpu.pc, 0x300);
}

static void test_2nnn_and_00ee_call_and_return(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.memory[0x200] = 0x23;
    cpu.memory[0x201] = 0x00;
    cpu.memory[0x300] = 0x00;
    cpu.memory[0x301] = 0xEE;

    run_steps(&cpu, 1);
    CHECK_EQ(cpu.pc, 0x300);
    CHECK_EQ(cpu.stack_ptr, 1);

    run_steps(&cpu, 1);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
    CHECK_EQ(cpu.stack_ptr, 0);
}

static void test_nested_calls_return_in_order(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.memory[0x200] = 0x23;
    cpu.memory[0x201] = 0x00;
    cpu.memory[0x300] = 0x24;
    cpu.memory[0x301] = 0x00;
    cpu.memory[0x302] = 0x00;
    cpu.memory[0x303] = 0xEE;
    cpu.memory[0x400] = 0x00;
    cpu.memory[0x401] = 0xEE;

    run_steps(&cpu, 2);
    CHECK_EQ(cpu.pc, 0x400);
    CHECK_EQ(cpu.stack_ptr, 2);

    run_steps(&cpu, 1);
    CHECK_EQ(cpu.pc, 0x302);
    CHECK_EQ(cpu.stack_ptr, 1);

    run_steps(&cpu, 1);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
    CHECK_EQ(cpu.stack_ptr, 0);
}

static void test_3xnn_skips_when_equal(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0x42;
    run_opcode(&cpu, 0x3042);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 4);
}

static void test_3xnn_does_not_skip_when_different(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0x42;
    run_opcode(&cpu, 0x3043);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
}

static void test_4xnn_skips_when_not_equal(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0x42;
    run_opcode(&cpu, 0x4041);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 4);
}

static void test_4xnn_does_not_skip_when_equal(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0x42;
    run_opcode(&cpu, 0x4042);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
}

static void test_5xy0_skips_when_registers_equal(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[1] = 7;
    cpu.V[2] = 7;
    run_opcode(&cpu, 0x5120);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 4);
}

static void test_5xy0_does_not_skip_when_different(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[1] = 7;
    cpu.V[2] = 8;
    run_opcode(&cpu, 0x5120);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
}

static void test_6xnn_sets_register(void) {
    Cpu cpu;
    cpu_init(&cpu);
    run_opcode(&cpu, 0x6A42);
    CHECK_EQ(cpu.V[0xA], 0x42);
}

static void test_7xnn_wraps_without_touching_vf(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0xFF;
    run_opcode(&cpu, 0x7002);
    CHECK_EQ(cpu.V[0], 0x01);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy0_copies_register(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 1;
    cpu.V[1] = 9;
    run_opcode(&cpu, 0x8010);
    CHECK_EQ(cpu.V[0], 9);
}

static void test_8xy1_or_resets_vf_chip8(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 0xF0;
    cpu.V[1] = 0x0F;
    cpu.V[0xF] = 1;
    run_opcode(&cpu, 0x8011);
    CHECK_EQ(cpu.V[0], 0xFF);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy1_or_keeps_vf_schip(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    cpu.V[0] = 0xF0;
    cpu.V[1] = 0x0F;
    cpu.V[0xF] = 1;
    run_opcode(&cpu, 0x8011);
    CHECK_EQ(cpu.V[0], 0xFF);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy2_and_resets_vf_chip8(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 0xFC;
    cpu.V[1] = 0x3F;
    cpu.V[0xF] = 1;
    run_opcode(&cpu, 0x8012);
    CHECK_EQ(cpu.V[0], 0x3C);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy3_xor_resets_vf_chip8(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 0xFF;
    cpu.V[1] = 0x0F;
    cpu.V[0xF] = 1;
    run_opcode(&cpu, 0x8013);
    CHECK_EQ(cpu.V[0], 0xF0);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy4_sets_carry(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0xF0;
    cpu.V[1] = 0x20;
    run_opcode(&cpu, 0x8014);
    CHECK_EQ(cpu.V[0], 0x10);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy4_no_carry_when_sum_fits(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 1;
    cpu.V[1] = 2;
    run_opcode(&cpu, 0x8014);
    CHECK_EQ(cpu.V[0], 3);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy4_edge_255_plus_zero_has_no_carry(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0xFF;
    cpu.V[1] = 0;
    run_opcode(&cpu, 0x8014);
    CHECK_EQ(cpu.V[0], 0xFF);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy4_edge_255_plus_one_has_carry(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0xFF;
    cpu.V[1] = 1;
    run_opcode(&cpu, 0x8014);
    CHECK_EQ(cpu.V[0], 0);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy4_carry_wins_when_x_is_f(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0xF] = 0xF0;
    cpu.V[1] = 0x20;
    run_opcode(&cpu, 0x8F14);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy5_sets_no_borrow(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 5;
    cpu.V[1] = 3;
    run_opcode(&cpu, 0x8015);
    CHECK_EQ(cpu.V[0], 2);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy5_borrow_when_y_is_larger(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 3;
    cpu.V[1] = 5;
    run_opcode(&cpu, 0x8015);
    CHECK_EQ(cpu.V[0], 0xFE);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy5_edge_equal_values_have_no_borrow(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 5;
    cpu.V[1] = 5;
    run_opcode(&cpu, 0x8015);
    CHECK_EQ(cpu.V[0], 0);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy6_shift_chip8_reads_vy(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[1] = 6;
    cpu.V[2] = 3;
    run_opcode(&cpu, 0x8126);
    CHECK_EQ(cpu.V[1], 1);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy6_shift_schip_reads_vx(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    cpu.V[1] = 6;
    cpu.V[2] = 3;
    run_opcode(&cpu, 0x8126);
    CHECK_EQ(cpu.V[1], 3);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy6_edge_shifted_bit_wins_when_x_is_f(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    cpu.V[0xF] = 2;
    run_opcode(&cpu, 0x8F06);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xye_shift_left_schip(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    cpu.V[1] = 0x81;
    run_opcode(&cpu, 0x810E);
    CHECK_EQ(cpu.V[1], 0x02);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy7_subtract_reversed(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 3;
    cpu.V[1] = 5;
    run_opcode(&cpu, 0x8017);
    CHECK_EQ(cpu.V[0], 2);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_8xy7_borrow_when_x_is_larger(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 5;
    cpu.V[1] = 3;
    run_opcode(&cpu, 0x8017);
    CHECK_EQ(cpu.V[0], 0xFE);
    CHECK_EQ(cpu.V[0xF], 0);
}

static void test_8xy7_edge_equal_values_have_no_borrow(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 5;
    cpu.V[1] = 5;
    run_opcode(&cpu, 0x8017);
    CHECK_EQ(cpu.V[0], 0);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_9xy0_skips_when_registers_differ(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[1] = 7;
    cpu.V[2] = 8;
    run_opcode(&cpu, 0x9120);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 4);
}

static void test_9xy0_does_not_skip_when_equal(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[1] = 7;
    cpu.V[2] = 7;
    run_opcode(&cpu, 0x9120);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
}

static void test_annn_sets_index(void) {
    Cpu cpu;
    cpu_init(&cpu);
    run_opcode(&cpu, 0xA123);
    CHECK_EQ(cpu.I, 0x123);
}

static void test_bnnn_jump_uses_v0(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 0x10;
    cpu.V[2] = 0x20;
    run_opcode(&cpu, 0xB234);
    CHECK_EQ(cpu.pc, 0x244);
}

static void test_bnnn_jump_uses_vx(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    cpu.V[0] = 0x10;
    cpu.V[2] = 0x20;
    run_opcode(&cpu, 0xB234);
    CHECK_EQ(cpu.pc, 0x254);
}

static void test_bnnn_rejects_fetch_past_end_of_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    cpu.V[0xF] = 0x10;
    run_opcode(&cpu, 0xBFFF);
    CHECK_EQ(cpu.pc, 0x100F);

    run_steps(&cpu, 1);
    CHECK_EQ(cpu.pc, 0x100F);
}

static void test_cxnn_mask_zero_gives_zero(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0xAA;
    run_opcode(&cpu, 0xC000);
    CHECK_EQ(cpu.V[0], 0);
}

static void test_cxnn_respects_mask(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0;
    run_opcode(&cpu, 0xC00F);
    CHECK((cpu.V[0] & 0xF0) == 0);
}

static void test_fx07_reads_delay_timer(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.delay_tmr = 5;
    run_opcode(&cpu, 0xF207);
    CHECK_EQ(cpu.V[2], 5);
}

static void test_fx15_sets_delay_timer(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 7;
    run_opcode(&cpu, 0xF015);
    CHECK_EQ(cpu.delay_tmr, 7);
}

static void test_fx18_sets_sound_timer(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 9;
    run_opcode(&cpu, 0xF018);
    CHECK_EQ(cpu.sound_tmr, 9);
}

static void test_fx1e_adds_to_index(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.I = 0x200;
    cpu.V[0] = 0x10;
    run_opcode(&cpu, 0xF01E);
    CHECK_EQ(cpu.I, 0x210);
}

static void test_fx1e_sets_vf_on_overflow_when_flag_on(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.flags.index_overflow_sets_vf = true;
    cpu.I = 0xFFF;
    cpu.V[0] = 2;
    run_opcode(&cpu, 0xF01E);
    CHECK_EQ(cpu.I, 0x1001);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_fx1e_leaves_vf_when_flag_off(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.I = 0xFFF;
    cpu.V[0] = 2;
    cpu.V[0xF] = 0x55;
    run_opcode(&cpu, 0xF01E);
    CHECK_EQ(cpu.V[0xF], 0x55);
}

static void test_fx1e_edge_x_is_f_reads_before_write(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.flags.index_overflow_sets_vf = true;
    cpu.I = 0xFF0;
    cpu.V[0xF] = 0x20;
    run_opcode(&cpu, 0xFF1E);
    CHECK_EQ(cpu.I, 0x1010);
    CHECK_EQ(cpu.V[0xF], 1);
}

static void test_fx29_points_at_font(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0xA;
    run_opcode(&cpu, 0xF029);
    CHECK_EQ(cpu.I, CHIP8_FONT_START + 0xA * 5);
}

static void test_fx29_edge_uses_low_nibble_only(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0x1F;
    run_opcode(&cpu, 0xF029);
    CHECK_EQ(cpu.I, CHIP8_FONT_START + 0xF * 5);
}

static void test_fx33_stores_bcd(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 254;
    cpu.I = 0x300;
    run_opcode(&cpu, 0xF033);
    CHECK_EQ(cpu.memory[0x300], 2);
    CHECK_EQ(cpu.memory[0x301], 5);
    CHECK_EQ(cpu.memory[0x302], 4);
}

static void test_fx33_edge_stores_zero_and_255(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0;
    cpu.I = 0x300;
    run_opcode(&cpu, 0xF033);
    CHECK_EQ(cpu.memory[0x300], 0);
    CHECK_EQ(cpu.memory[0x301], 0);
    CHECK_EQ(cpu.memory[0x302], 0);

    cpu_init(&cpu);
    cpu.V[0] = 255;
    cpu.I = 0x300;
    run_opcode(&cpu, 0xF033);
    CHECK_EQ(cpu.memory[0x300], 2);
    CHECK_EQ(cpu.memory[0x301], 5);
    CHECK_EQ(cpu.memory[0x302], 5);
}

static void test_fx33_edge_stores_at_last_three_bytes(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 123;
    cpu.I = 0xFFD;
    run_opcode(&cpu, 0xF033);
    CHECK_EQ(cpu.memory[0xFFD], 1);
    CHECK_EQ(cpu.memory[0xFFE], 2);
    CHECK_EQ(cpu.memory[0xFFF], 3);
}

static void test_fx33_rejects_past_end_of_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 123;
    cpu.memory[0xFFE] = 0xAA;
    cpu.memory[0xFFF] = 0xAA;
    cpu.I = 0xFFE;
    run_opcode(&cpu, 0xF033);
    CHECK_EQ(cpu.memory[0xFFE], 0xAA);
    CHECK_EQ(cpu.memory[0xFFF], 0xAA);

    cpu.I = 0xFFF;
    cpu.pc = CHIP8_ROM_START;
    run_opcode(&cpu, 0xF033);
    CHECK_EQ(cpu.memory[0xFFF], 0xAA);
}

static void test_fx55_stores_registers_and_advances_i_chip8(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 1;
    cpu.V[1] = 2;
    cpu.V[2] = 3;
    cpu.I = 0x300;
    run_opcode(&cpu, 0xF255);
    CHECK_EQ(cpu.memory[0x300], 1);
    CHECK_EQ(cpu.memory[0x301], 2);
    CHECK_EQ(cpu.memory[0x302], 3);
    CHECK_EQ(cpu.I, 0x303);
}

static void test_fx55_keeps_i_schip(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_SCHIP_MODERN);
    cpu.V[0] = 1;
    cpu.V[1] = 2;
    cpu.V[2] = 3;
    cpu.I = 0x300;
    run_opcode(&cpu, 0xF255);
    CHECK_EQ(cpu.memory[0x302], 3);
    CHECK_EQ(cpu.I, 0x300);
}

static void test_fx55_edge_stores_at_last_three_bytes(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 1;
    cpu.V[1] = 2;
    cpu.V[2] = 3;
    cpu.I = 0xFFD;
    run_opcode(&cpu, 0xF255);
    CHECK_EQ(cpu.memory[0xFFD], 1);
    CHECK_EQ(cpu.memory[0xFFE], 2);
    CHECK_EQ(cpu.memory[0xFFF], 3);
}

static void test_fx55_rejects_past_end_of_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 1;
    cpu.V[1] = 2;
    cpu.V[2] = 3;
    cpu.memory[0xFFE] = 0xAA;
    cpu.memory[0xFFF] = 0xAA;
    cpu.I = 0xFFE;
    run_opcode(&cpu, 0xF255);
    CHECK_EQ(cpu.memory[0xFFE], 0xAA);
    CHECK_EQ(cpu.memory[0xFFF], 0xAA);
    CHECK_EQ(cpu.I, 0xFFE);
}

static void test_fx55_rejects_after_index_overflow(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.flags.index_overflow_sets_vf = true;
    cpu.I = 0xFFF;
    cpu.V[0] = 2;
    run_opcode(&cpu, 0xF01E);
    CHECK_EQ(cpu.I, 0x1001);

    run_opcode(&cpu, 0xF055);
    CHECK_EQ(cpu.I, 0x1001);
}

static void test_fx65_loads_registers_and_advances_i_chip8(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.memory[0x300] = 7;
    cpu.memory[0x301] = 8;
    cpu.memory[0x302] = 9;
    cpu.I = 0x300;
    run_opcode(&cpu, 0xF265);
    CHECK_EQ(cpu.V[0], 7);
    CHECK_EQ(cpu.V[1], 8);
    CHECK_EQ(cpu.V[2], 9);
    CHECK_EQ(cpu.I, 0x303);
}

static void test_fx65_edge_loads_at_last_three_bytes(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.memory[0xFFD] = 4;
    cpu.memory[0xFFE] = 5;
    cpu.memory[0xFFF] = 6;
    cpu.I = 0xFFD;
    run_opcode(&cpu, 0xF265);
    CHECK_EQ(cpu.V[0], 4);
    CHECK_EQ(cpu.V[1], 5);
    CHECK_EQ(cpu.V[2], 6);
}

static void test_fx65_rejects_past_end_of_memory(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, MODEL_CHIP8);
    cpu.V[0] = 0xAA;
    cpu.I = 0xFFE;
    run_opcode(&cpu, 0xF265);
    CHECK_EQ(cpu.V[0], 0xAA);
    CHECK_EQ(cpu.I, 0xFFE);
}

static void test_ex9e_skips_when_key_pressed(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 5;
    keypad_set(&cpu, 5, true);
    run_opcode(&cpu, 0xE09E);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 4);
}

static void test_ex9e_does_not_skip_for_other_key(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 4;
    keypad_set(&cpu, 5, true);
    run_opcode(&cpu, 0xE09E);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
}

static void test_exa1_skips_when_key_not_pressed(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 5;
    run_opcode(&cpu, 0xE0A1);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 4);
}

static void test_exa1_does_not_skip_when_key_pressed(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 5;
    keypad_set(&cpu, 5, true);
    run_opcode(&cpu, 0xE0A1);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
}

static void test_fx0a_waits_for_key(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[0] = 0x33;
    run_opcode(&cpu, 0xF00A);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START);
    CHECK_EQ(cpu.V[0], 0x33);
}

static void test_fx0a_stores_pressed_key(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 5, true);
    run_opcode(&cpu, 0xF00A);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
    CHECK_EQ(cpu.V[0], 5);
}

static void test_00ee_underflow_advances_pc(void) {
    Cpu cpu;
    cpu_init(&cpu);
    run_opcode(&cpu, 0x00EE);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
    CHECK_EQ(cpu.stack_ptr, 0);
}

static void test_2nnn_overflow_advances_pc(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.stack_ptr = CHIP8_STACK_SIZE;
    run_opcode(&cpu, 0x2300);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 2);
    CHECK_EQ(cpu.stack_ptr, CHIP8_STACK_SIZE);
}

static void test_unknown_opcodes_are_skipped(void) {
    Cpu cpu;
    cpu_init(&cpu);
    cpu.V[1] = 4;
    run_opcode(&cpu, 0xFFFF);
    run_opcode(&cpu, 0x8128);
    run_opcode(&cpu, 0xE000);
    run_opcode(&cpu, 0xF000);
    CHECK_EQ(cpu.pc, CHIP8_ROM_START + 8);
    CHECK_EQ(cpu.V[1], 4);
}

static const TestCase opcode_tests[] = {
    {"1NNN jumps",                                       test_1nnn_jumps},
    {"2NNN and 00EE call and return",                    test_2nnn_and_00ee_call_and_return},
    {"nested calls return in order",                     test_nested_calls_return_in_order},
    {"3XNN skips when equal",                            test_3xnn_skips_when_equal},
    {"3XNN does not skip when different",                test_3xnn_does_not_skip_when_different},
    {"4XNN skips when not equal",                        test_4xnn_skips_when_not_equal},
    {"4XNN does not skip when equal",                    test_4xnn_does_not_skip_when_equal},
    {"5XY0 skips when registers equal",                  test_5xy0_skips_when_registers_equal},
    {"5XY0 does not skip when different",                test_5xy0_does_not_skip_when_different},
    {"6XNN sets register",                               test_6xnn_sets_register},
    {"7XNN wraps without touching VF",                   test_7xnn_wraps_without_touching_vf},
    {"8XY0 copies register",                             test_8xy0_copies_register},
    {"8XY1 OR resets VF (chip8)",                       test_8xy1_or_resets_vf_chip8},
    {"8XY1 OR keeps VF (schip)",                        test_8xy1_or_keeps_vf_schip},
    {"8XY2 AND resets VF (chip8)",                      test_8xy2_and_resets_vf_chip8},
    {"8XY3 XOR resets VF (chip8)",                      test_8xy3_xor_resets_vf_chip8},
    {"8XY4 sets carry",                                  test_8xy4_sets_carry},
    {"8XY4 no carry when sum fits",                      test_8xy4_no_carry_when_sum_fits},
    {"8XY4 edge 255 plus 0 has no carry",                test_8xy4_edge_255_plus_zero_has_no_carry},
    {"8XY4 edge 255 plus 1 has carry",                   test_8xy4_edge_255_plus_one_has_carry},
    {"8XY4 carry wins when X is F",                      test_8xy4_carry_wins_when_x_is_f},
    {"8XY5 sets no-borrow",                              test_8xy5_sets_no_borrow},
    {"8XY5 borrow when Y is larger",                     test_8xy5_borrow_when_y_is_larger},
    {"8XY5 edge equal values have no borrow",            test_8xy5_edge_equal_values_have_no_borrow},
    {"8XY6 shift reads VY (chip8)",                     test_8xy6_shift_chip8_reads_vy},
    {"8XY6 shift reads VX (schip)",                     test_8xy6_shift_schip_reads_vx},
    {"8XY6 edge shifted bit wins when X is F",           test_8xy6_edge_shifted_bit_wins_when_x_is_f},
    {"8XYE shift left (schip)",                         test_8xye_shift_left_schip},
    {"8XY7 subtract reversed",                           test_8xy7_subtract_reversed},
    {"8XY7 borrow when X is larger",                     test_8xy7_borrow_when_x_is_larger},
    {"8XY7 edge equal values have no borrow",            test_8xy7_edge_equal_values_have_no_borrow},
    {"9XY0 skips when registers differ",                 test_9xy0_skips_when_registers_differ},
    {"9XY0 does not skip when equal",                    test_9xy0_does_not_skip_when_equal},
    {"ANNN sets index",                                  test_annn_sets_index},
    {"BNNN jumps using V0 (chip8)",                     test_bnnn_jump_uses_v0},
    {"BNNN jumps using VX (schip)",                     test_bnnn_jump_uses_vx},
    {"BNNN rejects fetch past end of memory",            test_bnnn_rejects_fetch_past_end_of_memory},
    {"CXNN mask 0 gives 0",                              test_cxnn_mask_zero_gives_zero},
    {"CXNN respects mask",                               test_cxnn_respects_mask},
    {"FX07 reads delay timer",                           test_fx07_reads_delay_timer},
    {"FX15 sets delay timer",                            test_fx15_sets_delay_timer},
    {"FX18 sets sound timer",                            test_fx18_sets_sound_timer},
    {"FX1E adds to index",                               test_fx1e_adds_to_index},
    {"FX1E sets VF on overflow when flag on",            test_fx1e_sets_vf_on_overflow_when_flag_on},
    {"FX1E leaves VF when flag off",                     test_fx1e_leaves_vf_when_flag_off},
    {"FX1E edge X is F reads before write",              test_fx1e_edge_x_is_f_reads_before_write},
    {"FX29 points at font",                              test_fx29_points_at_font},
    {"FX29 edge uses low nibble only",                   test_fx29_edge_uses_low_nibble_only},
    {"FX33 stores BCD",                                  test_fx33_stores_bcd},
    {"FX33 edge stores zero and 255",                    test_fx33_edge_stores_zero_and_255},
    {"FX33 edge stores at last three bytes",             test_fx33_edge_stores_at_last_three_bytes},
    {"FX33 rejects past end of memory",                  test_fx33_rejects_past_end_of_memory},
    {"FX55 stores and advances I (chip8)",              test_fx55_stores_registers_and_advances_i_chip8},
    {"FX55 keeps I (schip)",                            test_fx55_keeps_i_schip},
    {"FX55 edge stores at last three bytes",             test_fx55_edge_stores_at_last_three_bytes},
    {"FX55 rejects past end of memory",                  test_fx55_rejects_past_end_of_memory},
    {"FX55 rejects after index overflow",                test_fx55_rejects_after_index_overflow},
    {"FX65 loads and advances I (chip8)",               test_fx65_loads_registers_and_advances_i_chip8},
    {"FX65 edge loads at last three bytes",              test_fx65_edge_loads_at_last_three_bytes},
    {"FX65 rejects past end of memory",                  test_fx65_rejects_past_end_of_memory},
    {"EX9E skips when key pressed",                      test_ex9e_skips_when_key_pressed},
    {"EX9E does not skip for other key",                 test_ex9e_does_not_skip_for_other_key},
    {"EXA1 skips when key not pressed",                  test_exa1_skips_when_key_not_pressed},
    {"EXA1 does not skip when key pressed",              test_exa1_does_not_skip_when_key_pressed},
    {"FX0A waits for key",                               test_fx0a_waits_for_key},
    {"FX0A stores pressed key",                          test_fx0a_stores_pressed_key},
    {"00EE underflow advances pc",                       test_00ee_underflow_advances_pc},
    {"2NNN overflow advances pc",                        test_2nnn_overflow_advances_pc},
    {"unknown opcodes are skipped",                      test_unknown_opcodes_are_skipped},
    {NULL, NULL},
};

const TestSuite opcode_suite = {"opcodes", opcode_tests};
