#include "game/game.h"

#include <math.h>
#include <stdlib.h>

#include "game/collider.h"
#include "game/renderer.h"

static const f32 DROP_COOLDOWN = 0.8f;
static const f32 FLOOR_MARGIN = 6.0f;
static const f32 HELD_MOVE_SPEED = 600.0f;
static const f32 HELD_MOVE_MAX_DT = 0.05f;
static const f32 TWO_PI = 6.283185307179586f;
static const f32 GAME_OVER_HOLD_TIME = 1.2f;
static const f32 GAME_OVER_SPEED = 400.0f;
static const f32 GAME_OVER_WARN_TIME = 0.35f;
static const f32 WINDOW_INERTIA_X = 1.2f;
static const f32 WINDOW_INERTIA_Y = 0.6f;
static const f32 WINDOW_VELOCITY_SMOOTHING = 0.5f;
static const f32 WINDOW_MAX_SPEED = 2500.0f;
static const f32 WINDOW_MAX_IMPULSE = 600.0f;
static const f32 WINDOW_WAKE_SPEED = 12.0f;
static const f32 WINDOW_TELEPORT_DISTANCE = 300.0f;
static const f32 WINDOW_MIN_DT = 0.001f;
static const f32 WINDOW_MAX_DT = 0.05f;

static f32 game_clamp_speed(f32 value, f32 limit) {
    if (value > limit) {
        return limit;
    }
    if (value < -limit) {
        return -limit;
    }
    return value;
}

static void game_clamp_held(Game *game) {
    f32 min_x = game->bounds.left + game->held.r;
    f32 max_x = game->bounds.right - game->held.r;

    if (game->held.pos.x < min_x) {
        game->held.pos.x = min_x;
    } else if (game->held.pos.x > max_x) {
        game->held.pos.x = max_x;
    }

    game->held.pos.y = GAME_DROP_LINE_Y;
}

static void game_drop(Game *game) {
    state_add_fruit(&game->state, game->held.pos, game->held.type);

    game->held = fruit_make((vec2){game->held.pos.x, GAME_DROP_LINE_Y}, fruit_get_random_type());
    game_clamp_held(game);
}

static void game_try_drop(Game *game) {
    if (game->dropCooldown > 0.0f) {
        return;
    }

    game_drop(game);
    game->dropCooldown = DROP_COOLDOWN;
}

static void game_move_held(Game *game, f32 dx) {
    game->held.pos.x += dx;
    game_clamp_held(game);
}

static void game_move_held_by_input(Game *game, f32 dt) {
    f32 direction = (f32)((int)game->moveRight - (int)game->moveLeft);

    if (direction == 0.0f) {
        return;
    }

    if (dt > HELD_MOVE_MAX_DT) {
        dt = HELD_MOVE_MAX_DT;
    }

    game_move_held(game, direction * HELD_MOVE_SPEED * dt);
}

static void game_apply_impulse(Game *game, f32 dvx, f32 dvy) {
    f32 impulse = fabsf(dvx) + fabsf(dvy);

    for (size_t i = 0; i < game->state.fruitsCount; ++i) {
        Fruit *fruit = &game->state.fruits[i];

        if (fruit->sleeping) {
            if (impulse < WINDOW_WAKE_SPEED) {
                continue;
            }

            fruit->sleeping = false;
            fruit->sleepFrames = 0;
        }

        fruit->vel.x += dvx;
        fruit->vel.y += dvy;
    }
}

static void game_apply_window_motion(Game *game, f64 frameDelta) {
    int x = 0;
    int y = 0;

    if (!SDL_GetWindowPosition(game->window, &x, &y)) {
        return;
    }

    WindowMotion *motion = &game->windowMotion;

    if (!motion->ready) {
        motion->x = x;
        motion->y = y;
        motion->velX = 0.0f;
        motion->velY = 0.0f;
        motion->ready = true;
        return;
    }

    int stepX = x - motion->x;
    int stepY = y - motion->y;

    motion->x = x;
    motion->y = y;

    if (abs(stepX) > (int)WINDOW_TELEPORT_DISTANCE || abs(stepY) > (int)WINDOW_TELEPORT_DISTANCE) {
        motion->velX = 0.0f;
        motion->velY = 0.0f;
        return;
    }

    f32 dt = (f32)frameDelta;

    if (dt < WINDOW_MIN_DT) {
        dt = WINDOW_MIN_DT;
    }
    if (dt > WINDOW_MAX_DT) {
        dt = WINDOW_MAX_DT;
    }

    f32 instantX = game_clamp_speed((f32)stepX / dt, WINDOW_MAX_SPEED);
    f32 instantY = game_clamp_speed((f32)stepY / dt, WINDOW_MAX_SPEED);

    f32 previousX = motion->velX;
    f32 previousY = motion->velY;

    motion->velX += (instantX - motion->velX) * WINDOW_VELOCITY_SMOOTHING;
    motion->velY += (instantY - motion->velY) * WINDOW_VELOCITY_SMOOTHING;

    f32 accelX = (motion->velX - previousX) / dt;
    f32 accelY = (motion->velY - previousY) / dt;

    f32 dvx = -accelX * dt * WINDOW_INERTIA_X;
    f32 dvy = -accelY * dt * WINDOW_INERTIA_Y;
    f32 impulse = fabsf(dvx) + fabsf(dvy);

    if (impulse > WINDOW_MAX_IMPULSE) {
        f32 scale = WINDOW_MAX_IMPULSE / impulse;
        dvx *= scale;
        dvy *= scale;
    }

    game_apply_impulse(game, dvx, dvy);
}

