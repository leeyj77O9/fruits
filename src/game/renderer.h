#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "state.h"

/* 폰트 등 렌더링에 필요한 자원을 준비한다. 실패해도 게임은 돌아간다 */
bool renderer_init(void);
void renderer_quit(void);

void render(SDL_Renderer *renderer, PlayerState *state);
void render_score(SDL_Renderer *renderer, u64 score);
void render_fruit(SDL_Renderer *renderer, Fruit* fruit);
