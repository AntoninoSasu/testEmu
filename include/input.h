#ifndef INPUT_H
#define INPUT_H
#include "chip8.h"
#include <stdbool.h>

// Polls SDL events, updates chip8->keypad[]
// Returns false if the user requested to quit (window close / Esc)
bool inputPoll(Chip8 *chip8);

#endif
