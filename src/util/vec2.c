#include "util/vec2.h"
#include <math.h>

vec2 vec2_zero(void) {
    return (vec2){0.0f, 0.0f};
}

vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2){a.x + b.x, a.y + b.y};
}

vec2 vec2_sub(vec2 a, vec2 b) {
    return (vec2){a.x - b.x, a.y - b.y};
}

vec2 vec2_scale(vec2 v, f32 s) {
    return (vec2){v.x * s, v.y * s};
}

f32 vec2_dot(vec2 a, vec2 b) {
    return (a.x * b.x) + (a.y * b.y);
}

f32 vec2_length_sq(vec2 v) {
    return vec2_dot(v, v);
}

f32 vec2_length(vec2 v) {
    return sqrtf(vec2_length_sq(v));
}
