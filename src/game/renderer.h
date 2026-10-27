#pragma once

#include <SDL3/SDL.h>
#include "state.h"

void render(SDL_Renderer *renderer, PlayerState *state);
void render_score(SDL_Renderer *renderer, u64 score);
void render_fruit(SDL_Renderer *rebderer, Fruit* fruit);