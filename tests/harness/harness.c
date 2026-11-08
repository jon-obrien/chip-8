#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"

int test_failures = 0;

void test_fail(const char *file, int line, const char *expr) {
    printf("    %s:%d: CHECK(%s)\n", file, line, expr);
    test_failures++;
}

void test_fail_eq(const char *file, int line, const char *actual_expr, long long actual,
                  const char *expected_expr, long long expected) {
    printf("    %s:%d: %s != %s: got %lld (0x%llX), expected %lld (0x%llX)\n",
           file, line, actual_expr, expected_expr,
           actual, (unsigned long long)actual,
           expected, (unsigned long long)expected);
    test_failures++;
}

static bool expected_pixel(const char *const *rows, size_t count, int x, int y) {
    if ((size_t)y >= count || rows[y] == NULL) {
        return false;
    }

    size_t len = strlen(rows[y]);
    return (size_t)x < len && rows[y][x] == '#';
}

static void print_screen(const char *label, const Cpu *cpu, const char *const *rows,
                         size_t count, int last_y, bool use_actual) {
    printf("      %s\n", label);
    for (int y = 0; y <= last_y; y++) {
        printf("      |");
        for (int x = 0; x < CHIP8_DISPLAY_WIDTH; x++) {
            bool on = use_actual ? cpu->display[x][y] != 0 : expected_pixel(rows, count, x, y);
            putchar(on ? '#' : '.');
        }
        printf("|\n");
    }
}

void test_check_screen(const char *file, int line, const Cpu *cpu,
                       const char *const *rows, size_t count) {
    int last_y = -1;
    bool mismatch = false;

    for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < CHIP8_DISPLAY_WIDTH; x++) {
            bool want = expected_pixel(rows, count, x, y);
            bool have = cpu->display[x][y] != 0;
            if (want || have) {
                last_y = y;
            }
            if (want != have) {
                mismatch = true;
            }
        }
    }

    if (!mismatch) {
        return;
    }

    printf("    %s:%d: screen does not match\n", file, line);
    print_screen("expected:", cpu, rows, count, last_y, false);
    print_screen("actual:", cpu, rows, count, last_y, true);
    test_failures++;
}

void load_program(Cpu *cpu, const uint8_t *rom, size_t len) {
    cpu_init(cpu);
    cpu_load_rom(cpu, rom, len);
}

void run_steps(Cpu *cpu, int count) {
    for (int i = 0; i < count; i++) {
        cpu_step(cpu);
    }
}

void run_opcode(Cpu *cpu, uint16_t opcode) {
    cpu->memory[cpu->pc] = (uint8_t)(opcode >> 8);
    cpu->memory[cpu->pc + 1] = (uint8_t)(opcode & 0xFF);
    cpu_step(cpu);
}
