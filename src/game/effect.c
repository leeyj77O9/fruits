#include "game/effect.h"

#include <math.h>
#include <stdlib.h>

static const f32 EFFECT_TWO_PI = 6.283185307179586f;
static const int EFFECT_SPARK_COUNT = 8;
static const f32 EFFECT_RING_DURATION = 0.34f;
static const f32 EFFECT_RING_GROWTH_SCALE = 2.5f;
static const f32 EFFECT_RING_RADIUS_SCALE = 0.85f;
static const f32 EFFECT_COLOR_SCALE = 0.72f;
static const f32 EFFECT_SPARK_DURATION_MIN = 0.26f;
static const f32 EFFECT_SPARK_DURATION_MAX = 0.44f;
static const f32 EFFECT_SPARK_SPEED_MIN = 120.0f;
static const f32 EFFECT_SPARK_SPEED_MAX = 250.0f;
static const f32 EFFECT_SPARK_GRAVITY = 950.0f;
static const f32 EFFECT_SPARK_RADIUS_MIN = 2.4f;
static const f32 EFFECT_SPARK_RADIUS_MAX = 4.4f;

static f32 effect_random01(void) {
    return (f32)(rand() % 1000) / 1000.0f;
}

static Effect *effect_alloc(EffectPool *pool) {
    for (int i = 0; i < EFFECT_POOL_SIZE; ++i) {
        if (!pool->items[i].active) {
            return &pool->items[i];
        }
    }

    return NULL;
}

void effect_pool_clear(EffectPool *pool) {
    for (int i = 0; i < EFFECT_POOL_SIZE; ++i) {
        pool->items[i] = (Effect){0};
    }
}

void effect_pool_spawn_merge(EffectPool *pool, vec2 pos, rgba color, f32 radius) {
    color = (rgba){
        (u8)((f32)color.r * EFFECT_COLOR_SCALE),
        (u8)((f32)color.g * EFFECT_COLOR_SCALE),
        (u8)((f32)color.b * EFFECT_COLOR_SCALE),
        color.a,
    };

    Effect *ring = effect_alloc(pool);

    if (ring != NULL) {
        *ring = (Effect){
            .pos = pos,
            .vel = vec2_zero(),
            .color = color,
            .life = EFFECT_RING_DURATION,
            .duration = EFFECT_RING_DURATION,
            .radius = radius * EFFECT_RING_RADIUS_SCALE,
            .growth = radius * EFFECT_RING_GROWTH_SCALE,
            .spark = false,
            .active = true,
        };
    }

    for (int i = 0; i < EFFECT_SPARK_COUNT; ++i) {
        Effect *spark = effect_alloc(pool);

        if (spark == NULL) {
            return;
        }

        f32 angle = EFFECT_TWO_PI * (f32)i / (f32)EFFECT_SPARK_COUNT;
        f32 spread = (effect_random01() - 0.5f) * 0.7f;
        f32 speed = EFFECT_SPARK_SPEED_MIN + effect_random01() * (EFFECT_SPARK_SPEED_MAX - EFFECT_SPARK_SPEED_MIN);

        speed *= 0.7f + radius / 70.0f;
        f32 duration = EFFECT_SPARK_DURATION_MIN + effect_random01() * (EFFECT_SPARK_DURATION_MAX - EFFECT_SPARK_DURATION_MIN);
        f32 size = EFFECT_SPARK_RADIUS_MIN + effect_random01() * (EFFECT_SPARK_RADIUS_MAX - EFFECT_SPARK_RADIUS_MIN);

        angle += spread;

        *spark = (Effect){
            .pos = {pos.x + cosf(angle) * radius * EFFECT_RING_RADIUS_SCALE, pos.y + sinf(angle) * radius * EFFECT_RING_RADIUS_SCALE},
            .vel = {cosf(angle) * speed, sinf(angle) * speed},
            .color = color,
            .life = duration,
            .duration = duration,
            .radius = size,
            .growth = 0.0f,
            .spark = true,
            .active = true,
        };
    }
}

void effect_pool_update(EffectPool *pool, f32 dt) {
    if (dt <= 0.0f) {
        return;
    }

    for (int i = 0; i < EFFECT_POOL_SIZE; ++i) {
        Effect *effect = &pool->items[i];

        if (!effect->active) {
            continue;
        }

        effect->life -= dt;

        if (effect->life <= 0.0f) {
            *effect = (Effect){0};
            continue;
        }

        effect->pos.x += effect->vel.x * dt;
        effect->pos.y += effect->vel.y * dt;

        if (effect->spark) {
            effect->vel.y += EFFECT_SPARK_GRAVITY * dt;
        } else {
            effect->radius += effect->growth * dt;
        }
    }
}
