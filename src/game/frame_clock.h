#pragma once
#include "../util/type.h"
#include <stdbool.h>

typedef struct FrameClock {
    u64 last;        
    u64 freq;
    u64 frameStart;
    f64 acc; 
} FrameClock;

void frame_clock_init(FrameClock *c);
f64  frame_clock_tick(FrameClock *c);                         
bool frame_clock_fixed_step(FrameClock *c, f64 step);       
/* 한 프레임의 남는 시간만큼 자서 targetFps 이상으로 돌지 않게 한다 */
void frame_clock_limit(FrameClock *c, f64 targetFps);        
