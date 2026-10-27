#pragma once

#include <stdbool.h>

#include "util/type.h"

typedef struct FrameClock {
    u64 last;
    u64 freq;
    u64 frameStart;
    f64 acc;
} FrameClock;

void frame_clock_init(FrameClock *c);
f64  frame_clock_tick(FrameClock *c);
bool frame_clock_fixed_step(FrameClock *c, f64 step);
void frame_clock_limit(FrameClock *c, f64 targetFps);
