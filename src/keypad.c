#include <string.h>

#include "keypad.h"

void keypad_set(Cpu *cpu, uint8_t key, bool pressed) {
    key &= 0xF;
    uint8_t row = key >> 2;
    uint8_t col = key & 0x3;

    cpu->keys[row][col] = pressed;
    if (pressed) {
        cpu->key_state[row][col] = true;
    }
}

bool keypad_is_pressed(const Cpu *cpu, uint8_t key) {
    uint8_t row = (uint8_t)((key & 0xF) >> 2);
    uint8_t col = key & 0x3;

    return cpu->keys[row][col] || cpu->key_state[row][col];
}

int keypad_first_pressed(const Cpu *cpu) {
    for (uint8_t key = 0; key < 16; key++) {
        if (keypad_is_pressed(cpu, key)) {
            return key;
        }
    }

    return -1;
}

void keypad_end_frame(Cpu *cpu) {
    memset(cpu->key_state, 0, sizeof cpu->key_state);
}
