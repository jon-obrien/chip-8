#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rom.h"
#include "cpu.h"
#include "display.h"
#include "keypad.h"

static void usage(const char *program) {
    fprintf(stderr, "usage: %s <rom.ch8> [--flags cosmac|modern]\n", program);
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 4) {
        usage(argv[0]);
        return 1;
    }

    const char *variant = "cosmac";
    if (argc == 4) {
        if (strcmp(argv[2], "--flags") != 0) {
            usage(argv[0]);
            return 1;
        }
        variant = argv[3];
    }

    if (strcmp(variant, "cosmac") != 0 && strcmp(variant, "modern") != 0) {
        fprintf(stderr, "unknown flags: %s\n", variant);
        return 1;
    }

    Rom rom;
    if (!rom_load(argv[1], &rom)) {
        return 1;
    }

    Cpu cpu;
    cpu_init(&cpu);
    if (strcmp(variant, "modern") == 0) {
        flags_modern(&cpu.flags);
    } else {
        flags_cosmac(&cpu.flags);
    }

    if (!cpu_load_rom(&cpu, rom.data, rom.size)) {
        rom_free(&rom);
        return 1;
    }

    Display *display;
    if (!display_init(&display)) {
        rom_free(&rom);
        return 1;
    }

    srand((unsigned)time(NULL));
    bool running = true;
    while (running) {
        if (display_poll_events(&cpu)) {
            running = false;
        }

        for (int i = 0; i < CHIP8_INSTRUCTIONS_PER_CYCLE; i++) {
            cpu_step(&cpu);
        }

        cpu_tick_timers(&cpu);
        display_update_sound(display, &cpu);
        keypad_end_frame(&cpu);
        display_draw(display, &cpu);
        display_wait_frame();
    }

    display_destroy(display);
    rom_free(&rom);
    return 0;
}
