#ifndef HARNESS_H
#define HARNESS_H

#include <stddef.h>
#include <stdint.h>
#include "cpu.h"

typedef void (*TestFn)(void);

typedef struct {
    const char *name;
    TestFn fn;
} TestCase;

typedef struct {
    const char *area;
    const TestCase *tests;
} TestSuite;

extern int test_failures;

void test_fail(const char *file, int line, const char *expr);
void test_fail_eq(const char *file, int line, const char *actual_expr, long long actual,
                  const char *expected_expr, long long expected);
void test_check_screen(const char *file, int line, const Cpu *cpu,
                       const char *const *rows, size_t count);

#define CHECK(cond) do { \
    if (!(cond)) { \
        test_fail(__FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_EQ(actual, expected) do { \
    long long harness_actual = (long long)(actual); \
    long long harness_expected = (long long)(expected); \
    if (harness_actual != harness_expected) { \
        test_fail_eq(__FILE__, __LINE__, #actual, harness_actual, #expected, harness_expected); \
    } \
} while (0)

#define CHECK_SCREEN(cpu, rows) \
    test_check_screen(__FILE__, __LINE__, (cpu), (rows), sizeof (rows) / sizeof (rows)[0])

void load_program(Cpu *cpu, const uint8_t *rom, size_t len);
void run_steps(Cpu *cpu, int count);
void run_opcode(Cpu *cpu, uint16_t opcode);

#endif