bool game_init(Game *game, SDL_Window *window) {
    if (game == NULL || window == NULL) {
        return false;
    }

    game->bounds = (rect){0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT - FLOOR_MARGIN};
    game->window = window;

    game->windowMotion.x = 0;
    game->windowMotion.y = 0;
    game->windowMotion.velX = 0.0f;
    game->windowMotion.velY = 0.0f;
    game->windowMotion.ready = false;

    frame_clock_init(&game->clock);

    if (!state_init(&game->state)) {
        return false;
    }

    srand((unsigned int)SDL_GetTicks());

    game->held = fruit_make((vec2){GAME_WIDTH * 0.5f, GAME_DROP_LINE_Y}, fruit_get_random_type());
    game->dropCooldown = 0.0f;
    game->overTime = 0.0f;
    game->gameOver = false;
    game->moveLeft = false;
    game->moveRight = false;

    int x = 0;
    int y = 0;

    if (SDL_GetWindowPosition(window, &x, &y)) {
        game->windowMotion.x = x;
        game->windowMotion.y = y;
        game->windowMotion.ready = true;
    }

    return true;
}

static void game_restart(Game *game) {
    game->state.fruitsCount = 0;
    game->state.score = 0;
    game->held = fruit_make((vec2){GAME_WIDTH * 0.5f, GAME_DROP_LINE_Y}, fruit_get_random_type());
    game->dropCooldown = 0.0f;
    game->overTime = 0.0f;
    game->gameOver = false;
    game->moveLeft = false;
    game->moveRight = false;

    game_clamp_held(game);
    frame_clock_init(&game->clock);
}

void game_shutdown(Game *game) {
    state_free(&game->state);
}

void game_handle_event(Game *game, const SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case SDLK_SPACE:
                    if (!event->key.repeat) {
                        game_try_drop(game);
                    }
                    break;
                case SDLK_LEFT:
                    game->moveLeft = true;
                    break;
                case SDLK_RIGHT:
                    game->moveRight = true;
                    break;
                case SDLK_R:
                    if (!event->key.repeat) {
                        game_restart(game);
                    }
                    break;
                default:
                    break;
            }
            break;
        case SDL_EVENT_KEY_UP:
            switch (event->key.key) {
                case SDLK_LEFT:
                    game->moveLeft = false;
                    break;
                case SDLK_RIGHT:
                    game->moveRight = false;
                    break;
                default:
                    break;
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            game->moveLeft = false;
            game->moveRight = false;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            game->held.pos.x = event->motion.x;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                game_try_drop(game);
            }
            break;
        default:
            break;
    }
}

static void game_update_rotation(Game *game, f32 dt) {
    if (dt <= 0.0f) {
        return;
    }

    for (size_t i = 0; i < game->state.fruitsCount; ++i) {
        Fruit *fruit = &game->state.fruits[i];
        f32 dx = fruit->pos.x - fruit->lastX;

        if (fabsf(dx) > fruit->r) {
            dx = 0.0f;
        }

        fruit->angle += dx / fruit->r;
        fruit->lastX = fruit->pos.x;

        if (fruit->angle > TWO_PI || fruit->angle < -TWO_PI) {
            fruit->angle = fmodf(fruit->angle, TWO_PI);
        }
    }
}

static void game_check_over(Game *game, f32 dt) {
    bool danger = false;

    for (size_t i = 0; i < game->state.fruitsCount; ++i) {
        const Fruit *fruit = &game->state.fruits[i];

        if (fruit->pos.y - fruit->r >= GAME_OVER_LINE_Y) {
            continue;
        }

        if (fabsf(fruit->vel.x) + fabsf(fruit->vel.y) >= GAME_OVER_SPEED) {
            continue;
        }

        danger = true;
        break;
    }

    if (!danger) {
        game->overTime = 0.0f;
        return;
    }

    game->overTime += dt;

    if (game->overTime >= GAME_OVER_HOLD_TIME) {
        game->gameOver = true;
    }
}

void game_update(Game *game) {
    f64 dt = frame_clock_tick(&game->clock);

    if (game->gameOver) {
        return;
    }

    if (game->dropCooldown > 0.0f) {
        game->dropCooldown -= (f32)dt;

        if (game->dropCooldown < 0.0f) {
            game->dropCooldown = 0.0f;
        }
    }

    game_clamp_held(game);
    game_move_held_by_input(game, (f32)dt);
    game_apply_window_motion(game, dt);

    while (frame_clock_fixed_step(&game->clock, GAME_PHYSICS_STEP)) {
        collider_step(&game->state, game->bounds, (f32)GAME_PHYSICS_STEP);
        state_merge_touching(&game->state);
        game_update_rotation(game, (f32)GAME_PHYSICS_STEP);
        game_check_over(game, (f32)GAME_PHYSICS_STEP);
    }
}

void game_draw(const Game *game, SDL_Renderer *renderer) {
    const Fruit *held = game->dropCooldown > 0.0f ? NULL : &game->held;

    renderer_draw(renderer, &game->state, NULL);
    renderer_draw_guide(renderer, game->overTime >= GAME_OVER_WARN_TIME);

    if (!game->gameOver) {
        renderer_draw_held(renderer, held);
    }

    if (game->gameOver) {
        renderer_draw_game_over(renderer, game->state.score);
    }

    SDL_RenderPresent(renderer);
}
