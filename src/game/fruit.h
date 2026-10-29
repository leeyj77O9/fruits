#pragma once

#include <stdbool.h>

#include "util/rgba.h"
#include "util/type.h"
#include "util/vec2.h"

typedef enum FruitType {
    FRUIT_TYPE_GRAPE,
    FRUIT_TYPE_STRAWBERRY,
    FRUIT_TYPE_LEMON,
    FRUIT_TYPE_APPLE,
    FRUIT_TYPE_ORANGE,
    FRUIT_TYPE_PEACH,
    FRUIT_TYPE_MELON,
    FRUIT_TYPE_WATERMELON,
    FRUIT_TYPE_COUNT,
} FruitType;

typedef struct Fruit {
    u32       id;
    vec2      pos;
    vec2      vel;
    f32       r;
    f32       angle;
    f32       lastX;
    FruitType type;
    bool      sleeping;
    u16       sleepFrames;
} Fruit;

typedef struct FruitInfo {
    const char *name;
    f32         radius;
    rgba        color;
} FruitInfo;

extern const FruitInfo FRUITS_INFOS[FRUIT_TYPE_COUNT];

Fruit       fruit_make(vec2 pos, FruitType type);

f32         fruit_radius(FruitType type);

bool        fruit_equals_id(const Fruit *a, u32 id);
bool        fruit_can_combine(const Fruit *a, const Fruit *b);
bool        fruit_is_max_type(FruitType type);

const char *fruit_get_name(FruitType type);

FruitType   fruit_get_random_type(void);
