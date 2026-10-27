#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game/frame_clock.h"
#include "game/game.h"
#include "game/renderer.h"

typedef struct App {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    Game          game;
} App;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    App *app = SDL_calloc(1, sizeof(App));

    if (app == NULL) {
        SDL_Log("앱 상태 할당 실패: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    *appstate = app;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL 초기화 실패: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Fruits", GAME_WIDTH, GAME_HEIGHT, 0, &app->window, &app->renderer)) {
        SDL_Log("창/렌더러 생성 실패: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(app->renderer, 1);

    if (!renderer_init()) {
        SDL_Log("렌더링 초기화에 실패했습니다: %s", SDL_GetError());
    }

    if (!game_init(&app->game, app->window)) {
        SDL_Log("게임 초기화에 실패했습니다: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    App *app = appstate;

    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return SDL_APP_SUCCESS;
    }

    game_handle_event(&app->game, event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    App *app = appstate;

    game_update(&app->game);
    game_draw(&app->game, app->renderer);
    frame_clock_limit(&app->game.clock, GAME_RENDER_FPS);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    (void)result;

    App *app = appstate;

    if (app == NULL) {
        return;
    }

    game_shutdown(&app->game);
    renderer_quit();

    if (app->renderer != NULL) {
        SDL_DestroyRenderer(app->renderer);
    }
    if (app->window != NULL) {
        SDL_DestroyWindow(app->window);
    }

    SDL_free(app);
}
