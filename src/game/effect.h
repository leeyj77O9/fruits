#pragma once

#include <stdbool.h>

#include "util/rgba.h"
#include "util/type.h"
#include "util/vec2.h"

enum {
    EFFECT_POOL_SIZE = 96,
};

typedef struct Effect {
    vec2 pos;
    vec2 vel;
    rgba color;
    f32  life;
    f32  duration;
    f32  radius;
    f32  growth;
    bool spark;
    bool active;
} Effect;

typedef struct EffectPool {
    Effect items[EFFECT_POOL_SIZE];
} EffectPool;

void effect_pool_clear(EffectPool *pool);
void effect_pool_spawn_merge(EffectPool *pool, vec2 pos, rgba color, f32 radius);
void effect_pool_update(EffectPool *pool, f32 dt);
