#include "game/game.h"

#include <stdlib.h>

#include "game/collider.h"
#include "game/renderer.h"

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

bool game_init(Game *game) {
    if (game == NULL) {
        return false;
    }

    game->bounds = (rect){0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT};
    game->running = true;

    frame_clock_init(&game->clock);

    if (!state_init(&game->state)) {
        return false;
    }

    srand((unsigned int)SDL_GetTicks());

    game->held = fruit_make((vec2){GAME_WIDTH * 0.5f, GAME_DROP_LINE_Y}, fruit_get_random_type());

    return true;
}

void game_shutdown(Game *game) {
    state_free(&game->state);
}

void game_handle_events(Game *game) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                game->running = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                game->held.pos.x = event.motion.x;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    game_drop(game);
                }
                break;
            default:
                break;
        }
    }

    game_clamp_held(game);
}

void game_update(Game *game) {
    frame_clock_tick(&game->clock);

    while (frame_clock_fixed_step(&game->clock, GAME_PHYSICS_STEP)) {
        collider_step(&game->state, game->bounds, (f32)GAME_PHYSICS_STEP);
        state_merge_touching(&game->state);
    }
}

void game_draw(const Game *game, SDL_Renderer *renderer) {
    renderer_draw(renderer, &game->state, &game->held);
    SDL_RenderPresent(renderer);
}
