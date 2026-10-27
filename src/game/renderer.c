#include "game/renderer.h"
#include "game/text.h"
#include <stddef.h>
#include <stdio.h>

static const f32 SCORE_MARGIN_X = 20.0f;
static const f32 SCORE_MARGIN_Y = 14.0f;

bool renderer_init(void) {
    return text_init();
}

void renderer_quit(void) {
    text_quit();
}

static void renderer_draw_score(SDL_Renderer *renderer, u64 score) {
    char text[32];

    snprintf(text, sizeof(text), "%llu", (unsigned long long)score);
    text_draw(renderer, text, SCORE_MARGIN_X, SCORE_MARGIN_Y);
}

static void renderer_draw_fruit(SDL_Renderer *renderer, const Fruit *fruit) {
    f32 r = fruit->r;
    rgba color = FRUITS_INFOS[fruit->type].color;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (f32 dx = -r; dx < r; dx += 1.0f) {
        for (f32 dy = -r; dy < r; dy += 1.0f) {
            if ((dx * dx) + (dy * dy) <= r * r) {
                SDL_RenderPoint(renderer, fruit->pos.x + dx, fruit->pos.y + dy);
            }
        }
    }
}

void renderer_draw(SDL_Renderer *renderer, const PlayerState *state, const Fruit *held) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        renderer_draw_fruit(renderer, &state->fruits[i]);
    }

    if (held != NULL) {
        renderer_draw_fruit(renderer, held);
    }

    renderer_draw_score(renderer, state->score);
}
