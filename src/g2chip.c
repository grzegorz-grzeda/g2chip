/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include "g2chip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*--------------------------------------------------------------------------------------------------------------------*/
#define G2CHIP_REGISTER_INDEX_LAST (G2CHIP_REGISTER_COUNT - 1)
#define G2CHIP_FONT_START_ADDRESS 0x50
#define G2CHIP_FONT_SIZE (16 * 5)
/*--------------------------------------------------------------------------------------------------------------------*/
typedef struct g2chip_instruction {
    uint16_t raw;
    uint16_t opcode;
    uint8_t x;
    uint8_t y;
    uint8_t n;
    uint8_t nn;
    uint16_t nnn;
} g2chip_instruction_t;
/*--------------------------------------------------------------------------------------------------------------------*/
typedef void (*instruction_handler_t)(g2chip_t* chip,
                                      g2chip_instruction_t* instr);
/*--------------------------------------------------------------------------------------------------------------------*/
typedef struct g2chip {
    g2chip_config_t config;
    uint8_t memory[G2CHIP_MEMORY_SIZE];
    uint8_t display[G2CHIP_DISPLAY_WIDTH * G2CHIP_DISPLAY_HEIGHT];
    uint8_t V[G2CHIP_REGISTER_COUNT];
    uint16_t stack[G2CHIP_STACK_SIZE];
    uint32_t last_time_ms;
    uint16_t I;
    uint16_t pc;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t sp;
} g2chip_t;
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_0(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_1(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_2(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_3(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_4(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_5(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_6(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_7(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_8(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_9(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_A(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_B(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_C(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_D(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_E(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
static void instruction_handler_opcode_F(g2chip_t* chip,
                                         g2chip_instruction_t* instr);
/*--------------------------------------------------------------------------------------------------------------------*/
static const instruction_handler_t instruction_handlers[] = {
    instruction_handler_opcode_0, instruction_handler_opcode_1,
    instruction_handler_opcode_2, instruction_handler_opcode_3,
    instruction_handler_opcode_4, instruction_handler_opcode_5,
    instruction_handler_opcode_6, instruction_handler_opcode_7,
    instruction_handler_opcode_8, instruction_handler_opcode_9,
    instruction_handler_opcode_A, instruction_handler_opcode_B,
    instruction_handler_opcode_C, instruction_handler_opcode_D,
    instruction_handler_opcode_E, instruction_handler_opcode_F,
};
/*--------------------------------------------------------------------------------------------------------------------*/
static const uint8_t g2chip_font_data[16 * 5] = {
    // 0
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    // 1
    0x20, 0x60, 0x20, 0x20, 0x70,
    // 2
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    // 3
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    // 4
    0x90, 0x90, 0xF0, 0x10, 0x10,
    // 5
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    // 6
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    // 7
    0xF0, 0x10, 0x20, 0x40, 0x40,
    // 8
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    // 9
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    // A
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    // B
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    // C
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    // D
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    // E
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    // F
    0xF0, 0x80, 0xF0, 0x80, 0x80};
/*--------------------------------------------------------------------------------------------------------------------*/
g2chip_t* g2chip_create(const g2chip_config_t* config) {
    if (config == NULL) {
        return NULL;
    }

    g2chip_t* chip = (g2chip_t*)calloc(1, sizeof(g2chip_t));
    if (chip == NULL) {
        return NULL;
    }

    chip->config = *config;
    g2chip_reset(chip);

    return chip;
}
/*--------------------------------------------------------------------------------------------------------------------*/
void g2chip_destroy(g2chip_t* chip) {
    if (chip != NULL) {
        free(chip);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
int g2chip_load_rom(g2chip_t* chip, const uint8_t* rom_data, size_t size) {
    if (chip == NULL || rom_data == NULL || size == 0 ||
        size > G2CHIP_MAX_ROM_SIZE) {
        return -1;
    }

    for (size_t i = 0; i < size; i++) {
        chip->memory[G2CHIP_PROGRAM_START_ADDRESS + i] = rom_data[i];
    }

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void load_font_data(g2chip_t* chip) {
    for (size_t i = 0; i < G2CHIP_FONT_SIZE; i++) {
        chip->memory[G2CHIP_FONT_START_ADDRESS + i] = g2chip_font_data[i];
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
void g2chip_reset(g2chip_t* chip) {
    if (chip == NULL) {
        return;
    }

    memset(chip->memory, 0, G2CHIP_MEMORY_SIZE);
    load_font_data(chip);

    memset(chip->V, 0, sizeof(chip->V));
    memset(chip->stack, 0, sizeof(chip->stack));
    chip->I = 0;
    chip->pc = G2CHIP_PROGRAM_START_ADDRESS;
    chip->sp = 0;

    memset(chip->display, 0, sizeof(chip->display));
    if (chip->config.display_clear) {
        chip->config.display_clear();
    }

    chip->delay_timer = 0;
    chip->sound_timer = 0;
    if (chip->config.get_time_ms) {
        chip->last_time_ms = chip->config.get_time_ms();
    } else {
        chip->last_time_ms = 0;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void update_timers(g2chip_t* chip) {
    if (chip->config.get_time_ms == NULL) {
        return;
    }

    uint32_t current_time = chip->config.get_time_ms();
    uint32_t elapsed = current_time - chip->last_time_ms;

    if (elapsed >= 16) {
        if (chip->delay_timer > 0) {
            chip->delay_timer--;
        }

        if (chip->sound_timer > 0) {
            chip->sound_timer--;
            if (chip->sound_timer == 0 && chip->config.sound_beep_stop) {
                chip->config.sound_beep_stop();
            }
        } else if (chip->sound_timer > 0 && chip->config.sound_beep_start) {
            chip->config.sound_beep_start();
        }

        chip->last_time_ms = current_time;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_clear_display(g2chip_t* chip) {
    memset(chip->display, 0, sizeof(chip->display));
    if (chip->config.display_clear) {
        chip->config.display_clear();
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_return_from_subroutine(g2chip_t* chip) {
    if (chip->sp == 0) {
        if (chip->config.debug_log) {
            char buffer[64];
            sprintf(buffer, "Stack underflow on RET for pc=%04X", chip->pc);
            chip->config.debug_log(buffer);
        }
        return;
    }
    chip->sp--;
    chip->pc = chip->stack[chip->sp];
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_jump_to_address(g2chip_t* chip, uint16_t address) {
    chip->pc = address;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void draw_sprite(g2chip_t* chip, uint8_t x, uint8_t y, uint8_t height) {
    chip->V[G2CHIP_REGISTER_INDEX_LAST] = 0;  // Clear collision flag

    for (int row = 0; row < height; row++) {
        uint8_t sprite_byte = chip->memory[chip->I + row];

        for (int col = 0; col < 8; col++) {
            if ((sprite_byte & (0x80 >> col)) != 0) {
                int px = (x + col) % G2CHIP_DISPLAY_WIDTH;
                int py = (y + row) % G2CHIP_DISPLAY_HEIGHT;
                int index = py * G2CHIP_DISPLAY_WIDTH + px;

                if (chip->display[index] == 1) {
                    chip->V[G2CHIP_REGISTER_INDEX_LAST] =
                        1;  // Collision detected
                }

                chip->display[index] ^= 1;  // XOR pixel

                if (chip->config.display_draw_pixel) {
                    chip->config.display_draw_pixel(px, py,
                                                    chip->display[index]);
                }
            }
        }
    }

    if (chip->config.display_refresh) {
        chip->config.display_refresh();
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static g2chip_instruction_t fetch_instruction(g2chip_t* chip) {
    g2chip_instruction_t instr;
    memset(&instr, 0, sizeof(instr));
    instr.raw = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
    instr.opcode = (instr.raw & 0xF000) >> 12;
    instr.x = (instr.raw & 0x0F00) >> 8;
    instr.y = (instr.raw & 0x00F0) >> 4;
    instr.n = instr.raw & 0x000F;
    instr.nn = instr.raw & 0x00FF;
    instr.nnn = instr.raw & 0x0FFF;
    return instr;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_not_implemented(g2chip_t* chip,
                                        g2chip_instruction_t* instr) {
    if (chip->config.debug_log) {
        char msg[64];
        snprintf(msg, sizeof(msg),
                 "Instruction not implemented: 0x%04X at pc=0x%04X", instr->raw,
                 chip->pc - 2);
        chip->config.debug_log(msg);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_0(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    if (instr->nnn == 0x0E0) {
        instruction_clear_display(chip);
    } else if (instr->nnn == 0x0EE) {
        instruction_return_from_subroutine(chip);
    } else {
        instruction_not_implemented(chip, instr);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_1(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    instruction_jump_to_address(chip, instr->nnn);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_2(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    chip->stack[chip->sp] = chip->pc;
    chip->sp++;
    instruction_jump_to_address(chip, instr->nnn);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_3(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    if (chip->V[instr->x] == instr->nn) {
        chip->pc += 2;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_4(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    if (chip->V[instr->x] != instr->nn) {
        chip->pc += 2;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_5(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    if (chip->V[instr->x] == chip->V[instr->y]) {
        chip->pc += 2;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_6(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    chip->V[instr->x] = instr->nn;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_7(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    chip->V[instr->x] += instr->nn;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_8(g2chip_t* chip,
                                         g2chip_instruction_t* instruction) {
    switch (instruction->n) {
        case 0x0:
            chip->V[instruction->x] = chip->V[instruction->y];
            break;
        case 0x1:
            chip->V[instruction->x] |= chip->V[instruction->y];
            break;
        case 0x2:
            chip->V[instruction->x] &= chip->V[instruction->y];
            break;
        case 0x3:
            chip->V[instruction->x] ^= chip->V[instruction->y];
            break;
        case 0x4: {
            uint16_t sum = chip->V[instruction->x] + chip->V[instruction->y];
            chip->V[G2CHIP_REGISTER_INDEX_LAST] = (sum > 0xFF) ? 1 : 0;
            chip->V[instruction->x] = sum & 0xFF;
            break;
        }
        case 0x5:
            chip->V[G2CHIP_REGISTER_INDEX_LAST] =
                (chip->V[instruction->x] > chip->V[instruction->y]) ? 1 : 0;
            chip->V[instruction->x] -= chip->V[instruction->y];
            break;
        case 0x6:
            chip->V[G2CHIP_REGISTER_INDEX_LAST] = chip->V[instruction->x] & 0x1;
            chip->V[instruction->x] >>= 1;
            break;
        case 0x7:
            chip->V[G2CHIP_REGISTER_INDEX_LAST] =
                (chip->V[instruction->y] > chip->V[instruction->x]) ? 1 : 0;
            chip->V[instruction->x] =
                chip->V[instruction->y] - chip->V[instruction->x];
            break;
        case 0xE:
            chip->V[G2CHIP_REGISTER_INDEX_LAST] =
                (chip->V[instruction->x] & 0x80) >> 7;
            chip->V[instruction->x] <<= 1;
            break;
        default:
            instruction_not_implemented(chip, instruction);
            break;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_9(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    if (chip->V[instr->x] != chip->V[instr->y]) {
        chip->pc += 2;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_A(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    chip->I = instr->nnn;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_B(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    chip->pc = instr->nnn + chip->V[0];
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_C(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    if (!chip->config.get_random_byte) {
        if (chip->config.debug_log) {
            chip->config.debug_log("Random byte generator not implemented");
        }
        return;
    }
    uint8_t rand_byte = chip->config.get_random_byte();
    chip->V[instr->x] = rand_byte & instr->nn;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_D(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    draw_sprite(chip, chip->V[instr->x], chip->V[instr->y], instr->n);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_E(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    if (instr->nn == 0x9E) {
        if (chip->config.key_is_pressed &&
            chip->config.key_is_pressed(chip->V[instr->x])) {
            chip->pc += 2;
        }
    } else if (instr->nn == 0xA1) {
        if (chip->config.key_is_pressed &&
            !chip->config.key_is_pressed(chip->V[instr->x])) {
            chip->pc += 2;
        }
    } else {
        instruction_not_implemented(chip, instr);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void instruction_handler_opcode_F(g2chip_t* chip,
                                         g2chip_instruction_t* instr) {
    switch (instr->nn) {
        case 0x07:
            chip->V[instr->x] = chip->delay_timer;
            break;
        case 0x0A:
            if (chip->config.key_wait_press) {
                chip->V[instr->x] = chip->config.key_wait_press();
            }
            break;
        case 0x15:
            chip->delay_timer = chip->V[instr->x];
            break;
        case 0x18:
            chip->sound_timer = chip->V[instr->x];
            if (chip->V[instr->x] > 0 && chip->config.sound_beep_start) {
                chip->config.sound_beep_start();
            }
            break;
        case 0x1E:
            chip->I += chip->V[instr->x];
            break;
        case 0x29:
            if (chip->V[instr->x] <= 0xF) {
                chip->I = G2CHIP_FONT_START_ADDRESS + (chip->V[instr->x] * 5);
            } else {
                if (chip->config.debug_log) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "Invalid font character: 0x%02X",
                             chip->V[instr->x]);
                    chip->config.debug_log(msg);
                }
            }
            break;
        case 0x33:
            chip->memory[chip->I] = chip->V[instr->x] / 100;
            chip->memory[chip->I + 1] = (chip->V[instr->x] / 10) % 10;
            chip->memory[chip->I + 2] = chip->V[instr->x] % 10;
            break;
        case 0x55:
            for (uint8_t i = 0; i <= instr->x; i++) {
                chip->memory[chip->I + i] = chip->V[i];
            }
            break;
        case 0x65:
            for (uint8_t i = 0; i <= instr->x; i++) {
                chip->V[i] = chip->memory[chip->I + i];
            }
            break;
        default:
            instruction_not_implemented(chip, instr);
            break;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void execute_step(g2chip_t* chip) {
    g2chip_instruction_t instruction = fetch_instruction(chip);
    chip->pc += 2;
    instruction_handlers[instruction.opcode](chip, &instruction);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void g2chip_step(g2chip_t* chip) {
    if (!chip) {
        return;
    }
    update_timers(chip);
    execute_step(chip);
}
/*--------------------------------------------------------------------------------------------------------------------*/
