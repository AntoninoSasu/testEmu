#include "input.h"
#include <SDL2/SDL.h>

/*
 * Original keypad          Mapped to
 * 1 2 3 C                  1 2 3 4
 * 4 5 6 D        -->       Q W E R
 * 7 8 9 E                  A S D F
 * A 0 B F                  Z X C V
 */

static int keymap(SDL_KeyCode key) {
    switch (key) {
    case SDLK_1:
        return 0x1;
    case SDLK_2:
        return 0x2;
    case SDLK_3:
        return 0x3;
    case SDLK_4:
        return 0xC;
    case SDLK_q:
        return 0x4;
    case SDLK_w:
        return 0x5;
    case SDLK_e:
        return 0x6;
    case SDLK_r:
        return 0xD;
    case SDLK_a:
        return 0x7;
    case SDLK_s:
        return 0x8;
    case SDLK_d:
        return 0x9;
    case SDLK_f:
        return 0xE;
    case SDLK_z:
        return 0xA;
    case SDLK_x:
        return 0x0;
    case SDLK_c:
        return 0xB;
    case SDLK_v:
        return 0xF;
    default:
        return -1;
    }
}

bool inputPoll(Chip8 *chip8) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return false;
        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                return false;
            int k = keymap(event.key.keysym.sym);
            if (k >= 0)
                chip8->keypad[k] = 1;
            break;
        }
        case SDL_KEYUP: {
            int k = keymap(event.key.keysym.sym);
            if (k >= 0)
                chip8->keypad[k] = 0;
            break;
        }
        }
    }
    return true;
}
