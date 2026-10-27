#pragma once

#include "../util/vec2.h"
#include "../util/type.h"
#include "../util/rgba.h"
#include <stdbool.h>

typedef enum FruitType {
   FRUIT_TYPE_GRAPE,
   FRUIT_TYPE_STRAWBERRY,
   FRUIT_TYPE_LEMON,
   FRUIT_TYPE_APPLE,
   FRUIT_TYPE_ORANGE,
   FRUIT_TYPE_PEAR,
   FRUIT_TYPE_PEACH,
   FRUIT_TYPE_KIWI,
   FRUIT_TYPE_MELON,
   FRUIT_TYPE_WATERMELON,
} FruitType;

typedef struct Fruit {
    u16       id;
    vec2      pos;
    vec2      vel;
    float     r;
    FruitType type;
    bool      sleeping;
    u16       sleepFrames;
} Fruit;

typedef struct FruitInfo {
    const char* name;
    const float r;
    const rgba  color;
} FruitInfo;

extern const FruitInfo FRUITS_INFOS[];
extern const int MAX_RANDOM_FRUIT_TYPE;
extern const float FRUIT_PIXELS_PER_UNIT;

Fruit*      fruit_create(vec2 pos, FruitType type);
Fruit       fruit_make(vec2 pos, FruitType type);
Fruit*      fruit_clone(Fruit* f);

bool        fruit_equals(Fruit* a, Fruit* b);
bool        fruit_equals_type(Fruit* a, FruitType type);
bool        fruit_equals_id(Fruit* a, u16 id);
bool        fruit_can_combine(Fruit* a, Fruit* b);

const char* fruit_get_name(FruitType type);

FruitType   fruit_get_random_fruitType();

void        fruit_upgrade(Fruit* f);