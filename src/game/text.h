#pragma once

#include <stdbool.h>
#include <SDL3/SDL.h>

#include "util/rgba.h"
#include "util/type.h"

bool text_init(void);
void text_quit(void);

void text_draw(SDL_Renderer *renderer, const char *text, f32 x, f32 y);
void text_draw_sized(SDL_Renderer *renderer, const char *text, f32 x, f32 y, f32 size);
void text_draw_centered(SDL_Renderer *renderer, const char *text, f32 centerX, f32 y, f32 size);

void text_draw_colored(SDL_Renderer *renderer, const char *text, f32 x, f32 y, f32 size, rgba color);
void text_draw_centered_colored(SDL_Renderer *renderer, const char *text, f32 centerX, f32 y, f32 size, rgba color);
void text_draw_center_middle_colored(SDL_Renderer *renderer, const char *text, f32 centerX, f32 centerY, f32 size, rgba color);
