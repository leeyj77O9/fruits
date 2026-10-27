#include "game/frame_clock.h"
#include "game/fruit.h"
#include "game/state.h"
#include "game/renderer.h"
#include "game/collider.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

void draw(SDL_Renderer *renderer, PlayerState *state);
int  close(SDL_Renderer *renderer, SDL_Window *window);
void handle_event(SDL_Event *event, bool *running);

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
    FrameClock clock;

    state_init(&state);
    frame_clock_init(&clock);

    state_add_fruit(&state, (vec2){400, 100}, FRUIT_TYPE_APPLE);

    while (running) {
        handle_event(&event, &running);
        while (frame_clock_fixed_step(&clock, 1.0 / 120.0)) {
            collider_step(&state, bounds, 1.0 / 120.0);
        }
        draw(renderer, &state);
    }
    
    return close(renderer, window);
}

void draw(SDL_Renderer *renderer, PlayerState *state) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    render(renderer, state);

    SDL_RenderPresent(renderer);
}
int close(SDL_Renderer *renderer, SDL_Window *window) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
void handle_event(SDL_Event *event, bool *running) {
    while (SDL_PollEvent(event)) {
            if (event->type == SDL_EVENT_QUIT) {
                running = false;
            } 
        } 
}