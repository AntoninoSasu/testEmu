#include <stdio.h>
#include <stdlib.h>

#include "audio.h"
#include "chip8.h"
#include "graphics.h"
#include "input.h"

#define CYCLES_PER_FRAME 10
#define FRAMES_PER_SECOND 60

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("%s rom\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Chip8 chip8;
    Graphics gfx;
    Audio audio;

    initChip8(&chip8);
    // initinput();
    if (!initGraphics(&gfx, 16)) {
        fprintf(stderr, "initGraphics failed\n");
        exit(EXIT_FAILURE);
    }

    if (!initAudio(&audio)) {
        fprintf(stderr, "initAudio failed\n");
        exit(EXIT_FAILURE);
    }

    char *rom_name = argv[1];
    if (!loadRom(&chip8, rom_name)) {
        fprintf(stderr, "loadRom failed\n");
        exit(EXIT_FAILURE);
    }

    bool systemisrunning = true;

    // Main Loop
    while (systemisrunning) {
        Uint32 frame_start = SDL_GetTicks();

        systemisrunning = inputPoll(&chip8);

        chip8.waiting_for_vblank = false;

        // execute instruction. Run several CPU cycles per frame
        for (int i = 0; i < CYCLES_PER_FRAME; i++)
            executeOP(&chip8);

        // Timers tick at 60Hz
        updateTimers(&chip8);

        setBeep(&audio, chip8.sound_timer > 0);

        // Refresh display if flag is set
        if (chip8.draw_flag)
            drawGraphics(&gfx, &chip8);

        // Cap to 60 FPS
        Uint32 elapsed = SDL_GetTicks() - frame_start;
        const Uint32 frame_delay = 1000 / FRAMES_PER_SECOND;
        if (elapsed < frame_delay)
            SDL_Delay(frame_delay - elapsed);
    }

    destroyGraphics(&gfx);

    return 0;
}
