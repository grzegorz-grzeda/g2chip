/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "g2chip.h"
/*--------------------------------------------------------------------------------------------------------------------*/
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static uint32_t display_buffer[G2CHIP_DISPLAY_WIDTH * G2CHIP_DISPLAY_HEIGHT];
static uint8_t key_state[16] = {0};
static uint8_t waiting_for_key = 0;
static uint8_t last_pressed_key = 0;
/*--------------------------------------------------------------------------------------------------------------------*/
#define DISPLAY_SCALE 10
/*--------------------------------------------------------------------------------------------------------------------*/
static uint8_t sdl_key_to_chip8_key(SDL_Keycode key) {
    switch (key) {
        // Top row: 1,2,3,4 -> 1,2,3,C
        case SDLK_1:
            return 0x1;
        case SDLK_2:
            return 0x2;
        case SDLK_3:
            return 0x3;
        case SDLK_4:
            return 0xC;

        // Second row: Q,W,E,R -> 4,5,6,D
        case SDLK_q:
            return 0x4;
        case SDLK_w:
            return 0x5;
        case SDLK_e:
            return 0x6;
        case SDLK_r:
            return 0xD;

        // Third row: A,S,D,F -> 7,8,9,E
        case SDLK_a:
            return 0x7;
        case SDLK_s:
            return 0x8;
        case SDLK_d:
            return 0x9;
        case SDLK_f:
            return 0xE;

        // Fourth row: Z,X,C,V -> A,0,B,F
        case SDLK_z:
            return 0xA;
        case SDLK_x:
            return 0x0;
        case SDLK_c:
            return 0xB;
        case SDLK_v:
            return 0xF;

        default:
            return 0xFF;  // Invalid key
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static uint32_t get_time_ms_impl(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static int init_sdl_display(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow(
        "G2Chip CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, G2CHIP_DISPLAY_WIDTH * DISPLAY_SCALE,
        G2CHIP_DISPLAY_HEIGHT * DISPLAY_SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                G2CHIP_DISPLAY_WIDTH, G2CHIP_DISPLAY_HEIGHT);
    if (texture == NULL) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void cleanup_sdl_display(void) {
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void display_clear_impl(void) {
    for (int i = 0; i < G2CHIP_DISPLAY_WIDTH * G2CHIP_DISPLAY_HEIGHT; i++) {
        display_buffer[i] = 0x000000FF;  // Black with full alpha
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void display_draw_pixel_impl(uint8_t x, uint8_t y, uint8_t state) {
    if (x >= G2CHIP_DISPLAY_WIDTH || y >= G2CHIP_DISPLAY_HEIGHT)
        return;

    int index = y * G2CHIP_DISPLAY_WIDTH + x;
    display_buffer[index] = state ? 0xFFFFFFFF : 0x000000FF;  // White or Black
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void display_refresh_impl(void) {
    SDL_UpdateTexture(texture, NULL, display_buffer,
                      G2CHIP_DISPLAY_WIDTH * sizeof(uint32_t));

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static uint8_t key_is_pressed_impl(uint8_t key) {
    if (key >= 16)
        return 0;
    return key_state[key];
}
/*--------------------------------------------------------------------------------------------------------------------*/

static uint8_t key_wait_press_impl(void) {
    waiting_for_key = 1;

    SDL_Event event;
    while (waiting_for_key) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                waiting_for_key = 0;
                return 0;
            }

            if (event.type == SDL_KEYDOWN) {
                uint8_t chip8_key = sdl_key_to_chip8_key(event.key.keysym.sym);
                if (chip8_key != 0xFF) {
                    waiting_for_key = 0;
                    return chip8_key;
                }
            }
        }
        SDL_Delay(10);
    }

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void debug_log_impl(const char* message) {
    printf("[DEBUG] %s\n", message);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static uint8_t get_random_byte_impl(void) {
    return (uint8_t)(rand() % 256);
}
/*--------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <ROM file>\n", argv[0]);
        return -1;
    }
    if (init_sdl_display() != 0) {
        return -1;
    }

    const char* rom_filename = argv[1];
    FILE* rom_file = fopen(rom_filename, "rb");
    if (rom_file == NULL) {
        printf("Failed to open ROM file: %s\n", rom_filename);
        return -1;
    }
    fseek(rom_file, 0, SEEK_END);
    long rom_size = ftell(rom_file);
    printf("Loaded ROM '%s' size: %ld B\n", rom_filename, rom_size);
    fseek(rom_file, 0, SEEK_SET);
    uint8_t* rom_data = (uint8_t*)calloc(rom_size, sizeof(uint8_t));
    if (rom_data == NULL) {
        printf("Failed to allocate memory for ROM\n");
        fclose(rom_file);
        return -1;
    }
    fread(rom_data, 1, rom_size, rom_file);
    fclose(rom_file);

    g2chip_config_t config = {0};
    config.get_time_ms = get_time_ms_impl;
    config.display_clear = display_clear_impl;
    config.display_draw_pixel = display_draw_pixel_impl;
    config.display_refresh = display_refresh_impl;
    config.key_is_pressed = key_is_pressed_impl;
    config.key_wait_press = key_wait_press_impl;
    config.get_random_byte = get_random_byte_impl;
    config.sound_beep_start = NULL;  // Implement as needed
    config.sound_beep_stop = NULL;   // Implement as needed
    config.debug_log = debug_log_impl;

    g2chip_t* chip = g2chip_create(&config);
    if (chip == NULL) {
        printf("Failed to create G2Chip instance\n");
        free(rom_data);
        cleanup_sdl_display();
        return -1;
    }

    if (g2chip_load_rom(chip, rom_data, rom_size) != 0) {
        printf("Failed to load ROM into G2Chip\n");
        g2chip_destroy(chip);
        free(rom_data);
        cleanup_sdl_display();
        return -1;
    }
    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                uint8_t chip8_key = sdl_key_to_chip8_key(event.key.keysym.sym);
                if (chip8_key != 0xFF) {
                    key_state[chip8_key] = 1;
                    last_pressed_key = chip8_key;
                }
            } else if (event.type == SDL_KEYUP) {
                uint8_t chip8_key = sdl_key_to_chip8_key(event.key.keysym.sym);
                if (chip8_key != 0xFF) {
                    key_state[chip8_key] = 0;
                }
            }
        }

        g2chip_step(chip);

        SDL_Delay(1);  // Basic timing control
    }

    free(rom_data);
    g2chip_destroy(chip);
    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/