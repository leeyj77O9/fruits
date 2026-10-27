#include "game/collider.h"
#include <math.h>

static const f32 GRAVITY = 2000.0f;
static const f32 RESTITUTION = 0.15f;
static const f32 FLOOR_FRICTION = 0.98f;
static const f32 PAIR_FRICTION = 0.22f;
static const f32 POSITION_SLOP = 0.5f;
static const f32 RESTITUTION_SPEED_THRESHOLD = 100.0f;
static const f32 CORRECTION_PERCENT = 0.6f;
static const f32 SLEEP_SPEED = 25.0f;
static const u16 SLEEP_FRAMES = 35;
static const f32 WAKE_SPEED = 50.0f;
static const f32 PERCH_NUDGE_SPEED = 90.0f;

static const int SOLVER_ITERATIONS = 8;
static const int MAX_SUBSTEPS = 8;

static bool collider_overlap(const Fruit *a, const Fruit *b) {
    f32 dx = b->pos.x - a->pos.x;
    f32 dy = b->pos.y - a->pos.y;
    f32 radii = a->r + b->r;

    return (dx * dx) + (dy * dy) <= radii * radii;
}

static void collider_resolve_pair(Fruit *a, Fruit *b) {
    vec2 delta = vec2_sub(b->pos, a->pos);
    f32 dist_sq = vec2_length_sq(delta);
    f32 radii = a->r + b->r;

    if (dist_sq >= radii * radii) {
        return;
    }

    if (a->sleeping && b->sleeping) {
        return;
    }

    if (a->sleeping && vec2_length_sq(b->vel) > WAKE_SPEED * WAKE_SPEED) {
        a->sleeping = false;
        a->sleepFrames = 0;
    }
    if (b->sleeping && vec2_length_sq(a->vel) > WAKE_SPEED * WAKE_SPEED) {
        b->sleeping = false;
        b->sleepFrames = 0;
    }

    f32 dist = sqrtf(dist_sq);
    vec2 normal;

    if (dist > 0.0001f) {
        normal = vec2_scale(delta, 1.0f / dist);
    } else {
        f32 angle = (f32)((a->id * 37u + b->id * 17u) % 360u) * 0.017453292f;

        normal = (vec2){cosf(angle), sinf(angle)};
        dist = 0.0f;
    }

    f32 overlap = radii - dist;

    f32 inv_a = a->sleeping ? 0.0f : 1.0f / (a->r * a->r);
    f32 inv_b = b->sleeping ? 0.0f : 1.0f / (b->r * b->r);
    f32 inv_sum = inv_a + inv_b;

    if (inv_sum <= 0.0f) {
        return;
    }

    if (overlap > POSITION_SLOP) {
        f32 correction = (overlap - POSITION_SLOP) * CORRECTION_PERCENT;

        a->pos = vec2_sub(a->pos, vec2_scale(normal, correction * (inv_a / inv_sum)));
        b->pos = vec2_add(b->pos, vec2_scale(normal, correction * (inv_b / inv_sum)));
    }

    vec2 rel = vec2_sub(b->vel, a->vel);
    f32 along = vec2_dot(rel, normal);

    if (along > 0.0f) {
        return;
    }

    f32 e = -along > RESTITUTION_SPEED_THRESHOLD ? RESTITUTION : 0.0f;
    f32 jn = -(1.0f + e) * along / inv_sum;

    vec2 tangent = vec2_sub(rel, vec2_scale(normal, along));
    f32 tangent_len = vec2_length(tangent);

    if (tangent_len > 0.0001f) {
        vec2 tangent_dir = vec2_scale(tangent, 1.0f / tangent_len);
        f32 jt = -vec2_dot(rel, tangent_dir) / inv_sum;
        f32 limit = PAIR_FRICTION * fabsf(jn);

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

static void collider_resolve_wall(Fruit *f, rect bounds) {
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

        f->vel.x *= FLOOR_FRICTION;
    }
}

static void collider_solve_contacts(PlayerState *state, rect bounds) {
    for (int iteration = 0; iteration < SOLVER_ITERATIONS; ++iteration) {
        for (size_t i = 0; i < state->fruitsCount; ++i) {
            collider_resolve_wall(&state->fruits[i], bounds);
        }

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

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        collider_resolve_wall(&state->fruits[i], bounds);
    }
}

static void collider_integrate(PlayerState *state, f32 dt) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];

        if (fruit->sleeping) {
            continue;
        }

        fruit->vel.y += GRAVITY * dt;
        fruit->pos = vec2_add(fruit->pos, vec2_scale(fruit->vel, dt));
    }
}

