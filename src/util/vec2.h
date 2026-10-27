#pragma once

#include <stdbool.h>
#include "type.h"

typedef struct vec2 {
    f32 x;
    f32 y;
} vec2;

vec2  vec2_zero();
vec2  vec2_add(vec2 a, vec2 b);
vec2  vec2_sub(vec2 a, vec2 b);
vec2  vec2_scale(vec2 v, f32 s);

f32 vec2_dot(vec2 a, vec2 b);
f32 vec2_length_sq(vec2 v);
vec2  vec2_normalize(vec2 v);

int   vec2_compare(vec2 a, vec2 b);
bool  vec2_equals(vec2 a, vec2 b);

