#pragma once

#include <stdbool.h>
#include <SDL3/SDL.h>

#include "game/state.h"

bool renderer_init(void);
void renderer_quit(void);

void renderer_draw(SDL_Renderer *renderer, const PlayerState *state, const Fruit *held);
