#include "audio.h"
#include <stdio.h>

#define SAMPLE_RATE 44100
#define AMPLITUDE 3000
#define FREQUENCY 440.0 // The selected A4 tone

static void audioCallback(void *userdata, Uint8 *stream, int len) {
    Audio *audio = (Audio *)userdata;
    Sint16 *buffer = (Sint16 *)stream;
    int samples = len / (int)sizeof(Sint16);

    for (int i = 0; i < samples; i++) {
        if (audio->playing) {
            // Play a square wave sound: high first half, low second half
            buffer[i] = (audio->phase < 0.5) ? AMPLITUDE : -AMPLITUDE;
            audio->phase += FREQUENCY / SAMPLE_RATE;
            if (audio->phase >= 1.0)
                audio->phase -= 1.0;
        } else {
            buffer[i] = 0;
        }
    }
}

bool initAudio(Audio *audio) {
    audio->playing = false;
    audio->phase = 0.0;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_InitSubSystem(AUDIO) failed: %s\n",
                SDL_GetError());
        return false;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audioCallback;
    want.userdata = audio;

    audio->device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio->device == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_PauseAudioDevice(audio->device, 0);
    return true;
}

void setBeep(Audio *audio, bool on) { audio->playing = on; }

void destroyAudio(Audio *audio) {
    if (audio->device)
        SDL_CloseAudioDevice(audio->device);
}