static bool collider_is_crowded(const PlayerState *state, size_t index) {
    const f32 tolerance = 4.0f;
    const Fruit *fruit = &state->fruits[index];

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        if (i == index) {
            continue;
        }

        const Fruit *other = &state->fruits[i];
        f32 dx = other->pos.x - fruit->pos.x;
        f32 dy = other->pos.y - fruit->pos.y;
        f32 limit = (fruit->r + other->r) - tolerance;

        if (limit > 0.0f && (dx * dx) + (dy * dy) < limit * limit) {
            return true;
        }
    }

    return false;
}

static bool collider_is_perched(const PlayerState *state, size_t index, rect bounds) {
    const Fruit *fruit = &state->fruits[index];

    if (fruit->pos.y + fruit->r >= bounds.bottom - 1.0f) {
        return false;
    }
    if (fruit->pos.x - fruit->r <= bounds.left + 0.5f || fruit->pos.x + fruit->r >= bounds.right - 0.5f) {
        return false;
    }

    int supports = 0;
    f32 normal_x = 0.0f;

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        if (i == index) {
            continue;
        }

        const Fruit *other = &state->fruits[i];

        if (other->pos.y <= fruit->pos.y) {
            continue;
        }

        f32 dx = other->pos.x - fruit->pos.x;
        f32 dy = other->pos.y - fruit->pos.y;
        f32 dist = sqrtf((dx * dx) + (dy * dy));

        if (dist > fruit->r + other->r + 2.0f) {
            continue;
        }

        supports++;
        normal_x = dist > 0.0001f ? dx / dist : 0.0f;
    }

    if (supports != 1) {
        return false;
    }

    return fabsf(normal_x) < 0.30f;
}

static int collider_substep_count(const PlayerState *state, f32 dt) {
    f32 max_speed = 0.0f;
    f32 min_radius = 0.0f;

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        const Fruit *fruit = &state->fruits[i];
        f32 speed = vec2_length(fruit->vel);

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

static void collider_update_sleep(PlayerState *state, rect bounds) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];

        if (fruit->sleeping) {
            continue;
        }

        bool slow = vec2_length_sq(fruit->vel) < SLEEP_SPEED * SLEEP_SPEED;

        if (!slow || collider_is_crowded(state, i)) {
            fruit->sleepFrames = 0;
            continue;
        }

        if (collider_is_perched(state, i, bounds)) {
            f32 direction = (fruit->id & 1u) != 0u ? 1.0f : -1.0f;

            fruit->vel.x += direction * PERCH_NUDGE_SPEED;
            fruit->sleepFrames = 0;
        } else if (++fruit->sleepFrames >= SLEEP_FRAMES) {
            fruit->sleeping = true;
            fruit->vel = vec2_zero();
        }
    }
}

void collider_step(PlayerState *state, rect bounds, f32 dt) {
    int substeps = collider_substep_count(state, dt);
    f32 sub_dt = dt / (f32)substeps;

    for (int step = 0; step < substeps; ++step) {
        collider_integrate(state, sub_dt);
        collider_solve_contacts(state, bounds);
    }

    collider_update_sleep(state, bounds);
}
