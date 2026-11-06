#ifndef ROM_H
#define ROM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ROM_MAX_SIZE 3584

typedef struct {
    uint8_t *data;
    size_t size;
} Rom;

bool rom_load(const char *path, Rom *rom);
void rom_free(Rom *rom);

#endif
