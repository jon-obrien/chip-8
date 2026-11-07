#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdbool.h>

#include "cpu.h"

void keypad_set(Cpu *cpu, uint8_t key, bool pressed);
bool keypad_is_pressed(const Cpu *cpu, uint8_t key);
int keypad_first_pressed(const Cpu *cpu);

#endif
