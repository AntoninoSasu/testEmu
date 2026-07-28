#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"

static const uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initChip8(Chip8 *chip8) {
    memset(chip8, 0, sizeof(Chip8));
    chip8->pc = START_ADDRESS;
    memcpy(chip8->memory + FONTSET_ADDRESS, fontset, FONTSET_SIZE);
}

bool loadRom(Chip8 *chip8, const char *filename) {
    FILE *file_rom = fopen(filename, "r");

    if (!file_rom) {
        fprintf(stderr, "Failed to open ROM: %s\n", filename);
        return false;
    }

    fseek(file_rom, 0, SEEK_END);
    long rom_size = ftell(file_rom);
    fseek(file_rom, 0, SEEK_SET);

    fread(chip8->memory + START_ADDRESS, 1, (size_t)rom_size, file_rom);
    fclose(file_rom);
    return true;
}

void executeOP(Chip8 *chip8) {
    // Fetch: opcodes are 2 bytes, big-endian
    chip8->opcode =
        (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];

    uint16_t opcode = chip8->opcode;
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t kk = opcode & 0x00FF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    // TEST:
    // printf("PC:0x%04x OP:0x%04x I:0x%04x\n", chip8->pc, opcode, chip8->I);
    chip8->pc += 2;

    // Decode
    switch (opcode & 0xF000) {
    // Execute
    case 0x0000:
        if (kk == 0XE0) { // Clears the screen
            memset(chip8->display, 0, sizeof(chip8->display));
            chip8->draw_flag = true;
        } else if (kk == 0xEE) { // Return from a subroutine
            if (chip8->sp <= 0) {
                fprintf(stderr, "Stack call, but stack is empty\n");
                showSystemState(chip8);
                exit(EXIT_FAILURE);
            }
            chip8->sp--;
            chip8->pc = chip8->stack[chip8->sp];
        }
        // Other 0nnn opcodes are ignored
        break;
    case 0x1000: // 1nnn: Jump to addres nnn
        chip8->pc = nnn;
        break;
    case 0x2000: // 2nnn: Call a subroutine at nnn
        if (chip8->sp >= STACK_SIZE) {
            fprintf(stderr, "Stack overflow\n");
            showSystemState(chip8);
            exit(EXIT_FAILURE);
        }
        chip8->stack[chip8->sp] = chip8->pc;
        chip8->sp++;
        chip8->pc = nnn;
        break;
    case 0x3000: // 3xkk: Skip next instruction if Vx == kk
        if (chip8->V[x] == kk)
            chip8->pc += 2;
        break;
    case 0x4000: // 4xkk: Skip next instruction if Vx != kk
        if (chip8->V[x] != kk)
            chip8->pc += 2;
        break;
    case 0x5000: // 5xy0: Skip next instruction if Vx == Vy
        if (n == 0) {
            if (chip8->V[x] == chip8->V[y])
                chip8->pc += 2;
        } else {
            fprintf(stderr, "Opcode is not valid: 0x%04X\n", opcode);
            showSystemState(chip8);
            exit(EXIT_FAILURE);
        }
        break;
    case 0x6000: // 6xkk: Sets Vx to kk
        chip8->V[x] = kk;
        break;
    case 0x7000: // 7xkk: Set Vx = Vx + kk;
        chip8->V[x] += kk;
        break;
    case 0x8000: // 8xyn: Register operations
        switch (n) {
        case 0x0: // Set Vx = Vy
            chip8->V[x] = chip8->V[y];
            break;
        case 0x1: // Set Vx = Vx OR Vy
            chip8->V[x] |= chip8->V[y];
            // NOTE: Chip8 quirk. See also in 0x2 and 0x3
            chip8->V[0xF] = 0;
            break;
        case 0x2: // Set Vx = Vx AND Vy
            chip8->V[x] &= chip8->V[y];
            chip8->V[0xF] = 0;
            break;
        case 0x3: // Set Vx = Vx XOR Vy
            chip8->V[x] ^= chip8->V[y];
            chip8->V[0xF] = 0;
            break;
        case 0x4: { // The values of Vx and Vy are added together. If the result
                    // is greater than 8 bits VF is set to 1, otherwise 0. Only
                    // the lowest 8 bits of the result are kept, and stored in
                    // Vx
            // NOTE: In these cases where V[0xF] flag is set, we gotta care
            // about the order of operations, because the V[0XF] itself could be
            // used as V[x] or V[y] and we must set the flag AFTER the
            // operations. This also happens in 0x5, 0x6, 0x7, 0xE.
            // We also have to be careful with the carry values.
            uint16_t aux = chip8->V[x] + chip8->V[y];
            chip8->V[x] = aux;
            chip8->V[0xF] = (aux > 255) ? 1 : 0;
            break;
        }
        case 0x5: { // If Vx >= Vy, then VF is set to 1, otherwise 0. Then Vy is
                    // subtracted from Vx, and the results stored in Vx
            uint8_t aux = (chip8->V[x] >= chip8->V[y]) ? 1 : 0;
            chip8->V[x] = chip8->V[x] - chip8->V[y];
            chip8->V[0xF] = aux;
            break;
        }
        case 0x6: { // If the least-significant bit of Vx is 1, then VF is set
                    // to
                    // 1, otherwise 0. Then Vx is divided by 2
            // NOTE: Chip8 quirk: whats stored in V[x] is the content if V[y]
            // shifted. See also in 0xE
            uint8_t aux = chip8->V[x];
            chip8->V[x] = chip8->V[y] >> 1;
            chip8->V[0xF] = aux & 1;
            break;
        }
        case 0x7: // If Vy >= Vx, then VF is set to 1, otherwise 0. Then Vx is
                  // subtracted from Vy, and the results stored in Vx
            chip8->V[x] = chip8->V[y] - chip8->V[x];
            chip8->V[0xF] = (chip8->V[y] >= chip8->V[x]) ? 1 : 0;
            break;
        case 0xE: { // If the most-significant bit of Vx is 1, then VF is set to
                    // 1, otherwise to 0. Then Vx is multiplied by 2
            uint8_t aux = chip8->V[x];
            chip8->V[x] = chip8->V[y] << 1;
            chip8->V[0xF] = (aux & 0x80) >> 7;
            break;
        }
        default:
            fprintf(stderr, "Opcode not valid: 0x%04X\n", opcode);
            showSystemState(chip8);
            exit(EXIT_FAILURE);
        }
        break;
    case 0x9000: // 9xy0: Skip next instruction if Vx != Vy
        if (n == 0) {
            if (chip8->V[x] != chip8->V[y])
                chip8->pc += 2;
        } else {
            fprintf(stderr, "Opcode is not valid: 0x%04X\n", opcode);
            showSystemState(chip8);
            exit(EXIT_FAILURE);
        }
        break;
    case 0xA000: // Annn: Sets I address to nnn
        chip8->I = nnn;
        break;
    case 0xB000: // Bnnn: Jump to location nnn + V0
        chip8->pc = nnn + chip8->V[0];
        break;
    case 0xC000: // Cxkk: Set Vx = random byte AND kk
        chip8->V[x] = rand() & kk;
        break;
    case 0xD000: // Dxyn: Draw a sprite at coordinates (VX, VY) with a width of
                 // 8 pixels and a height of N pixels

        // NOTE: Chip8 quirk: it can only draw once per frame
        if (chip8->waiting_for_vblank) {
            // If we didnt pass to a new frame we stay in this instruction
            chip8->pc -= 2;
            break;
        }
        chip8->waiting_for_vblank = true;

        // NOTE: Chip8 quirk: it has clipping
        chip8->V[0xF] = 0;

        uint8_t start_x = chip8->V[x] % DISPLAY_WIDTH;
        uint8_t start_y = chip8->V[y] % DISPLAY_HEIGHT;
        for (int row = 0; row < n; row++) {

            uint8_t py = start_y + row;
            if (py >= DISPLAY_HEIGHT) // Clipping: the rest of the sprite is
                break;                // off-screen

            uint8_t spriteByte = chip8->memory[chip8->I + row];
            for (int col = 0; col < 8; col++) {
                if (!(spriteByte & (0x80 >> col)))
                    continue;

                uint8_t px = start_x + col;
                if (px >= DISPLAY_WIDTH) // Clipping: this pixel is off-screen
                    continue;

                int idx = py * DISPLAY_WIDTH + px;
                if (chip8->display[idx])
                    chip8->V[0xF] = 1;
                chip8->display[idx] ^= 1; // Drawing is an XOR operation
            }
        }

        chip8->draw_flag = true;
        break;
    case 0xE000: // Input purposes
        if (kk == 0x9E) {
            if (chip8->keypad[chip8->V[x]])
                chip8->pc += 2;
        } else if (kk == 0xA1) {
            if (!chip8->keypad[chip8->V[x]])
                chip8->pc += 2;
        } else {
            fprintf(stderr, "Opcode is not valid: 0x%04X\n", opcode);
            showSystemState(chip8);
            exit(EXIT_FAILURE);
        }
        break;
    case 0xF000: // More general purposes
        switch (kk) {
        case 0x07: // Set Vx to delay timer value
            chip8->V[x] = chip8->delay_timer;
            break;
        case 0x0A: { // Wait for a key press, store the value of the key in Vx
            bool key_pressed = false;
            for (int i = 0; i < KEYPAD_SIZE; i++) {
                if (chip8->keypad[i]) {
                    chip8->V[x] = (uint8_t)i;
                    key_pressed = true;
                    break;
                }
            }
            if (!key_pressed) {
                chip8->pc -= 2; // Re-execute same instruction
            }
            break;
        }
        case 0x15: // Set delay timer value to Vx
            chip8->delay_timer = chip8->V[x];
            break;
        case 0x18: // Set sound timer value to Vx
            chip8->sound_timer = chip8->V[x];
            break;
        case 0x1E: // Set I = I + Vx
            chip8->I += chip8->V[x];
            break;
        case 0x29: // Set I = location of sprite for digit Vx
            chip8->I = FONTSET_ADDRESS + chip8->V[x] * 5;
            break;
        case 0x33: // Store BCD representation of Vx in memory locations I, I+1,
                   // and I+2
            chip8->memory[chip8->I] = chip8->V[x] / 100;
            chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
            chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
            break;
        case 0x55: // Store registers V0 through Vx in memory starting at
                   // location I
            for (int i = 0; i <= x; i++) {
                chip8->memory[chip8->I] = chip8->V[i];
                // NOTE: Chip8 quirk (instead of doing memory[I + i])
                // See also in 0x65
                chip8->I++;
            }
            break;
        case 0x65: // Read registers V0 through Vx from memory starting at
                   // location I
            for (int i = 0; i <= x; i++) {
                chip8->V[i] = chip8->memory[chip8->I];
                chip8->I++;
            }
            break;
        default:
            fprintf(stderr,
                    "Opcode not implemented yet (or not valid): 0x%04X\n",
                    opcode);
            showSystemState(chip8);
            exit(EXIT_FAILURE);
        }
        break;
    default:
        fprintf(stderr, "Unknown opcode: 0x%04X\n", opcode);
        showSystemState(chip8);
        exit(EXIT_FAILURE);
    }
}

void updateTimers(Chip8 *chip8) {
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;
    if (chip8->sound_timer > 0)
        chip8->sound_timer--;
}

void showSystemState(Chip8 *chip8) {
    printf("PC: 0x%04x\n\n", chip8->pc);
    printf("Memory: 0x%04x\n\n", chip8->memory[chip8->pc]);
    printf("Registers:\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("V%1x: 0x%04x\n", i, chip8->V[i]);
    }
    printf("\nI: 0x%04x\n\n", chip8->I);
    printf("Callstack:\n");
    while (chip8->sp > 0) {
        chip8->sp--;
        printf("0x%04x\n", chip8->stack[chip8->sp]);
    }
}
