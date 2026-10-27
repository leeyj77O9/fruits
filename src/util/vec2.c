#include "vec2.h"

vec2 vec2_zero() {
    return (vec2){0.0f, 0.0f};
}
vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2){a.x + b.x, a.y + b.y};
}
vec2 vec2_sub(vec2 a, vec2 b) {
    return (vec2){a.x - b.x, a.y - b.y};
}
vec2 vec2_scale(vec2 v, float s) {
    return (vec2){v.x * s, v.y * s};
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
