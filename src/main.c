#include "game/fruit.h"
#include "game/state.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

void render(SDL_Renderer *renderer, PlayerState *state);
void render_score(SDL_Renderer *renderer, u64 score);
void render_fruit(SDL_Renderer *rebderer, Fruit* fruit);

int main(int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("Fruits", 800, 600, 0, &window, &renderer)) {
        SDL_Log("Unable to create window and renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;
    PlayerState state;
    state_init(&state);

    state_add_fruit(&state, (vec2){100, 200}, FRUIT_TYPE_APPLE);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } 
        } 

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render(renderer, &state);

        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void render(SDL_Renderer *rederer, PlayerState* state) {
    render_score(rederer, state->score);
    
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        render_fruit(rederer, &state->fruits[i]);
        printf("%d: fruit rendered", state->fruits[i].id);
    }
}

void render_score(SDL_Renderer *renderer, u64 score) {

}

void render_fruit(SDL_Renderer *renderer, Fruit *fruit) {
    float r = fruit->r;
    SDL_SetRenderDrawColor(renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
    for (float dx = -r; dx < r; dx += 1.0f) {
        for (float dy = -r; dy < r; dy += 1.0f) {
            if ((dx * dx) + (dy * dy) <= r * r) {
                SDL_RenderPoint(renderer, fruit->pos.x + dx, fruit->pos.y + dy);
            }
        }
    }
}