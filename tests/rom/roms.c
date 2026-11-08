#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "keypad.h"
#include "rom.h"

#define ROM_FRAMES 600
#define SCREEN_LINE_MAX (CHIP8_DISPLAY_WIDTH + 2)

typedef struct {
    char lines[CHIP8_DISPLAY_HEIGHT][SCREEN_LINE_MAX];
    const char *rows[CHIP8_DISPLAY_HEIGHT];
    size_t count;
} Screen;

static bool load_screen(const char *name, Screen *screen) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", CHIP8_SCREEN_DIR, name);

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("    could not open %s\n", path);
        return false;
    }

    screen->count = 0;
    while (screen->count < CHIP8_DISPLAY_HEIGHT &&
           fgets(screen->lines[screen->count], SCREEN_LINE_MAX, file) != NULL) {
        char *line = screen->lines[screen->count];
        line[strcspn(line, "\r\n")] = '\0';
        screen->rows[screen->count] = line;
        screen->count++;
    }

    char extra[SCREEN_LINE_MAX];
    bool more = fgets(extra, sizeof extra, file) != NULL;
    fclose(file);

    if (screen->count != CHIP8_DISPLAY_HEIGHT || more) {
        printf("    %s must have exactly %d rows\n", path, CHIP8_DISPLAY_HEIGHT);
        return false;
    }

    return true;
}

static void check_screen_file(const char *file, int line, const Cpu *cpu, const char *name) {
    Screen screen;
    if (!load_screen(name, &screen)) {
        test_fail(file, line, name);
        return;
    }
    test_check_screen(file, line, cpu, screen.rows, screen.count);
}

#define CHECK_SCREEN_FILE(cpu, name) check_screen_file(__FILE__, __LINE__, (cpu), (name))

static bool run_rom(Cpu *cpu, const char *file, void (*preset)(Chip8Flags *)) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", CHIP8_ROM_DIR, file);

    Rom rom;
    if (!rom_load(path, &rom)) {
        return false;
    }

    cpu_init(cpu);
    preset(&cpu->flags);
    bool loaded = cpu_load_rom(cpu, rom.data, rom.size);
    rom_free(&rom);
    if (!loaded) {
        return false;
    }

    for (int f = 0; f < ROM_FRAMES; f++) {
        for (int i = 0; i < CHIP8_INSTRUCTIONS_PER_CYCLE; i++) {
            cpu_step(cpu);
        }
        cpu_tick_timers(cpu);
        keypad_end_frame(cpu);
    }

    return true;
}

static void test_chip8_logo_splash(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "1-chip8-logo.ch8", flags_cosmac));
    CHECK_SCREEN_FILE(&cpu, "1-chip8-logo.txt");
}

static void test_ibm_logo_shows_ibm(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "2-ibm-logo.ch8", flags_cosmac));
    CHECK_SCREEN_FILE(&cpu, "2-ibm-logo.txt");
}

static void test_corax_all_checks_cosmac(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "3-corax+.ch8", flags_cosmac));
    CHECK_SCREEN_FILE(&cpu, "3-corax+.txt");
}

static void test_corax_all_checks_modern(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "3-corax+.ch8", flags_modern));
    CHECK_SCREEN_FILE(&cpu, "3-corax+.txt");
}

static void test_flags_all_checks_cosmac(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "4-flags.ch8", flags_cosmac));
    CHECK_SCREEN_FILE(&cpu, "4-flags.txt");
}

static void test_flags_all_checks_modern(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "4-flags.ch8", flags_modern));
    CHECK_SCREEN_FILE(&cpu, "4-flags.txt");
}

static const TestCase rom_tests[] = {
    {"1-chip8-logo shows splash",     test_chip8_logo_splash},
    {"2-ibm-logo shows IBM logo",     test_ibm_logo_shows_ibm},
    {"3-corax+ all checks on cosmac", test_corax_all_checks_cosmac},
    {"3-corax+ all checks on modern", test_corax_all_checks_modern},
    {"4-flags all checks on cosmac",  test_flags_all_checks_cosmac},
    {"4-flags all checks on modern",  test_flags_all_checks_modern},
    {NULL, NULL},
};

const TestSuite rom_suite = {"rom", rom_tests};
