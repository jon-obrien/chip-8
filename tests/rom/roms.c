#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "keypad.h"
#include "rom.h"

#define ROM_FRAMES 600
#define QUIRKS_MENU_ADDR 0x1FF
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

/* menu_choice is written to QUIRKS_MENU_ADDR when non-zero, to skip a ROM's menu. */
static bool start_rom(Cpu *cpu, const char *file, Chip8Model model, uint8_t menu_choice) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", CHIP8_ROM_DIR, file);

    Rom rom;
    if (!rom_load(path, &rom)) {
        return false;
    }

    cpu_init(cpu);
    cpu_set_model(cpu, model);
    bool loaded = cpu_load_rom(cpu, rom.data, rom.size);
    rom_free(&rom);
    if (!loaded) {
        return false;
    }

    if (menu_choice != 0) {
        cpu->memory[QUIRKS_MENU_ADDR] = menu_choice;
    }

    return true;
}

static void run_frames(Cpu *cpu, int frames) {
    for (int f = 0; f < frames; f++) {
        for (int i = 0; i < CHIP8_INSTRUCTIONS_PER_CYCLE; i++) {
            cpu_step(cpu);
        }
        cpu_tick_timers(cpu);
        keypad_end_frame(cpu);
    }
}

static bool run_rom(Cpu *cpu, const char *file, Chip8Model model, uint8_t menu_choice) {
    if (!start_rom(cpu, file, model, menu_choice)) {
        return false;
    }

    run_frames(cpu, ROM_FRAMES);
    return true;
}

static void test_chip8_logo_splash(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "1-chip8-logo.ch8", MODEL_CHIP8, 0));
    CHECK_SCREEN_FILE(&cpu, "1-chip8-logo.txt");
}

static void test_ibm_logo_shows_ibm(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "2-ibm-logo.ch8", MODEL_CHIP8, 0));
    CHECK_SCREEN_FILE(&cpu, "2-ibm-logo.txt");
}

static void test_corax_all_checks_chip8(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "3-corax+.ch8", MODEL_CHIP8, 0));
    CHECK_SCREEN_FILE(&cpu, "3-corax+.txt");
}

static void test_corax_all_checks_schip(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "3-corax+.ch8", MODEL_SCHIP_MODERN, 0));
    CHECK_SCREEN_FILE(&cpu, "3-corax+.txt");
}

static void test_flags_all_checks_chip8(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "4-flags.ch8", MODEL_CHIP8, 0));
    CHECK_SCREEN_FILE(&cpu, "4-flags.txt");
}

static void test_flags_all_checks_schip(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "4-flags.ch8", MODEL_SCHIP_MODERN, 0));
    CHECK_SCREEN_FILE(&cpu, "4-flags.txt");
}

static void test_quirks_shows_chip8_results(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "5-quirks.ch8", MODEL_CHIP8, 1));
    CHECK_SCREEN_FILE(&cpu, "5-quirks-chip8.txt");
}

static void test_quirks_shows_schip_modern_results(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "5-quirks.ch8", MODEL_SCHIP_MODERN, 2));
    CHECK_SCREEN_FILE(&cpu, "5-quirks-schip-modern.txt");
}

static void test_quirks_shows_schip_legacy_results(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "5-quirks.ch8", MODEL_SCHIP_LEGACY, 4));
    CHECK_SCREEN_FILE(&cpu, "5-quirks-schip-legacy.txt");
}

static void test_quirks_shows_xochip_results(void) {
    Cpu cpu;
    CHECK(run_rom(&cpu, "5-quirks.ch8", MODEL_XO_CHIP, 3));
    CHECK_SCREEN_FILE(&cpu, "5-quirks-xochip.txt");
}

/* The menu cursor blinks every 10 frames, so this frame count is one where it is visible. */
static void test_keypad_shows_menu(void) {
    Cpu cpu;
    CHECK(start_rom(&cpu, "6-keypad.ch8", MODEL_CHIP8, 0));
    run_frames(&cpu, 610);
    CHECK_SCREEN_FILE(&cpu, "6-keypad-menu.txt");
}

static void test_keypad_down_highlights_pressed_keys(void) {
    Cpu cpu;
    CHECK(start_rom(&cpu, "6-keypad.ch8", MODEL_CHIP8, 1));
    keypad_set(&cpu, 0x1, true);
    keypad_set(&cpu, 0x6, true);
    run_frames(&cpu, ROM_FRAMES);
    CHECK_SCREEN_FILE(&cpu, "6-keypad-down.txt");
}

static void test_keypad_up_unhighlights_pressed_keys(void) {
    Cpu cpu;
    CHECK(start_rom(&cpu, "6-keypad.ch8", MODEL_CHIP8, 2));
    keypad_set(&cpu, 0x1, true);
    keypad_set(&cpu, 0x6, true);
    run_frames(&cpu, ROM_FRAMES);
    CHECK_SCREEN_FILE(&cpu, "6-keypad-up.txt");
}

static void test_keypad_getkey_shows_all_good(void) {
    Cpu cpu;
    CHECK(start_rom(&cpu, "6-keypad.ch8", MODEL_CHIP8, 3));
    run_frames(&cpu, 60);
    keypad_set(&cpu, 0x5, true);
    run_frames(&cpu, 10);
    keypad_set(&cpu, 0x5, false);
    run_frames(&cpu, 60);
    CHECK_SCREEN_FILE(&cpu, "6-keypad-getkey.txt");
}

static const TestCase rom_tests[] = {
    {"1-chip8-logo shows splash",      test_chip8_logo_splash},
    {"2-ibm-logo shows IBM logo",      test_ibm_logo_shows_ibm},
    {"3-corax+ all checks on chip8",   test_corax_all_checks_chip8},
    {"3-corax+ all checks on schip",   test_corax_all_checks_schip},
    {"4-flags all checks on chip8",    test_flags_all_checks_chip8},
    {"4-flags all checks on schip",    test_flags_all_checks_schip},
    {"5-quirks shows chip8 results",   test_quirks_shows_chip8_results},
    {"5-quirks shows schip modern results", test_quirks_shows_schip_modern_results},
    {"5-quirks shows schip legacy results", test_quirks_shows_schip_legacy_results},
    {"5-quirks shows xochip results",       test_quirks_shows_xochip_results},
    {"6-keypad shows menu",                 test_keypad_shows_menu},
    {"6-keypad down highlights pressed keys", test_keypad_down_highlights_pressed_keys},
    {"6-keypad up unhighlights pressed keys", test_keypad_up_unhighlights_pressed_keys},
    {"6-keypad getkey shows all good",      test_keypad_getkey_shows_all_good},
    {NULL, NULL},
};

const TestSuite rom_suite = {"rom", rom_tests};
