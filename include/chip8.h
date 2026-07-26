#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16
#define START_ADDRESS 0x200 // Place in memory where the interpreter space ends

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[NUM_REGISTERS]; // General purpose registers
    uint16_t I;               // Index register
    uint16_t pc;              // Program counter
    uint16_t stack[STACK_SIZE];
    uint8_t sp; // Stack pointer
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    uint8_t keypad[KEYPAD_SIZE];
    uint16_t opcode;

    bool draw_flag;
} Chip8;

void initChip8(Chip8 *chip8);
bool loadRom(Chip8 *chip8, const char *filename);
void executeOP(Chip8 *chip8);
void updateTimers(Chip8 *chip8);

// WARNING: When this function is called, it empties the stack (moves sp to 0)
// So an exit() should be called right afterwards
void showSystemState(Chip8 *chip8);

#endif
