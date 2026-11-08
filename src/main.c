#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rom.h"
#include "cpu.h"
#include "display.h"
#include "keypad.h"

static void usage(const char *program) {
    fprintf(stderr, "usage: %s <rom.ch8> [--model chip8|schip|schip-legacy|xochip]\n", program);
}

static bool parse_model(const char *name, Chip8Model *model) {
    if (strcmp(name, "chip8") == 0) {
        *model = MODEL_CHIP8;
    } else if (strcmp(name, "schip") == 0) {
        *model = MODEL_SCHIP_MODERN;
    } else if (strcmp(name, "schip-legacy") == 0) {
        *model = MODEL_SCHIP_LEGACY;
    } else if (strcmp(name, "xochip") == 0) {
        *model = MODEL_XO_CHIP;
    } else {
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 4) {
        usage(argv[0]);
        return 1;
    }

    Chip8Model model = MODEL_CHIP8;
    if (argc == 4) {
        if (strcmp(argv[2], "--model") != 0) {
            usage(argv[0]);
            return 1;
        }
        if (!parse_model(argv[3], &model)) {
            fprintf(stderr, "unknown model: %s\n", argv[3]);
            return 1;
        }
    }

    Rom rom;
    if (!rom_load(argv[1], &rom)) {
        return 1;
    }

    Cpu cpu;
    cpu_init(&cpu);
    cpu_set_model(&cpu, model);

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

        cpu_run_frame(&cpu);
        display_update_sound(display, &cpu);
        cpu_tick_timers(&cpu);
        keypad_end_frame(&cpu);
        display_draw(display, &cpu);
        display_wait_frame();
    }

    display_destroy(display);
    rom_free(&rom);
    return 0;
}
