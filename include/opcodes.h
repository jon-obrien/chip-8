#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

#include "cpu.h"

void opcode_execute(Cpu *cpu, uint16_t opcode);

#endif
