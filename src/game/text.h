#pragma once

#include <stdbool.h>
#include <SDL3/SDL.h>

#include "util/type.h"

bool text_init(void);
void text_quit(void);

void text_draw(SDL_Renderer *renderer, const char *text, f32 x, f32 y);
