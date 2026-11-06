#include <stdio.h>

#include "rom.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.ch8>\n", argv[0]);
        return 1;
    }

    Rom rom;
    if (!rom_load(argv[1], &rom)) {
        return 1;
    }

    printf("Loaded %s: %zu bytes\n", argv[1], rom.size);

    size_t to_show = rom.size < 16 ? rom.size : 16;
    for (size_t i = 0; i < to_show; i++) {
        printf("%02X ", rom.data[i]);
    }
    printf("\n");

    rom_free(&rom);

    return 0;
}
