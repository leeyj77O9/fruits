#include "game/renderer.h"
#include "game/text.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static const f32 SCORE_MARGIN_X = 20.0f;
static const f32 SCORE_MARGIN_Y = 14.0f;
static const f32 CIRCLE_PADDING = 1.0f;

enum {
    CIRCLE_SAMPLES = 8,
    CIRCLE_SAMPLE_COUNT = CIRCLE_SAMPLES * CIRCLE_SAMPLES,
};

typedef struct CircleTexture {
    SDL_Texture *texture;
    f32          half;
} CircleTexture;

static CircleTexture fruitTextures[FRUIT_TYPE_COUNT];

static bool renderer_create_circle_texture(SDL_Renderer *renderer, f32 radius, CircleTexture *result) {
    int size = (int)ceilf(radius * 2.0f + CIRCLE_PADDING * 2.0f);
    SDL_Surface *surface = SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA8888);

    if (surface == NULL) {
        return false;
    }

    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(surface->format);
    Uint32 colors[CIRCLE_SAMPLE_COUNT + 1];

    for (int i = 0; i <= CIRCLE_SAMPLE_COUNT; ++i) {
        Uint8 alpha = (Uint8)((i * 255) / CIRCLE_SAMPLE_COUNT);
        colors[i] = SDL_MapRGBA(format, NULL, 255, 255, 255, alpha);
    }

    f32 center = (f32)size * 0.5f;
    f32 radiusSq = radius * radius;
    f32 sampleStep = 1.0f / (f32)CIRCLE_SAMPLES;

    for (int y = 0; y < size; ++y) {
        Uint32 *row = (Uint32 *)((Uint8 *)surface->pixels + (size_t)y * (size_t)surface->pitch);

        for (int x = 0; x < size; ++x) {
            int hits = 0;

            for (int sy = 0; sy < CIRCLE_SAMPLES; ++sy) {
                f32 py = (f32)y + ((f32)sy + 0.5f) * sampleStep - center;

                for (int sx = 0; sx < CIRCLE_SAMPLES; ++sx) {
                    f32 px = (f32)x + ((f32)sx + 0.5f) * sampleStep - center;

                    if (px * px + py * py <= radiusSq) {
                        hits++;
                    }
                }
            }

            row[x] = colors[hits];
        }
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_DestroySurface(surface);

    if (texture == NULL) {
        return false;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    result->texture = texture;
    result->half = (f32)size * 0.5f;
    return true;
}

bool renderer_init(SDL_Renderer *renderer) {
    bool ok = text_init();

    for (int type = 0; type < FRUIT_TYPE_COUNT; ++type) {
        fruitTextures[type].texture = NULL;
        fruitTextures[type].half = 0.0f;

        if (!renderer_create_circle_texture(renderer, fruit_radius((FruitType)type), &fruitTextures[type])) {
            SDL_Log("과일 텍스처 생성 실패: %s", SDL_GetError());
            ok = false;
        }
    }

    return ok;
}

void renderer_quit(void) {
    for (int i = 0; i < FRUIT_TYPE_COUNT; ++i) {
        if (fruitTextures[i].texture != NULL) {
            SDL_DestroyTexture(fruitTextures[i].texture);
            fruitTextures[i].texture = NULL;
            fruitTextures[i].half = 0.0f;
        }
    }

    text_quit();
}

static void renderer_draw_score(SDL_Renderer *renderer, u64 score) {
    char text[32];

    snprintf(text, sizeof(text), "%llu", (unsigned long long)score);
    text_draw(renderer, text, SCORE_MARGIN_X, SCORE_MARGIN_Y);
}

static void renderer_draw_fruit(SDL_Renderer *renderer, const Fruit *fruit) {
    const CircleTexture *circle = &fruitTextures[fruit->type];
    rgba color = FRUITS_INFOS[fruit->type].color;

    if (circle->texture != NULL) {
        SDL_FRect dest = {
            fruit->pos.x - circle->half,
            fruit->pos.y - circle->half,
            circle->half * 2.0f,
            circle->half * 2.0f,
        };

        SDL_SetTextureColorMod(circle->texture, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(circle->texture, color.a);
        SDL_RenderTexture(renderer, circle->texture, NULL, &dest);
        return;
    }

    f32 r = fruit->r;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (f32 dx = -r; dx < r; dx += 1.0f) {
        for (f32 dy = -r; dy < r; dy += 1.0f) {
            if ((dx * dx) + (dy * dy) <= r * r) {
                SDL_RenderPoint(renderer, fruit->pos.x + dx, fruit->pos.y + dy);
            }
        }
    }
}

void renderer_draw(SDL_Renderer *renderer, const PlayerState *state, const Fruit *held) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        renderer_draw_fruit(renderer, &state->fruits[i]);
    }

    if (held != NULL) {
        renderer_draw_fruit(renderer, held);
    }

    renderer_draw_score(renderer, state->score);
}
