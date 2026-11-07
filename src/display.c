#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "display.h"
#include "keypad.h"

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_SAMPLES_PER_FRAME (AUDIO_SAMPLE_RATE / CHIP8_FRAMES_PER_SECOND)
#define AUDIO_BEEP_HALF_PERIOD 50
#define AUDIO_BEEP_AMPLITUDE 3000
#define AUDIO_MAX_QUEUED_FRAMES 3

struct Display {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioDeviceID audio;
    unsigned beep_sample;
};

static const struct {
    SDL_Scancode scancode;
    uint8_t key;
} key_map[] = {
    {SDL_SCANCODE_1, 0x1}, {SDL_SCANCODE_2, 0x2}, {SDL_SCANCODE_3, 0x3}, {SDL_SCANCODE_4, 0xC},
    {SDL_SCANCODE_Q, 0x4}, {SDL_SCANCODE_W, 0x5}, {SDL_SCANCODE_E, 0x6}, {SDL_SCANCODE_R, 0xD},
    {SDL_SCANCODE_A, 0x7}, {SDL_SCANCODE_S, 0x8}, {SDL_SCANCODE_D, 0x9}, {SDL_SCANCODE_F, 0xE},
    {SDL_SCANCODE_Z, 0xA}, {SDL_SCANCODE_X, 0x0}, {SDL_SCANCODE_C, 0xB}, {SDL_SCANCODE_V, 0xF},
};

bool display_init(Display **out) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
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

    SDL_AudioSpec want = {0};
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;

    display->audio = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (display->audio == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->window);
        free(display);
        SDL_Quit();
        return false;
    }
    SDL_PauseAudioDevice(display->audio, 0);

    display->beep_sample = 0;
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

bool display_poll_events(Cpu *cpu) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        }

        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if (event.key.repeat) {
                continue;
            }

            for (size_t i = 0; i < sizeof key_map / sizeof key_map[0]; i++) {
                if (key_map[i].scancode == event.key.keysym.scancode) {
                    keypad_set(cpu, key_map[i].key, event.type == SDL_KEYDOWN);
                }
            }
        }
    }
    return false;
}

void display_update_sound(Display *display, const Cpu *cpu) {
    bool key_beep = cpu->flags.key_beep && keypad_first_pressed(cpu) >= 0;
    if (cpu->sound_tmr == 0 && !key_beep) {
        return;
    }

    Uint32 max_queued = (Uint32)(AUDIO_SAMPLES_PER_FRAME * sizeof(int16_t) * AUDIO_MAX_QUEUED_FRAMES);
    if (SDL_GetQueuedAudioSize(display->audio) > max_queued) {
        return;
    }

    int16_t samples[AUDIO_SAMPLES_PER_FRAME];
    for (int i = 0; i < AUDIO_SAMPLES_PER_FRAME; i++) {
        bool high = (display->beep_sample / AUDIO_BEEP_HALF_PERIOD) % 2;
        samples[i] = high ? AUDIO_BEEP_AMPLITUDE : -AUDIO_BEEP_AMPLITUDE;
        display->beep_sample = (display->beep_sample + 1) % (2 * AUDIO_BEEP_HALF_PERIOD);
    }

    SDL_QueueAudio(display->audio, samples, sizeof samples);
}

void display_wait_frame(void) {
    SDL_Delay(FRAME_MS);
}

void display_destroy(Display *display) {
    if (display == NULL) {
        return;
    }
    SDL_CloseAudioDevice(display->audio);
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    free(display);
    SDL_Quit();
}
