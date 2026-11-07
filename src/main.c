#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rom.h"
#include "cpu.h"
#include "display.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.ch8>\n", argv[0]);
        return 1;
    }

    Rom rom;
    if (!rom_load(argv[1], &rom)) {
        return 1;
    }

    Cpu cpu;
    cpu_init(&cpu);
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
        display_draw(display, &cpu);
        display_wait_frame();
    }

    display_destroy(display);
    rom_free(&rom);
    return 0;
}
