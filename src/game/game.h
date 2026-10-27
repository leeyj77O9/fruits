#pragma once

#include <stdbool.h>
#include <SDL3/SDL.h>

#include "game/frame_clock.h"
#include "game/fruit.h"
#include "game/state.h"
#include "util/rect.h"

enum {
    GAME_WIDTH = 800,
    GAME_HEIGHT = 600,
};

#define GAME_DROP_LINE_Y 100.0f
#define GAME_PHYSICS_STEP (1.0 / 120.0)
#define GAME_RENDER_FPS 60.0

typedef struct Game {
    PlayerState state;
    FrameClock  clock;
    Fruit       held;
    rect        bounds;
    bool        running;
} Game;

bool game_init(Game *game);
void game_shutdown(Game *game);

void game_handle_events(Game *game);
void game_update(Game *game);
void game_draw(const Game *game, SDL_Renderer *renderer);
