#include "collider.h"
#include <math.h>

const float GRAVITY = 2000.0f;
const float RESTITUTION = 0.15f;
const float FLOOR_FRICTION = 0.85f;
const float PAIR_FRICTION = 0.3f;
const float POSITION_SLOP = 0.5f;

/* 이 속도보다 느린 충격은 튕기지 않게 해서 더미가 영원히 떨리는 것을 막는다 */
static const float RESTITUTION_SPEED_THRESHOLD = 100.0f;
/* 한 번에 겹침을 100% 풀면 튀어오르므로 일부만 보정한다 */
static const float CORRECTION_PERCENT = 0.6f;
/* 이 속도 이하로 이만큼 연속 유지되면 정지 상태로 본다 */
static const float SLEEP_SPEED = 40.0f;
static const u16 SLEEP_FRAMES = 30;
/* 정지해 있는 과일을 깨우는 충격 속도 */
static const float WAKE_SPEED = 50.0f;

static const int SOLVER_ITERATIONS = 8;
static const int MAX_SUBSTEPS = 8;

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

    /* 둘 다 자고 있으면 계산할 필요 없음 */
    if (a->sleeping && b->sleeping) {
        return;
    }

    /* 자는 과일을 충분히 빠른 상대가 건드리면 깨운다 */
    if (a->sleeping && vec2_length_sq(b->vel) > WAKE_SPEED * WAKE_SPEED) {
        a->sleeping = false;
        a->sleepFrames = 0;
    }
    if (b->sleeping && vec2_length_sq(a->vel) > WAKE_SPEED * WAKE_SPEED) {
        b->sleeping = false;
        b->sleepFrames = 0;
    }

    float dist = sqrtf(dist_sq);
    vec2 normal;

    /* 완전히 같은 위치면 법선이 없으므로 id로 결정되는 각도로 흩어뜨린다.
       (한 방향으로만 밀면 과일이 전부 옆으로 늘어서 한 줄이 된다) */
    if (dist > 0.0001f) {
        normal = vec2_scale(delta, 1.0f / dist);
    } else {
        float angle = (float)((a->id * 37u + b->id * 17u) % 360u) * 0.017453292f;

        normal = (vec2){cosf(angle), sinf(angle)};
        dist = 0.0f;
    }

    float overlap = radii - dist;

    /* 질량은 넓이에 비례한다고 가정 (자는 과일은 고정된 것으로 취급) */
    float inv_a = a->sleeping ? 0.0f : 1.0f / (a->r * a->r);
    float inv_b = b->sleeping ? 0.0f : 1.0f / (b->r * b->r);
    float inv_sum = inv_a + inv_b;

    if (inv_sum <= 0.0f) {
        return;
    }

    /* 위치 보정: slop을 남기고 일부만 밀어내어 진동을 억제 */
    if (overlap > POSITION_SLOP) {
        float correction = (overlap - POSITION_SLOP) * CORRECTION_PERCENT;

        a->pos = vec2_sub(a->pos, vec2_scale(normal, correction * (inv_a / inv_sum)));
        b->pos = vec2_add(b->pos, vec2_scale(normal, correction * (inv_b / inv_sum)));
    }

    vec2 rel = vec2_sub(b->vel, a->vel);
    float along = vec2_dot(rel, normal);

    /* 이미 떨어지는 중이면 반발 없음 */
    if (along > 0.0f) {
        return;
    }

    float e = -along > RESTITUTION_SPEED_THRESHOLD ? RESTITUTION : 0.0f;
    float jn = -(1.0f + e) * along / inv_sum;

    /* 접선 마찰 (쿨롱 한계) */
    vec2 tangent = vec2_sub(rel, vec2_scale(normal, along));
    float tangent_len = sqrtf(vec2_length_sq(tangent));

    if (tangent_len > 0.0001f) {
        vec2 tangent_dir = vec2_scale(tangent, 1.0f / tangent_len);
        float jt = -vec2_dot(rel, tangent_dir) / inv_sum;
        float limit = PAIR_FRICTION * fabsf(jn);

        if (jt > limit) {
            jt = limit;
        } else if (jt < -limit) {
            jt = -limit;
        }

        vec2 friction = vec2_scale(tangent_dir, jt);
        a->vel = vec2_sub(a->vel, vec2_scale(friction, inv_a));
        b->vel = vec2_add(b->vel, vec2_scale(friction, inv_b));
    }

    vec2 impulse = vec2_scale(normal, jn);
    a->vel = vec2_sub(a->vel, vec2_scale(impulse, inv_a));
    b->vel = vec2_add(b->vel, vec2_scale(impulse, inv_b));
}

