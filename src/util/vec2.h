#pragma once

#include <stdbool.h>

typedef struct vec2 {
    float x;
    float y;
} vec2;

vec2 vec2_zero();
vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_scale(vec2 v, float s);

int vec2_compare(vec2 a, vec2 b);
bool vec2_equals(vec2 a, vec2 b);

