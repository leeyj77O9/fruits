#include "game/renderer.h"
#include "game/game.h"
#include "game/text.h"
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static const f32 SCORE_MARGIN_X = 20.0f;
static const f32 SCORE_MARGIN_Y = 14.0f;
static const f32 CIRCLE_PADDING = 1.0f;
static const Uint8 SPRITE_ALPHA_THRESHOLD = 8;

enum {
    CIRCLE_SAMPLES = 8,
    CIRCLE_SAMPLE_COUNT = CIRCLE_SAMPLES * CIRCLE_SAMPLES,
};

typedef struct FruitSprite {
    SDL_Texture *texture;
    f32          width;
    f32          height;
    bool         tinted;
} FruitSprite;

static const char *const SPRITE_FILES[FRUIT_TYPE_COUNT] = {
    [FRUIT_TYPE_GRAPE] = "00",
    [FRUIT_TYPE_STRAWBERRY] = "10",
    [FRUIT_TYPE_LEMON] = "20",
    [FRUIT_TYPE_APPLE] = "30",
    [FRUIT_TYPE_ORANGE] = "40",
    [FRUIT_TYPE_PEAR] = "01",
    [FRUIT_TYPE_PEACH] = "11",
    [FRUIT_TYPE_MELON] = "31",
    [FRUIT_TYPE_WATERMELON] = "41",
};

static FruitSprite fruitSprites[FRUIT_TYPE_COUNT];

static bool renderer_find_sprite_box(SDL_Surface *surface, SDL_Rect *box) {
    int minX = surface->w;
    int minY = surface->h;
    int maxX = -1;
    int maxY = -1;

    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            Uint8 r, g, b, a;

            SDL_ReadSurfacePixel(surface, x, y, &r, &g, &b, &a);

            if (a <= SPRITE_ALPHA_THRESHOLD) {
                continue;
            }

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }

    if (maxX < 0) {
        return false;
    }

    *box = (SDL_Rect){minX, minY, maxX - minX + 1, maxY - minY + 1};
    return true;
}

static SDL_Surface *renderer_resample_surface(SDL_Surface *source, const SDL_Rect *box, int width, int height) {
    SDL_Surface *result = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);

    if (result == NULL) {
        return NULL;
    }

    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(result->format);
    f32 stepX = (f32)box->w / (f32)width;
    f32 stepY = (f32)box->h / (f32)height;

    for (int dy = 0; dy < height; ++dy) {
        Uint32 *row = (Uint32 *)((Uint8 *)result->pixels + (size_t)dy * (size_t)result->pitch);
        f32 y0 = (f32)box->y + (f32)dy * stepY;
        f32 y1 = y0 + stepY;
        int sy0 = (int)floorf(y0);
        int sy1 = (int)ceilf(y1);

        if (sy0 < 0) sy0 = 0;
        if (sy1 > source->h) sy1 = source->h;

        for (int dx = 0; dx < width; ++dx) {
            f32 x0 = (f32)box->x + (f32)dx * stepX;
            f32 x1 = x0 + stepX;
            int sx0 = (int)floorf(x0);
            int sx1 = (int)ceilf(x1);
            f32 rSum = 0.0f;
            f32 gSum = 0.0f;
            f32 bSum = 0.0f;
            f32 aSum = 0.0f;
            f32 wSum = 0.0f;

            if (sx0 < 0) sx0 = 0;
            if (sx1 > source->w) sx1 = source->w;

            for (int sy = sy0; sy < sy1; ++sy) {
                f32 wy = fminf(y1, (f32)(sy + 1)) - fmaxf(y0, (f32)sy);

                if (wy <= 0.0f) {
                    continue;
                }

                for (int sx = sx0; sx < sx1; ++sx) {
                    f32 wx = fminf(x1, (f32)(sx + 1)) - fmaxf(x0, (f32)sx);

                    if (wx <= 0.0f) {
                        continue;
                    }

                    Uint8 r, g, b, a;
                    f32 weight = wx * wy;

                    SDL_ReadSurfacePixel(source, sx, sy, &r, &g, &b, &a);

                    rSum += (f32)r * (f32)a * weight;
                    gSum += (f32)g * (f32)a * weight;
                    bSum += (f32)b * (f32)a * weight;
                    aSum += (f32)a * weight;
                    wSum += weight;
                }
            }

            if (aSum > 0.0f && wSum > 0.0f) {
                Uint8 r = (Uint8)lroundf(rSum / aSum);
                Uint8 g = (Uint8)lroundf(gSum / aSum);
                Uint8 b = (Uint8)lroundf(bSum / aSum);
                Uint8 a = (Uint8)lroundf(aSum / wSum);

                row[dx] = SDL_MapRGBA(format, NULL, r, g, b, a);
            } else {
                row[dx] = 0;
            }
        }
    }

    return result;
}

