#include "collider.h"
#include <math.h>

#pragma once

#include <stdbool.h>
#include "fruit.h"
#include "state.h"
#include "../util/rect.h"
#include <math.h>

const float GRAVITY = 2000.0f;
const float RESTITUTION = 0.15f;    
const float FLOOR_FRICTION;
const float POSITION_SLOP;

bool collider_overlap(Fruit *a, Fruit *b) {
    float d = pow(b->pos.x - a->pos.x, 2) + pow(a->pos.y - b->pos.y, 2);
    float rs = pow(a->r + b->r, 2);

    return d <= rs;
}     
bool collider_overlap_wall(Fruit *f, rect bounds) {
    float x = f->pos.x, y = f->pos.y, r = f->r;
    vec2 vel = {0, 0};

    if (x - r <= bounds.left) {
        x = bounds.left + r;
        vel.x *= -1;
        return true;
    } else if (x + r >= bounds.right) {
        x = bounds.right - r;
        vel.x *= -1;
        return true;
    }

    if (y - r <= bounds.top) {
        y = bounds.top + r;
        vel.y *= -1;
        return true;
    } else if (y + r >= bounds.bottom) {
        y = bounds.bottom - r;
        vel.y *= -1;
        return true;
    }     
    return false;
}

void collider_resolve_pair(Fruit *a, Fruit *b); // 위치 보정 + 임펄스
void collider_resolve_wall(Fruit *f, rect bounds);

void collider_step(PlayerState *state, rect bounds, float dt) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];
        fruit->pos = vec2_add(fruit->pos, vec2_scale(fruit->vel, dt)); 
    }
}