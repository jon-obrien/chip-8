#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "rom.h"

static bool has_rom_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    return dot != NULL && strcmp(dot, ".ch8") == 0;
}

bool rom_load(const char *path, Rom *rom) {
    rom->data = NULL;
    rom->size = 0;

    if (!has_rom_extension(path)) {
        fprintf(stderr, "%s: expected a .ch8 file\n", path);
        return false;
    }

    FILE *rom_file = fopen(path, "rb");
    if (!rom_file) { perror(path); return false; }

    struct stat st;
    if (fstat(fileno(rom_file), &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "%s is not a regular file\n", path);
        fclose(rom_file);
        return false;
    }

    if (st.st_size <= 0 || st.st_size > ROM_MAX_SIZE) {
           fprintf(stderr, "%s: invalid ROM size (%lld bytes)\n", path, (long long)st.st_size);
           fclose(rom_file);
           return false;
       }

    size_t size = (size_t)st.st_size;
    uint8_t *data = malloc(size);
    if (!data) {
        fprintf(stderr, "out of memory loading %s\n", path);
        fclose(rom_file);
        return false;
    }

    size_t bytes_read = fread(data, 1, size, rom_file);
    fclose(rom_file);

    if (bytes_read != size) {
        fprintf(stderr, "%s: could not read full file\n", path);
        free(data);
        return false;
    }

    rom->data = data;
    rom->size = size;

    return true;
}

void rom_free(Rom *rom) {
    free(rom->data);

    rom->data = NULL;
    rom->size = 0;
}
