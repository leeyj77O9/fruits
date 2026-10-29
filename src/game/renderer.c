#include "game/renderer.h"
#include "game/game.h"
#include "game/text.h"
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static const rgba TEXT_COLOR = {28, 28, 28, 255};
static const rgba LABEL_COLOR = {70, 70, 70, 255};
static const rgba NEXT_LABEL_COLOR = {158, 158, 158, 255};
static const rgba SCORE_VALUE_COLOR = {226, 228, 233, 255};

static const f32 SCORE_X = 26.0f;
static const f32 SCORE_VALUE_Y = 10.0f;
static const f32 SCORE_VALUE_SIZE = 56.0f;
static const f32 SCORE_VALUE_SMALL_SIZE = 40.0f;
static const f32 SCORE_PULSE_DURATION = 0.65f;
static const f32 SCORE_PULSE_ATTACK = 0.55f;
static const f32 SCORE_PULSE_FREQUENCY = 15.0f;
static const f32 SCORE_PULSE_DAMPING = 5.2f;
static const f32 SCORE_PULSE_CENTER_BIAS = 0.55f;
static const f32 SCORE_ECHO_BLOOM = 0.04f;
static const f32 SCORE_ECHO_SCALES[] = {0.32f, 0.25f, 0.18f, 0.11f, 0.05f};
static const f32 SCORE_ECHO_DAMPINGS[] = {2.4f, 3.1f, 3.9f, 4.8f, 5.8f};
static const rgba SCORE_ECHO_COLORS[] = {
    {242, 243, 247, 255},
    {239, 240, 244, 255},
    {236, 238, 242, 255},
    {233, 235, 240, 255},
    {229, 231, 236, 255},
};
static const f32 EFFECT_RING_THICKNESS_SCALE = 0.09f;
static const f32 EFFECT_RING_THICKNESS_MIN = 1.5f;
static const f32 EFFECT_RING_THICKNESS_MAX = 12.0f;
static const f32 CIRCLE_PADDING = 1.0f;
static const Uint8 SPRITE_ALPHA_THRESHOLD = 8;

static const f32 NEXT_CENTER_X = 552.0f;
static const f32 NEXT_CENTER_Y = 64.0f;
static const f32 NEXT_MAX_SIZE = 70.0f;
static const f32 NEXT_LABEL_Y = 8.0f;
static const f32 NEXT_LABEL_SIZE = 18.0f;
static const Uint8 NEXT_ALPHA = 160;

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

static f32 renderer_score_pulse_scale(f32 time) {
    if (time >= SCORE_PULSE_DURATION) {
        return 1.0f;
    }

    f32 wave = expf(-SCORE_PULSE_DAMPING * time) * sinf(SCORE_PULSE_FREQUENCY * time);

    return fmaxf(1.0f, 1.0f + SCORE_PULSE_ATTACK * wave);
}

static f32 renderer_score_echo_scale(f32 time, f32 amount, f32 damping) {
    if (time >= SCORE_PULSE_DURATION) {
        return 1.0f;
    }

    f32 progress = time / SCORE_PULSE_DURATION;
    f32 bloom = 1.0f - expf(-time / SCORE_ECHO_BLOOM);
    f32 window = 1.0f - progress * progress * progress * progress;

    return 1.0f + amount * bloom * window * expf(-damping * time);
}

static void renderer_draw_score_text(SDL_Renderer *renderer, const char *text, f32 baseSize, f32 baseWidth, f32 scale, rgba color) {
    f32 size = lroundf(baseSize * scale);
    f32 width = text_measure(text, size);
    f32 x = SCORE_X + (baseWidth - width) * 0.5f;
    f32 y = SCORE_VALUE_Y - (size - baseSize) * SCORE_PULSE_CENTER_BIAS;

    text_draw_colored(renderer, text, x, y, size, color);
}

