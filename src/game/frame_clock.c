#include "game/frame_clock.h"
#include <SDL3/SDL.h>

void frame_clock_init(FrameClock *c) {
    c->freq = SDL_GetPerformanceFrequency();
    c->last = SDL_GetPerformanceCounter();
    c->frameStart = c->last;
    c->acc = 0.0;
}

f64 frame_clock_tick(FrameClock *c) {
    u64 now = SDL_GetPerformanceCounter();
    f64 dt = (f64)(now - c->last) / (f64)c->freq;

    c->last = now;
    if (dt > 0.25) {
        dt = 0.25;
    }
    c->acc += dt;

    return dt;
}

bool frame_clock_fixed_step(FrameClock *c, f64 step) {
    if (c->acc < step) {
        return false;
    }

    c->acc -= step;
    return true;
}

void frame_clock_limit(FrameClock *c, f64 targetFps) {
    u64 now = SDL_GetPerformanceCounter();

    if (targetFps > 0.0) {
        f64 target = 1.0 / targetFps;
        f64 elapsed = (f64)(now - c->frameStart) / (f64)c->freq;
        f64 remaining = target - elapsed;

        if (remaining > 0.0) {
            SDL_DelayPrecise((u64)(remaining * 1000000000.0));
        }
    }

    c->frameStart = SDL_GetPerformanceCounter();
}
