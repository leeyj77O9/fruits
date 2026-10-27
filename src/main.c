#include <SDL3/SDL.h>

#include "game/frame_clock.h"
#include "game/game.h"
#include "game/renderer.h"

static void app_quit(SDL_Renderer *renderer, SDL_Window *window) {
    renderer_quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL 초기화 실패: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("Fruits", GAME_WIDTH, GAME_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("창/렌더러 생성 실패: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    if (!renderer_init()) {
        SDL_Log("렌더링 초기화에 실패했습니다: %s", SDL_GetError());
    }

    Game game;

    if (!game_init(&game)) {
        SDL_Log("게임 초기화에 실패했습니다: %s", SDL_GetError());
        app_quit(renderer, window);
        return 1;
    }

    while (game.running) {
        game_handle_events(&game);
        game_update(&game);
        game_draw(&game, renderer);
        frame_clock_limit(&game.clock, GAME_RENDER_FPS);
    }

    game_shutdown(&game);
    app_quit(renderer, window);

    return 0;
}
