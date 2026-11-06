#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "display.h"

struct Display {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

bool display_init(Display **out) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    Display *display = malloc(sizeof *display);
    if (display == NULL) {
        SDL_Quit();
        return false;
    }

    display->window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        CHIP8_DISPLAY_WIDTH * DISPLAY_SCALE,
        CHIP8_DISPLAY_HEIGHT * DISPLAY_SCALE,
        0);
    if (display->window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        free(display);
        SDL_Quit();
        return false;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    if (display->renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(display->window);
        free(display);
        SDL_Quit();
        return false;
    }

    SDL_RenderSetScale(display->renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    *out = display;
    return true;
}

void display_draw(Display *display, const Cpu *cpu) {
    SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(display->renderer);

    SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);
    for (int x = 0; x < CHIP8_DISPLAY_WIDTH; x++) {
        for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
            if (cpu->display[x][y]) {
                SDL_Rect pixel = {x, y, 1, 1};
                SDL_RenderFillRect(display->renderer, &pixel);
            }
        }
    }

    SDL_RenderPresent(display->renderer);
}

bool display_poll_quit(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        }
    }
    return false;
}

void display_wait_frame(void) {
    SDL_Delay(FRAME_MS);
}

void display_destroy(Display *display) {
    if (display == NULL) {
        return;
    }
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    free(display);
    SDL_Quit();
}
