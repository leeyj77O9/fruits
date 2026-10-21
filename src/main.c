#include <SDL3/SDL.h>
#include <stdbool.h>
#include <math.h>

typedef struct {
    float x;
    float y;
} vector2_t;

typedef struct {
    vector2_t position;
    float_t radius;
} fruit_t;

void draw_fruit(SDL_Renderer *renderer, fruit_t *fruit) {
    int POINTS = 20000;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_FPoint points[POINTS];
    int count = 0;

    for (float_t dx = -fruit->radius; dx < fruit->radius; dx += 1.0f) {
        for (float_t dy = -fruit->radius; dy < fruit->radius; dy += 1.0f) {
            if ((dx*dx + dy*dy) <= (fruit->radius * fruit->radius)) {
                points[count].x = fruit->position.x + dx;
                points[count].y = fruit->position.y + dy;
                count++;
            }
            if (count >= POINTS) {
                break;
            }
        }
    }
    SDL_RenderPoints(renderer, points, count);
}

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

    fruit_t fruit = {
        .position = {400, 100},
        .radius = 10
    };
    bool is_dropping = false;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                is_dropping = true;
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (!is_dropping) {
                    fruit.position.x = event.motion.x;
                }
                
                if (fruit.position.x + fruit.radius > 800) {
                    fruit.position.x = 800 - fruit.radius;
                } else if (fruit.position.x - fruit.radius < 0) {
                    fruit.position.x = fruit.radius;
                }
            }
        }

        if (is_dropping) {
            fruit.position.y += 0.3; 
            if (fruit.position.y + fruit.radius > 600) {
                fruit.position.y = 600 - fruit.radius;
                is_dropping = false; 
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_fruit(renderer, &fruit);

        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}