void collider_resolve_wall(Fruit *f, rect bounds) {
    if (f->pos.x - f->r < bounds.left) {
        f->pos.x = bounds.left + f->r;
        if (f->vel.x < 0.0f) {
            f->vel.x = -f->vel.x > RESTITUTION_SPEED_THRESHOLD ? -f->vel.x * RESTITUTION : 0.0f;
        }
    } else if (f->pos.x + f->r > bounds.right) {
        f->pos.x = bounds.right - f->r;
        if (f->vel.x > 0.0f) {
            f->vel.x = f->vel.x > RESTITUTION_SPEED_THRESHOLD ? -f->vel.x * RESTITUTION : 0.0f;
        }
    }

    if (f->pos.y - f->r < bounds.top) {
        f->pos.y = bounds.top + f->r;
        if (f->vel.y < 0.0f) {
            f->vel.y = -f->vel.y > RESTITUTION_SPEED_THRESHOLD ? -f->vel.y * RESTITUTION : 0.0f;
        }
    } else if (f->pos.y + f->r > bounds.bottom) {
        f->pos.y = bounds.bottom - f->r;
        if (f->vel.y > 0.0f) {
            f->vel.y = f->vel.y > RESTITUTION_SPEED_THRESHOLD ? -f->vel.y * RESTITUTION : 0.0f;
        }
        /* 바닥에 닿아 있는 동안에는 옆으로 미끄러지는 힘을 줄인다 */
        f->vel.x *= FLOOR_FRICTION;
    }
}

static void collider_solve_contacts(PlayerState *state, rect bounds) {
    for (int iteration = 0; iteration < SOLVER_ITERATIONS; ++iteration) {
        for (size_t i = 0; i < state->fruitsCount; ++i) {
            collider_resolve_wall(&state->fruits[i], bounds);
        }

        /* 반복마다 순회 방향을 바꿔 인덱스 순서에 따른 편향을 줄인다 */
        bool forward = (iteration % 2) == 0;

        for (size_t k = 0; k < state->fruitsCount; ++k) {
            size_t i = forward ? k : state->fruitsCount - 1 - k;

            for (size_t j = i + 1; j < state->fruitsCount; ++j) {
                Fruit *a = &state->fruits[i];
                Fruit *b = &state->fruits[j];

                if (collider_overlap(a, b)) {
                    collider_resolve_pair(a, b);
                }
            }
        }
    }

    /* 쌍 보정으로 밀려난 과일이 경계를 벗어나지 않도록 마지막에 한 번 더 정리 */
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        collider_resolve_wall(&state->fruits[i], bounds);
    }
}

static void collider_integrate(PlayerState *state, float dt) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];

        if (fruit->sleeping) {
            continue;
        }

        fruit->vel.y += GRAVITY * dt;
        fruit->pos = vec2_add(fruit->pos, vec2_scale(fruit->vel, dt));
    }
}

/* 다른 과일과 크게 겹쳐 있으면 아직 자리를 못 잡은 상태이므로 재우지 않는다 */
static bool collider_is_crowded(PlayerState *state, size_t index) {
    const float tolerance = 2.0f;
    Fruit *fruit = &state->fruits[index];

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        if (i == index) {
            continue;
        }

        Fruit *other = &state->fruits[i];
        float dx = other->pos.x - fruit->pos.x;
        float dy = other->pos.y - fruit->pos.y;
        float limit = (fruit->r + other->r) - tolerance;

        if (limit > 0.0f && (dx * dx) + (dy * dy) < limit * limit) {
            return true;
        }
    }

    return false;
}

/* 가장 빠른 과일이 자기 반지름의 절반보다 많이 움직이면 놓칠 수 있으므로 단계를 쪼갠다 */
static int collider_substep_count(PlayerState *state, float dt) {
    float max_speed = 0.0f;
    float min_radius = 0.0f;

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];
        float speed = sqrtf(vec2_length_sq(fruit->vel));

        if (speed > max_speed) {
            max_speed = speed;
        }
        if (min_radius == 0.0f || fruit->r < min_radius) {
            min_radius = fruit->r;
        }
    }

    if (max_speed <= 0.0f || min_radius <= 0.0f) {
        return 1;
    }

    int count = (int)ceilf(max_speed * dt / (min_radius * 0.5f));

    if (count < 1) {
        return 1;
    }
    if (count > MAX_SUBSTEPS) {
        return MAX_SUBSTEPS;
    }
    return count;
}

void collider_step(PlayerState *state, rect bounds, float dt) {
    int substeps = collider_substep_count(state, dt);
    float sub_dt = dt / (float)substeps;

    for (int step = 0; step < substeps; ++step) {
        collider_integrate(state, sub_dt);
        collider_solve_contacts(state, bounds);
    }

    /* 충분히 오래 느리면 정지 상태로 전환한다 */
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];

        if (fruit->sleeping) {
            continue;
        }

        if (vec2_length_sq(fruit->vel) < SLEEP_SPEED * SLEEP_SPEED && !collider_is_crowded(state, i)) {
            if (++fruit->sleepFrames >= SLEEP_FRAMES) {
                fruit->sleeping = true;
                fruit->vel = vec2_zero();
            }
        } else {
            fruit->sleepFrames = 0;
        }
    }
}
