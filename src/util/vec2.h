#pragma once

#include "util/type.h"

typedef struct vec2 {
    f32 x;
    f32 y;
} vec2;

vec2 vec2_zero(void);
vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_scale(vec2 v, f32 s);

f32  vec2_dot(vec2 a, vec2 b);
f32  vec2_length_sq(vec2 v);
f32  vec2_length(vec2 v);
