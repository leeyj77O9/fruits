#pragma once
#include "../util/type.h"
#include <stdbool.h>

typedef struct FrameClock {
    u64    last;        
    u64    freq;
    double acc; 
} FrameClock;

void   frame_clock_init(FrameClock *c);
double frame_clock_tick(FrameClock *c);                         
bool   frame_clock_fixed_step(FrameClock *c, double step);       