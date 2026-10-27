#include "renderer.h"

const float FRUIT_PIXELS_PER_UNIT = 10.0f;

void render(SDL_Renderer *rederer, PlayerState* state) {
    render_score(rederer, state->score);
    
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        render_fruit(rederer, &state->fruits[i]);
    }
}

void render_score(SDL_Renderer *renderer, u64 score) {

}

void render_fruit(SDL_Renderer *renderer, Fruit *fruit) {
    float r = fruit->r * FRUIT_PIXELS_PER_UNIT;
    rgba color = FRUITS_INFOS[fruit->type].color;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (float dx = -r; dx < r; dx += 1.0f) {
        for (float dy = -r; dy < r; dy += 1.0f) {
            if ((dx * dx) + (dy * dy) <= r * r) {
                SDL_RenderPoint(renderer, fruit->pos.x + dx, fruit->pos.y + dy);
            }
        }
    }
}