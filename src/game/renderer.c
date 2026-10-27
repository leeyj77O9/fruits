#include "renderer.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 점수 글꼴 크기(px)와 화면 여백 */
static const float SCORE_FONT_SIZE = 32.0f;
static const float SCORE_MARGIN_X = 20.0f;
static const float SCORE_MARGIN_Y = 14.0f;

/* 시스템 폰트를 못 찾으면 점수만 표시하지 않고 게임은 계속 돌린다 */
static const char *SCORE_FONT_CANDIDATES[] = {
    "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
    "/Library/Fonts/Arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "C:/Windows/Fonts/arialbd.ttf",
};

static TTF_Font   *score_font = NULL;
static SDL_Texture *score_texture = NULL;
static u64         score_shown = 0;
static bool        score_shown_valid = false;
static bool        ttf_ready = false;
static float       score_width = 0.0f;
static float       score_height = 0.0f;

bool renderer_init(void) {
    if (!TTF_Init()) {
        SDL_Log("TTF 초기화 실패: %s", SDL_GetError());
        return false;
    }
    ttf_ready = true;

    /* FRUITS_FONT 환경변수로 원하는 폰트를 지정할 수 있다 */
    const char *env = getenv("FRUITS_FONT");

    if (env != NULL && env[0] != '\0') {
        score_font = TTF_OpenFont(env, SCORE_FONT_SIZE);
    }

    for (size_t i = 0; score_font == NULL && i < sizeof(SCORE_FONT_CANDIDATES) / sizeof(SCORE_FONT_CANDIDATES[0]); ++i) {
        score_font = TTF_OpenFont(SCORE_FONT_CANDIDATES[i], SCORE_FONT_SIZE);
    }

    if (score_font == NULL) {
        SDL_Log("점수용 폰트를 찾지 못했습니다: %s", SDL_GetError());
    }

    return true;
}

void renderer_quit(void) {
    if (score_texture != NULL) {
        SDL_DestroyTexture(score_texture);
        score_texture = NULL;
    }
    if (score_font != NULL) {
        TTF_CloseFont(score_font);
        score_font = NULL;
    }
    if (ttf_ready) {
        TTF_Quit();
        ttf_ready = false;
    }
    score_shown_valid = false;
}

void render(SDL_Renderer *renderer, PlayerState* state) {
    render_score(renderer, state->score);
    
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        render_fruit(renderer, &state->fruits[i]);
    }
}

void render_score(SDL_Renderer *renderer, u64 score) {
    if (score_font == NULL) {
        return;
    }

    /* 점수가 바뀔 때만 텍스처를 다시 만든다 */
    if (!score_shown_valid || score != score_shown) {
        char text[32];
        int length = snprintf(text, sizeof(text), "%llu", (unsigned long long)score);

        if (length <= 0) {
            return;
        }

        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface *surface = TTF_RenderText_Blended(score_font, text, (size_t)length, color);

        if (surface == NULL) {
            return;
        }

        if (score_texture != NULL) {
            SDL_DestroyTexture(score_texture);
            score_texture = NULL;
        }

        score_texture = SDL_CreateTextureFromSurface(renderer, surface);

        if (score_texture != NULL) {
            score_width = (float)surface->w;
            score_height = (float)surface->h;
            score_shown = score;
            score_shown_valid = true;
        }

        SDL_DestroySurface(surface);
    }

    if (score_texture == NULL) {
        return;
    }

    SDL_FRect dest = {SCORE_MARGIN_X, SCORE_MARGIN_Y, score_width, score_height};
    SDL_RenderTexture(renderer, score_texture, NULL, &dest);
}

void render_fruit(SDL_Renderer *renderer, Fruit *fruit) {
    float r = fruit->r;
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
