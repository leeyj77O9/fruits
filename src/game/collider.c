#include "collider.h"
#include <math.h>

const float GRAVITY = 2000.0f;
const float RESTITUTION = 0.15f;
const float FLOOR_FRICTION = 0.98f;
const float POSITION_SLOP = 0.01f;

bool collider_overlap(Fruit *a, Fruit *b) {
    float dx = b->pos.x - a->pos.x;
    float dy = b->pos.y - a->pos.y;
    float radii = a->r + b->r;

    return (dx * dx) + (dy * dy) <= radii * radii;
}
bool collider_overlap_wall(Fruit *f, rect bounds) {
    return f->pos.x - f->r <= bounds.left
        || f->pos.x + f->r >= bounds.right
        || f->pos.y - f->r <= bounds.top
        || f->pos.y + f->r >= bounds.bottom;
}

void collider_resolve_pair(Fruit *a, Fruit *b) {
    vec2 delta = vec2_sub(b->pos, a->pos);
    float dist_sq = vec2_length_sq(delta);
    float radii = a->r + b->r;

    if (dist_sq >= radii * radii) {
        return;
    }

    float dist = sqrtf(dist_sq);
    vec2 normal = dist > 0.0f ? vec2_scale(delta, 1.0f / dist) : (vec2){1.0f, 0.0f};
    float overlap = radii - dist;

    if (overlap <= POSITION_SLOP) {
        return;
    }

    // 질량은 넓이에 비례한다고 가정
    float mass_a = a->r * a->r;
    float mass_b = b->r * b->r;
    float inv_total = 1.0f / (mass_a + mass_b);

    // 위치 보정 (가벼운 쪽이 더 많이 밀림)
    a->pos = vec2_sub(a->pos, vec2_scale(normal, overlap * mass_b * inv_total));
    b->pos = vec2_add(b->pos, vec2_scale(normal, overlap * mass_a * inv_total));

    // 상대 속도가 이미 떨어지는 방향이면 반발 없음
    float along = vec2_dot(vec2_sub(b->vel, a->vel), normal);
    if (along > 0.0f) {
        return;
    }

    float impulse = -(1.0f + RESTITUTION) * along / ((1.0f / mass_a) + (1.0f / mass_b));
    a->vel = vec2_sub(a->vel, vec2_scale(normal, impulse / mass_a));
    b->vel = vec2_add(b->vel, vec2_scale(normal, impulse / mass_b));
}

void collider_resolve_wall(Fruit *f, rect bounds) {
    float left = (float)bounds.left;
    float right = (float)bounds.right;
    float top = (float)bounds.top;
    float bottom = (float)bounds.bottom;

    if (f->pos.x - f->r < left) {
        f->pos.x = left + f->r;
        if (f->vel.x < 0.0f) {
            f->vel.x = -f->vel.x * RESTITUTION;
        }
    } else if (f->pos.x + f->r > right) {
        f->pos.x = right - f->r;
        if (f->vel.x > 0.0f) {
            f->vel.x = -f->vel.x * RESTITUTION;
        }
    }

    if (f->pos.y - f->r < top) {
        f->pos.y = top + f->r;
        if (f->vel.y < 0.0f) {
            f->vel.y = -f->vel.y * RESTITUTION;
        }
    } else if (f->pos.y + f->r > bottom) {
        f->pos.y = bottom - f->r;
        if (f->vel.y > 0.0f) {
            f->vel.y = -f->vel.y * RESTITUTION;

            /* 아주 작은 반발은 멈춰서 미세 진동을 막는다 */
            if (fabsf(f->vel.y) < 10.0f) {
                f->vel.y = 0.0f;
            }
        }
        f->vel.x *= FLOOR_FRICTION;
    }
}

void collider_step(PlayerState *state, rect bounds, float dt) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];

        fruit->vel.y += GRAVITY * dt;
        fruit->pos = vec2_add(fruit->pos, vec2_scale(fruit->vel, dt));
    }

    /* 겹침을 여러 번 풀어야 여러 개가 동시에 겹친 상황도 안정적으로 분리된다 */
    for (int iteration = 0; iteration < 4; ++iteration) {
        for (size_t i = 0; i < state->fruitsCount; ++i) {
            collider_resolve_wall(&state->fruits[i], bounds);
        }

        for (size_t i = 0; i < state->fruitsCount; ++i) {
            for (size_t j = i + 1; j < state->fruitsCount; ++j) {
                Fruit *a = &state->fruits[i];
                Fruit *b = &state->fruits[j];

                if (collider_overlap(a, b)) {
                    collider_resolve_pair(a, b);
                }
            }
        }
    }

    /* 마지막 쌍 보정으로 밀려난 과일이 경계를 벗어나지 않도록 한 번 더 정리 */
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        collider_resolve_wall(&state->fruits[i], bounds);
    }
}
