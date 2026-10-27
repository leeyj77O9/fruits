#include "game/text.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_FONT_SIZE 32.0f
#define TEXT_CACHE_KEY_SIZE 64

static const char *TEXT_FONT_CANDIDATES[] = {
    "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
    "/Library/Fonts/Arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "C:/Windows/Fonts/arialbd.ttf",
};

static TTF_Font    *text_font = NULL;
static bool         text_ready = false;

static SDL_Texture *cache_texture = NULL;
static char         cache_key[TEXT_CACHE_KEY_SIZE];
static f32          cache_width = 0.0f;
static f32          cache_height = 0.0f;
static bool         cache_valid = false;

static bool text_update_cache(SDL_Renderer *renderer, const char *text) {
    cache_valid = false;

    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(text_font, text, strlen(text), color);

    if (surface == NULL) {
        return false;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == NULL) {
        SDL_DestroySurface(surface);
        return false;
    }

    if (cache_texture != NULL) {
        SDL_DestroyTexture(cache_texture);
    }

    cache_texture = texture;
    cache_width = (f32)surface->w;
    cache_height = (f32)surface->h;

    snprintf(cache_key, sizeof(cache_key), "%s", text);
    cache_valid = true;

    SDL_DestroySurface(surface);
    return true;
}

bool text_init(void) {
    text_font = NULL;
    cache_texture = NULL;
    cache_valid = false;
    text_ready = false;

    if (!TTF_Init()) {
        SDL_Log("TTF 초기화 실패: %s", SDL_GetError());
        return false;
    }

    text_ready = true;

    const char *env = getenv("FRUITS_FONT");

    if (env != NULL && env[0] != '\0') {
        text_font = TTF_OpenFont(env, TEXT_FONT_SIZE);
    }

    size_t candidateCount = sizeof(TEXT_FONT_CANDIDATES) / sizeof(TEXT_FONT_CANDIDATES[0]);

    for (size_t i = 0; text_font == NULL && i < candidateCount; ++i) {
        text_font = TTF_OpenFont(TEXT_FONT_CANDIDATES[i], TEXT_FONT_SIZE);
    }

    if (text_font == NULL) {
        SDL_Log("글꼴을 찾지 못했습니다: %s", SDL_GetError());
    }

    return true;
}

void text_quit(void) {
    if (cache_texture != NULL) {
        SDL_DestroyTexture(cache_texture);
        cache_texture = NULL;
    }
    if (text_font != NULL) {
        TTF_CloseFont(text_font);
        text_font = NULL;
    }
    if (text_ready) {
        TTF_Quit();
        text_ready = false;
    }

    cache_valid = false;
}

void text_draw(SDL_Renderer *renderer, const char *text, f32 x, f32 y) {
    if (text_font == NULL || text == NULL) {
        return;
    }

    if (!cache_valid || strcmp(cache_key, text) != 0) {
        if (!text_update_cache(renderer, text)) {
            return;
        }
    }

    SDL_FRect dest = {x, y, cache_width, cache_height};
    SDL_RenderTexture(renderer, cache_texture, NULL, &dest);
}
