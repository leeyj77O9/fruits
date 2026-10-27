#include "frame_clock.h"
#include <SDL3/SDL.h>

void frame_clock_init(FrameClock *c) {
    c->freq = SDL_GetPerformanceFrequency();
    c->last = SDL_GetPerformanceCounter();
    c->acc  = 0.0;
}

double frame_clock_tick(FrameClock *c) {
    u64 now = SDL_GetPerformanceCounter();
    double dt = (double)(now - c->last) / (double)c->freq;
    c->last = now;
    if (dt > 0.25) dt = 0.25;   
    c->acc += dt;
    return dt;
}

bool frame_clock_fixed_step(FrameClock *c, double step) {
    if (c->acc < step) return false;
    c->acc -= step;
    return true;
}