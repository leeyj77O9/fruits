#include "game/frame_clock.h"
#include "game/fruit.h"
#include "game/state.h"
#include "game/renderer.h"
#include "game/collider.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define DROP_LINE_Y 100.0f

void draw(SDL_Renderer *renderer, PlayerState *state, Fruit *held);
int  close(SDL_Renderer *renderer, SDL_Window *window);
void handle_event(SDL_Event *event, bool *running, Fruit *held, PlayerState *state, rect bounds);
void clamp_to_bounds(Fruit *fruit, rect bounds);

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

    SDL_SetRenderVSync(renderer, 1);

    bool running = true;
    SDL_Event event;
    PlayerState state;
    FrameClock clock;
    rect bounds = {0, 0, 800, 600};

    state_init(&state);
    frame_clock_init(&clock);
    srand((unsigned int)SDL_GetTicks());

    /* 아직 떨어뜨리지 않은 과일. 마우스를 따라다니며 물리 연산에는 참여하지 않는다 */
    Fruit held = fruit_make((vec2){400, DROP_LINE_Y}, fruit_get_random_fruitType());

    while (running) {
        handle_event(&event, &running, &held, &state, bounds);
        frame_clock_tick(&clock);
        while (frame_clock_fixed_step(&clock, 1.0 / 120.0)) {
            collider_step(&state, bounds, 1.0 / 120.0);
        }
        draw(renderer, &state, &held);
    }
    
    return close(renderer, window);
}

void draw(SDL_Renderer *renderer, PlayerState *state, Fruit *held) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    render(renderer, state);
    render_fruit(renderer, held);

    SDL_RenderPresent(renderer);
}
int close(SDL_Renderer *renderer, SDL_Window *window) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
void handle_event(SDL_Event *event, bool *running, Fruit *held, PlayerState *state, rect bounds) {
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_EVENT_QUIT:
                *running = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                held->pos.x = event->motion.x;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    state_add_fruit(state, held->pos, held->type);
                    *held = fruit_make((vec2){held->pos.x, DROP_LINE_Y}, fruit_get_random_fruitType());
                }
                break;
            default:
                break;
        }
    }

    clamp_to_bounds(held, bounds);
}
void clamp_to_bounds(Fruit *fruit, rect bounds) {
    float min_x = bounds.left + fruit->r;
    float max_x = bounds.right - fruit->r;

    if (fruit->pos.x < min_x) {
        fruit->pos.x = min_x;
    } else if (fruit->pos.x > max_x) {
        fruit->pos.x = max_x;
    }
}
