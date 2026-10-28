#include "game/text.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_FONT_SIZE 32.0f
#define TEXT_CACHE_KEY_SIZE 96

enum {
    TEXT_CACHE_COUNT = 8,
};

static const char *TEXT_FONT_CANDIDATES[] = {
    "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
    "/Library/Fonts/Arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "C:/Windows/Fonts/segoeuib.ttf",
    "C:/Windows/Fonts/arialbd.ttf",
    "C:/Windows/Fonts/arial.ttf",
};

typedef struct TextEntry {
    SDL_Texture *texture;
    char         key[TEXT_CACHE_KEY_SIZE];
    f32          width;
    f32          height;
    bool         used;
} TextEntry;

static TTF_Font  *text_font = NULL;
static bool       text_ready = false;
static TextEntry  cache[TEXT_CACHE_COUNT];
static int        cacheNext = 0;

static void text_release(TextEntry *entry) {
    if (entry->texture != NULL) {
        SDL_DestroyTexture(entry->texture);
    }

    entry->texture = NULL;
    entry->used = false;
    entry->width = 0.0f;
    entry->height = 0.0f;
    entry->key[0] = '\0';
}

static bool text_build(TextEntry *entry, SDL_Renderer *renderer, const char *text, const char *key, f32 size) {
    if (!TTF_SetFontSize(text_font, size)) {
        return false;
    }

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

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

    text_release(entry);

    entry->texture = texture;
    entry->width = (f32)surface->w;
    entry->height = (f32)surface->h;
    entry->used = true;

    snprintf(entry->key, sizeof(entry->key), "%s", key);

    SDL_DestroySurface(surface);
    return true;
}

static TextEntry *text_lookup(SDL_Renderer *renderer, const char *text, f32 size) {
    char key[TEXT_CACHE_KEY_SIZE];

    snprintf(key, sizeof(key), "%.0f|%s", size, text);

    for (int i = 0; i < TEXT_CACHE_COUNT; ++i) {
        if (cache[i].used && strcmp(cache[i].key, key) == 0) {
            return &cache[i];
        }
    }

    TextEntry *entry = &cache[cacheNext];

    cacheNext = (cacheNext + 1) % TEXT_CACHE_COUNT;

    if (!text_build(entry, renderer, text, key, size)) {
        return NULL;
    }

    return entry;
}

bool text_init(void) {
    text_font = NULL;
    text_ready = false;
    cacheNext = 0;

    for (int i = 0; i < TEXT_CACHE_COUNT; ++i) {
        cache[i] = (TextEntry){0};
    }

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
    for (int i = 0; i < TEXT_CACHE_COUNT; ++i) {
        text_release(&cache[i]);
    }

    cacheNext = 0;

    if (text_font != NULL) {
        TTF_CloseFont(text_font);
        text_font = NULL;
    }

    if (text_ready) {
        TTF_Quit();
        text_ready = false;
    }
}

void text_draw_sized(SDL_Renderer *renderer, const char *text, f32 x, f32 y, f32 size) {
    if (text_font == NULL || text == NULL) {
        return;
    }

    TextEntry *entry = text_lookup(renderer, text, size);

    if (entry == NULL) {
        return;
    }

    SDL_FRect dest = {x, y, entry->width, entry->height};
    SDL_RenderTexture(renderer, entry->texture, NULL, &dest);
}

void text_draw(SDL_Renderer *renderer, const char *text, f32 x, f32 y) {
    text_draw_sized(renderer, text, x, y, TEXT_FONT_SIZE);
}

void text_draw_centered(SDL_Renderer *renderer, const char *text, f32 centerX, f32 y, f32 size) {
    if (text_font == NULL || text == NULL) {
        return;
    }

    TextEntry *entry = text_lookup(renderer, text, size);

    if (entry == NULL) {
        return;
    }

    SDL_FRect dest = {centerX - entry->width * 0.5f, y, entry->width, entry->height};
    SDL_RenderTexture(renderer, entry->texture, NULL, &dest);
}
