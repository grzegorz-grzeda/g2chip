/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#ifndef G2CHIP_H
#define G2CHIP_H
/*--------------------------------------------------------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>
/*--------------------------------------------------------------------------------------------------------------------*/
#define G2CHIP_MEMORY_SIZE 4096
#define G2CHIP_DISPLAY_WIDTH 64
#define G2CHIP_DISPLAY_HEIGHT 32
#define G2CHIP_REGISTER_COUNT 16
#define G2CHIP_STACK_SIZE 16
#define G2CHIP_PROGRAM_START_ADDRESS 0x200
#define G2CHIP_MAX_ROM_SIZE (G2CHIP_MEMORY_SIZE - G2CHIP_PROGRAM_START_ADDRESS)
/*--------------------------------------------------------------------------------------------------------------------*/
typedef struct g2chip g2chip_t;
/*--------------------------------------------------------------------------------------------------------------------*/
typedef struct g2chip_config {
    uint32_t (*get_time_ms)(
        void); /**< Function pointer to get current time in milliseconds */
    void (*display_clear)(void);
    void (*display_draw_pixel)(uint8_t x, uint8_t y, uint8_t state);
    void (*display_refresh)(void);
    uint8_t (*key_is_pressed)(uint8_t key); /**< Check if key 0-F is pressed */
    uint8_t (*key_wait_press)(
        void); /**< Wait for any key press, return key value */

    void (*sound_beep_start)(void);
    void (*sound_beep_stop)(void);

    uint8_t (*get_random_byte)(
        void); /**< Function pointer to get a random byte */

    void (*debug_log)(const char* message);
} g2chip_config_t;
/*--------------------------------------------------------------------------------------------------------------------*/
g2chip_t* g2chip_create(const g2chip_config_t* config);
void g2chip_destroy(g2chip_t* chip);
int g2chip_load_rom(g2chip_t* chip, const uint8_t* rom_data, size_t size);
void g2chip_reset(g2chip_t* chip);
void g2chip_step(g2chip_t* chip);
/*--------------------------------------------------------------------------------------------------------------------*/
#endif  // G2CHIP_H