static void renderer_draw_score(SDL_Renderer *renderer, u64 score, f32 pulseTime) {
    char text[32];
    int digits = snprintf(text, sizeof(text), "%llu", (unsigned long long)score);
    f32 baseSize = digits > 5 ? SCORE_VALUE_SMALL_SIZE : SCORE_VALUE_SIZE;
    f32 baseWidth = text_measure(text, baseSize);

    for (size_t i = 0; i < sizeof(SCORE_ECHO_SCALES) / sizeof(SCORE_ECHO_SCALES[0]); ++i) {
        f32 scale = renderer_score_echo_scale(pulseTime, SCORE_ECHO_SCALES[i], SCORE_ECHO_DAMPINGS[i]);

        if (scale > 1.0001f) {
            renderer_draw_score_text(renderer, text, baseSize, baseWidth, scale, SCORE_ECHO_COLORS[i]);
        }
    }

    renderer_draw_score_text(renderer, text, baseSize, baseWidth, renderer_score_pulse_scale(pulseTime), SCORE_VALUE_COLOR);
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

void renderer_draw_effects(SDL_Renderer *renderer, const EffectPool *pool) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < EFFECT_POOL_SIZE; ++i) {
        const Effect *effect = &pool->items[i];

        if (!effect->active) {
            continue;
        }

        f32 fade = effect->life / effect->duration;

        if (fade < 0.0f) {
            fade = 0.0f;
        }

        Uint8 alpha = (Uint8)lroundf((effect->spark ? 255.0f : 240.0f) * fade);

        SDL_SetRenderDrawColor(renderer, effect->color.r, effect->color.g, effect->color.b, alpha);

        if (effect->spark) {
            SDL_FRect dot = {
                effect->pos.x - effect->radius,
                effect->pos.y - effect->radius,
                effect->radius * 2.0f,
                effect->radius * 2.0f,
            };

            SDL_RenderFillRect(renderer, &dot);
            continue;
        }

        f32 radius = effect->radius;
        f32 thickness = radius * EFFECT_RING_THICKNESS_SCALE + EFFECT_RING_THICKNESS_MIN;
        f32 inner = radius - thickness;
        int extent = (int)ceilf(radius);

        if (thickness > EFFECT_RING_THICKNESS_MAX) {
            thickness = EFFECT_RING_THICKNESS_MAX;
            inner = radius - thickness;
        }

        for (int offset = -extent; offset <= extent; ++offset) {
            f32 y = (f32)offset;
            f32 outerSq = radius * radius - y * y;

            if (outerSq <= 0.0f) {
                continue;
            }

            f32 outerX = sqrtf(outerSq);
            f32 innerSq = inner * inner - y * y;
            f32 innerX = innerSq > 0.0f ? sqrtf(innerSq) : 0.0f;
            f32 lineY = effect->pos.y + y;

            SDL_RenderLine(renderer, effect->pos.x - outerX, lineY, effect->pos.x - innerX, lineY);
            SDL_RenderLine(renderer, effect->pos.x + innerX, lineY, effect->pos.x + outerX, lineY);
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void renderer_draw_guide(SDL_Renderer *renderer, bool danger) {
    SDL_FRect line = {0.0f, GAME_OVER_LINE_Y - 1.0f, (f32)GAME_WIDTH, 2.0f};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (danger) {
        SDL_SetRenderDrawColor(renderer, 226, 58, 58, 170);
    } else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 46);
    }

    SDL_RenderFillRect(renderer, &line);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void renderer_draw_game_over(SDL_Renderer *renderer, u64 score) {
    SDL_FRect cover = {0.0f, 0.0f, (f32)GAME_WIDTH, (f32)GAME_HEIGHT};
    char text[64];

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 242);
    SDL_RenderFillRect(renderer, &cover);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    text_draw_centered_colored(renderer, "GAME OVER", (f32)GAME_WIDTH * 0.5f, 296.0f, 58.0f, TEXT_COLOR);

    snprintf(text, sizeof(text), "SCORE %llu", (unsigned long long)score);
    text_draw_centered_colored(renderer, text, (f32)GAME_WIDTH * 0.5f, 384.0f, 38.0f, TEXT_COLOR);

    text_draw_centered_colored(renderer, "PRESS R TO RESTART", (f32)GAME_WIDTH * 0.5f, 452.0f, 24.0f, LABEL_COLOR);
}

void renderer_draw(SDL_Renderer *renderer, const PlayerState *state, const Fruit *held) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    renderer_draw_score(renderer, (u64)state->scoreShown, state->scorePulseTime);

    for (size_t i = 0; i < state->fruitsCount; ++i) {
        renderer_draw_fruit(renderer, &state->fruits[i]);
    }

    if (held != NULL) {
        renderer_draw_fruit(renderer, held);
    }
}

void renderer_draw_held(SDL_Renderer *renderer, const Fruit *held) {
    if (held != NULL) {
        renderer_draw_fruit(renderer, held);
    }
}

void renderer_draw_next(SDL_Renderer *renderer, FruitType type) {
    const FruitSprite *sprite = &fruitSprites[type];

    text_draw_centered_colored(renderer, "NEXT", NEXT_CENTER_X, NEXT_LABEL_Y, NEXT_LABEL_SIZE, NEXT_LABEL_COLOR);

    if (sprite->texture == NULL) {
        return;
    }

    f32 longest = sprite->width > sprite->height ? sprite->width : sprite->height;
    f32 scale = NEXT_MAX_SIZE / longest;
    f32 width = sprite->width * scale;
    f32 height = sprite->height * scale;
    SDL_FRect dest = {
        NEXT_CENTER_X - width * 0.5f,
        NEXT_CENTER_Y - height * 0.5f,
        width,
        height,
    };

    if (sprite->tinted) {
        rgba color = FRUITS_INFOS[type].color;

        SDL_SetTextureColorMod(sprite->texture, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(sprite->texture, color.a);
    }

    SDL_SetTextureAlphaMod(sprite->texture, NEXT_ALPHA);
    SDL_RenderTexture(renderer, sprite->texture, NULL, &dest);
    SDL_SetTextureAlphaMod(sprite->texture, 255);
}
