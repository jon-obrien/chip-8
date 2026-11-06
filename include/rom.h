#ifndef ROM_H
#define ROM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu.h"

#define ROM_MAX_SIZE (CHIP8_RAM_SIZE - CHIP8_ROM_START)

typedef struct {
    uint8_t *data;
    size_t size;
} Rom;

bool rom_load(const char *path, Rom *rom);
void rom_free(Rom *rom);

#endif
