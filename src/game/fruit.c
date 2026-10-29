#include "game/fruit.h"
#include <stdlib.h>

#define FRUIT_RANDOM_TYPE_COUNT 5

static u32 fruit_next_id = 0;

const FruitInfo FRUITS_INFOS[FRUIT_TYPE_COUNT] = {
    { "Grape",       20.0f, RGBA_PURPLE },
    { "Strawberry",  25.0f, RGBA_RED },
    { "Lemon",       31.0f, RGBA_YELLOW },
    { "Apple",       38.0f, RGBA_RED },
    { "Orange",      47.0f, RGBA_ORANGE },
    { "Peach",       72.0f, RGBA_PINK },
    { "Melon",       89.0f, RGBA_GREEN },
    { "Watermelon", 110.0f, RGBA_DARK_GREEN },
};

_Static_assert(sizeof(FRUITS_INFOS) / sizeof(FRUITS_INFOS[0]) == FRUIT_TYPE_COUNT,
               "FRUITS_INFOS 항목 수가 FRUIT_TYPE_COUNT와 같아야 한다");
_Static_assert(FRUIT_RANDOM_TYPE_COUNT <= FRUIT_TYPE_COUNT,
               "떨어뜨릴 수 있는 과일 종류가 전체 과일 수보다 많을 수 없다");

Fruit fruit_make(vec2 pos, FruitType type) {
    return (Fruit){
        .id = fruit_next_id++,
        .pos = pos,
        .vel = vec2_zero(),
        .r = fruit_radius(type),
        .angle = 0.0f,
        .lastX = pos.x,
        .type = type,
        .sleeping = false,
        .sleepFrames = 0,
    };
}

f32 fruit_radius(FruitType type) {
    return FRUITS_INFOS[type].radius;
}

bool fruit_equals_id(const Fruit *a, u32 id) {
    return a->id == id;
}

bool fruit_can_combine(const Fruit *a, const Fruit *b) {
    return !fruit_is_max_type(a->type) && a->type == b->type;
}

bool fruit_is_max_type(FruitType type) {
    return type == FRUIT_TYPE_WATERMELON;
}

const char *fruit_get_name(FruitType type) {
    return FRUITS_INFOS[type].name;
}

FruitType fruit_get_random_type(void) {
    return (FruitType)(rand() % FRUIT_RANDOM_TYPE_COUNT);
}
