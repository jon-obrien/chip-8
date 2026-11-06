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

    int steps = 0;
    while (steps < 100) {
        cpu_step(&cpu);
        steps++;
    }

    rom_free(&rom);

    return 0;
}