static bool renderer_load_sprite(SDL_Renderer *renderer, const char *basePath, const char *name, f32 radius, FruitSprite *result) {
    char path[1024];
    SDL_Surface *surface = NULL;

    if (basePath != NULL) {
        snprintf(path, sizeof(path), "%sasset/fruit/%s.png", basePath, name);
        surface = IMG_Load(path);
    }

    if (surface == NULL) {
        snprintf(path, sizeof(path), "asset/fruit/%s.png", name);
        surface = IMG_Load(path);
    }

    if (surface == NULL) {
        return false;
    }

    SDL_Rect box;

    if (!renderer_find_sprite_box(surface, &box)) {
        SDL_DestroySurface(surface);
        return false;
    }

    int width = (int)lroundf(radius * 2.0f);
    int height = (int)lroundf((f32)width * (f32)box.h / (f32)box.w);

    if (width < 1) width = 1;
    if (height < 1) height = 1;

    SDL_Surface *scaled = renderer_resample_surface(surface, &box, width, height);

    SDL_DestroySurface(surface);

    if (scaled == NULL) {
        return false;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, scaled);

    SDL_DestroySurface(scaled);

    if (texture == NULL) {
        return false;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    result->texture = texture;
    result->width = (f32)width;
    result->height = (f32)height;
    result->tinted = false;
    return true;
}

static bool renderer_create_circle_texture(SDL_Renderer *renderer, f32 radius, FruitSprite *result) {
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
    result->width = (f32)size;
    result->height = (f32)size;
    result->tinted = true;
    return true;
}

bool renderer_init(SDL_Renderer *renderer) {
    bool ok = text_init();
    const char *basePath = SDL_GetBasePath();

    for (int type = 0; type < FRUIT_TYPE_COUNT; ++type) {
        FruitSprite *sprite = &fruitSprites[type];

        sprite->texture = NULL;
        sprite->width = 0.0f;
        sprite->height = 0.0f;
        sprite->tinted = false;

        if (SPRITE_FILES[type] != NULL &&
            renderer_load_sprite(renderer, basePath, SPRITE_FILES[type], fruit_radius((FruitType)type), sprite)) {
            continue;
        }

        if (!renderer_create_circle_texture(renderer, fruit_radius((FruitType)type), sprite)) {
            SDL_Log("과일 텍스처 생성 실패: %s", SDL_GetError());
            ok = false;
        }
    }

    return ok;
}

void renderer_quit(void) {
    for (int i = 0; i < FRUIT_TYPE_COUNT; ++i) {
        if (fruitSprites[i].texture != NULL) {
            SDL_DestroyTexture(fruitSprites[i].texture);
        }

        fruitSprites[i] = (FruitSprite){0};
    }

    text_quit();
}

static void renderer_draw_score(SDL_Renderer *renderer, u64 score) {
    char text[32];

    snprintf(text, sizeof(text), "%llu", (unsigned long long)score);
    text_draw(renderer, text, SCORE_MARGIN_X, SCORE_MARGIN_Y);
}

static void renderer_draw_fruit(SDL_Renderer *renderer, const Fruit *fruit) {
    const FruitSprite *sprite = &fruitSprites[fruit->type];
    rgba color = FRUITS_INFOS[fruit->type].color;

    if (sprite->texture != NULL) {
        SDL_FRect dest = {
            fruit->pos.x - sprite->width * 0.5f,
            fruit->pos.y + fruit->r - sprite->height,
            sprite->width,
            sprite->height,
        };

        if (sprite->tinted) {
            SDL_SetTextureColorMod(sprite->texture, color.r, color.g, color.b);
            SDL_SetTextureAlphaMod(sprite->texture, color.a);
        }

        if (fabsf(fruit->angle) > 0.0001f) {
            SDL_FPoint center = {sprite->width * 0.5f, sprite->height * 0.5f};

            SDL_RenderTextureRotated(renderer, sprite->texture, NULL, &dest, (double)(fruit->angle * 57.2957795f), &center, SDL_FLIP_NONE);
        } else {
            SDL_RenderTexture(renderer, sprite->texture, NULL, &dest);
        }

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

void renderer_draw_guide(SDL_Renderer *renderer, bool danger) {
    SDL_FRect line = {0.0f, GAME_OVER_LINE_Y - 1.0f, (f32)GAME_WIDTH, 2.0f};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (danger) {
        SDL_SetRenderDrawColor(renderer, 255, 96, 96, 130);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 34);
    }

    SDL_RenderFillRect(renderer, &line);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void renderer_draw_game_over(SDL_Renderer *renderer, u64 score) {
    SDL_FRect cover = {0.0f, 0.0f, (f32)GAME_WIDTH, (f32)GAME_HEIGHT};
    char text[64];

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 190);
    SDL_RenderFillRect(renderer, &cover);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    text_draw_centered(renderer, "GAME OVER", (f32)GAME_WIDTH * 0.5f, 296.0f, 58.0f);

    snprintf(text, sizeof(text), "SCORE %llu", (unsigned long long)score);
    text_draw_centered(renderer, text, (f32)GAME_WIDTH * 0.5f, 384.0f, 38.0f);

    text_draw_centered(renderer, "PRESS R TO RESTART", (f32)GAME_WIDTH * 0.5f, 452.0f, 24.0f);
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

void renderer_draw_held(SDL_Renderer *renderer, const Fruit *held) {
    if (held != NULL) {
        renderer_draw_fruit(renderer, held);
    }
}
