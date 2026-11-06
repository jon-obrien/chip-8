#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include "cpu.h"

#define DISPLAY_SCALE 10
#define FRAME_MS (1000 / CHIP8_FRAMES_PER_SECOND)

typedef struct Display Display;

bool display_init(Display **display);
void display_draw(Display *display, const Cpu *cpu);
bool display_poll_quit(void);
void display_destroy(Display *display);
void display_wait_frame(void);

#endif
