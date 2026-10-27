#include "vec2.h"
#include <math.h>

vec2 vec2_zero() {
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
vec2 vec2_normalize(vec2 v) {
    f32 length_sq = vec2_length_sq(v);

    if (length_sq == 0.0f) {
        return vec2_zero();
    } 
    return vec2_scale(v, 1.0f / sqrtf(length_sq));
}

int vec2_compare(vec2 a, vec2 b) {
    if (a.x < b.x) return -1;
    if (a.x > b.x) return 1;
    if (a.y < b.y) return -1;
    if (a.y > b.y) return 1;
    return 0;
}

bool vec2_equals(vec2 a, vec2 b) {
    return vec2_compare(a, b) == 0;
}
