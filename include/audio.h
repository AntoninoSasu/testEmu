#ifndef AUDIO_H
#define AUDIO_H
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_AudioDeviceID device;
    bool playing;
    double phase;
} Audio;

bool initAudio(Audio *audio);
void destroyAudio(Audio *audio);
void setBeep(Audio *audio, bool on);

#endif
