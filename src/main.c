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

    for (size_t i = 0; i < CHIP8_RAM_SIZE; i++) {
        printf(" %02X", cpu.memory[i]);
    }
    printf("\n");

    rom_free(&rom);

    return 0;
}
