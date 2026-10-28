#pragma once

#include <stdbool.h>
#include <SDL3/SDL.h>

#include "game/state.h"

bool renderer_init(SDL_Renderer *renderer);
void renderer_quit(void);

void renderer_draw(SDL_Renderer *renderer, const PlayerState *state, const Fruit *held);
void renderer_draw_guide(SDL_Renderer *renderer, bool danger);
void renderer_draw_game_over(SDL_Renderer *renderer, u64 score);
