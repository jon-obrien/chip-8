#include <stdio.h>

#include "rom.h"
#include "cpu.h"

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

    for (int i = 0; i < 500; i++) {
        cpu_step(&cpu);
    }

    for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < CHIP8_DISPLAY_WIDTH; x++) {
            putchar(cpu.display[x][y] ? '#' : '.');
        }
        putchar('\n');
    }

    rom_free(&rom);

    return 0;
}
