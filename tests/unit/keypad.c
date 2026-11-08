#include "harness.h"
#include "keypad.h"

static void test_tap_within_frame_is_latched(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 5, true);
    keypad_set(&cpu, 5, false);
    CHECK(keypad_is_pressed(&cpu, 5));
}

static void test_end_frame_clears_latch(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 5, true);
    keypad_set(&cpu, 5, false);
    keypad_end_frame(&cpu);
    CHECK(!keypad_is_pressed(&cpu, 5));
}

static void test_held_key_survives_end_frame(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 5, true);
    keypad_end_frame(&cpu);
    CHECK(keypad_is_pressed(&cpu, 5));
}

static void test_release_clears_held_key(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 5, true);
    keypad_end_frame(&cpu);
    keypad_set(&cpu, 5, false);
    CHECK(!keypad_is_pressed(&cpu, 5));
}

static void test_first_pressed_returns_lowest(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 9, true);
    keypad_set(&cpu, 3, true);
    CHECK_EQ(keypad_first_pressed(&cpu), 3);
}

static void test_first_pressed_none_is_negative(void) {
    Cpu cpu;
    cpu_init(&cpu);
    CHECK_EQ(keypad_first_pressed(&cpu), -1);
}

static void test_first_pressed_edge_key_zero(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 0, true);
    CHECK_EQ(keypad_first_pressed(&cpu), 0);
}

static void test_first_pressed_ignores_ended_tap(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 4, true);
    keypad_set(&cpu, 4, false);
    keypad_end_frame(&cpu);
    CHECK_EQ(keypad_first_pressed(&cpu), -1);
}

static void test_set_masks_key_to_low_nibble(void) {
    Cpu cpu;
    cpu_init(&cpu);
    keypad_set(&cpu, 0x15, true);
    CHECK(keypad_is_pressed(&cpu, 5));
}

static const TestCase keypad_tests[] = {
    {"tap within a frame is latched",         test_tap_within_frame_is_latched},
    {"end_frame clears the latch",            test_end_frame_clears_latch},
    {"held key survives end_frame",           test_held_key_survives_end_frame},
    {"release clears a held key",             test_release_clears_held_key},
    {"first_pressed returns lowest key",      test_first_pressed_returns_lowest},
    {"first_pressed is -1 when none pressed", test_first_pressed_none_is_negative},
    {"first_pressed edge key zero",           test_first_pressed_edge_key_zero},
    {"first_pressed ignores an ended tap",    test_first_pressed_ignores_ended_tap},
    {"set masks key to low nibble",           test_set_masks_key_to_low_nibble},
    {NULL, NULL},
};

const TestSuite keypad_suite = {"keypad", keypad_tests};
