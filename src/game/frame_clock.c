#include "frame_clock.h"
#include <SDL3/SDL.h>

void frame_clock_init(FrameClock *c) {
    c->freq = SDL_GetPerformanceFrequency();
    c->last = SDL_GetPerformanceCounter();
    c->frameStart = c->last;
    c->acc  = 0.0;
}

f64 frame_clock_tick(FrameClock *c) {
    u64 now = SDL_GetPerformanceCounter();
    f64 dt = (f64)(now - c->last) / (f64)c->freq;
    c->last = now;
    if (dt > 0.25) dt = 0.25;   
    c->acc += dt;
    return dt;
}

bool frame_clock_fixed_step(FrameClock *c, f64 step) {
    if (c->acc < step) return false;
    c->acc -= step;
    return true;
}

void frame_clock_limit(FrameClock *c, f64 targetFps) {
    u64 now = SDL_GetPerformanceCounter();
    f64 target = 1.0 / targetFps;

    if (targetFps > 0.0) {
        f64 elapsed = (f64)(now - c->frameStart) / (f64)c->freq;
        f64 remaining = target - elapsed;

        /* vsync가 실제로 동작하면 남는 시간이 없으므로 그냥 지나간다.
           vsync가 무시되는 환경에서도 CPU를 100% 쓰지 않도록 여기서 상한을 건다 */
        if (remaining > 0.0) {
            SDL_DelayPrecise((u64)(remaining * 1000000000.0));
        }
    }

    c->frameStart = SDL_GetPerformanceCounter();
}
