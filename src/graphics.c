#include "graphics.h"
#include "chip8.h"
#include <stdio.h>

bool initGraphics(Graphics *gfx, int scale) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize. SLD_Error: %s\n", SDL_GetError());
        return false;
    }

    gfx->window = SDL_CreateWindow(
        "CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH * scale, DISPLAY_HEIGHT * scale, SDL_WINDOW_SHOWN);

    gfx->renderer =
        SDL_CreateRenderer(gfx->window, -1, SDL_RENDERER_ACCELERATED);
    // NOTE: If it cant find accelerator, uses CPU (rarely happens)
    if (!gfx->renderer) {
        fprintf(stderr, "Accelerated renderer failed: %s\n", SDL_GetError());
        gfx->renderer =
            SDL_CreateRenderer(gfx->window, -1, SDL_RENDERER_SOFTWARE);
    }

    gfx->texture = SDL_CreateTexture(gfx->renderer, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH,
                                     DISPLAY_HEIGHT);

    SDL_StopTextInput(); // Used to silence MacOs message when using unicode

    return gfx->window && gfx->renderer && gfx->texture;
}

void destroyGraphics(Graphics *gfx) {
    SDL_DestroyRenderer(gfx->renderer);
    SDL_DestroyWindow(gfx->window);
    SDL_Quit();
}

void drawGraphics(Graphics *gfx, Chip8 *chip8) {
    static uint32_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        pixels[i] =
            chip8->display[i] ? 0xFFFFFFFF : 0x000000FF; // white or black
    }

    SDL_UpdateTexture(gfx->texture, NULL, pixels,
                      DISPLAY_WIDTH * (int)sizeof(uint32_t));
    SDL_RenderClear(gfx->renderer);
    SDL_RenderCopy(gfx->renderer, gfx->texture, NULL, NULL);
    SDL_RenderPresent(gfx->renderer);

    chip8->draw_flag = false;
}
