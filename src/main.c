#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"
#include "graphics.h"
#include "input.h"

#define CYCLES_PER_FRAME 10 // nº of opcodes executed per 60Hz frame (~600)

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("%s rom\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Chip8 chip8;
    Graphics gfx;

    initChip8(&chip8);
    // initinput();
    if (!initGraphics(&gfx, 10)) {
        fprintf(stderr, "initGraphics failed\n");
        exit(EXIT_FAILURE);
    }

    char *rom_name = argv[1];
    if (!loadRom(&chip8, rom_name)) {
        fprintf(stderr, "loadRom failed\n");
        exit(EXIT_FAILURE);
    }

    bool systemisrunning = true;

    while (systemisrunning) {
        Uint32 frame_start = SDL_GetTicks();

        // execute instruction. Run several CPU cycles per frame
        for (int i = 0; i < CYCLES_PER_FRAME; i++)
            executeOP(&chip8);

        // Timers tick at 60Hz
        updateTimers(&chip8);

        // refresh display if flag is set
        if (chip8.draw_flag)
            drawGraphics(&gfx, &chip8);

        // play beep if flag is set
        if (chip8.sound_timer > 0) {
            // TODO: play beep
        }

        // set input and set keys states
        // setinput();

        // Cap to 60 FPS
        Uint32 elapsed = SDL_GetTicks() - frame_start;
        const Uint32 frame_delay = 1000 / 60;
        if (elapsed < frame_delay)
            SDL_Delay(frame_delay - elapsed);
    }

    destroyGraphics(&gfx);

    return 0;
}
