#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "chip8.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} Graphics;

bool initGraphics(Graphics *gfx, int scale);
void destroyGraphics(Graphics *gfx);
void drawGraphics(Graphics *gfx, Chip8 *chip8);

#endif
