#include "fruit.h"
#include <stdlib.h>
#include "../util/vec2.h"

const FruitInfo FRUITS_INFOS[] = {
    { "Grape",      1.0, RGBA_PURPLE },
    { "Strawberry", 1.5, RGBA_RED },
    { "Lemon",      2.0, RGBA_YELLOW },
    { "Apple",      2.5, RGBA_RED },
    { "Orange",     3.0, RGBA_ORANGE },
    { "Pear",       3.5, RGBA_YELLOW_GREEN },
    { "Peach",      4.0, RGBA_PINK },
    { "Kiwi",       4.5, RGBA_BROWN },
    { "Melon",      5.0, RGBA_GREEN },
    { "Watermelon", 5.5, RGBA_DARK_GREEN },
};
const int MAX_RANDOM_FRUIT_TYPE = 5;

Fruit* fruit_create(vec2 pos, FruitType type) {
    static u16 id = 0;
    Fruit* f = malloc(sizeof(Fruit));
    *f = (Fruit){ .id = id++, .pos = pos, .r = FRUITS_INFOS[type].r, .type = type };
    return f;
}
Fruit* fruit_clone(Fruit* f) {
    return fruit_create(f->pos, f->type);
}
bool fruit_equals(Fruit* a, Fruit* b) {
    return a->type == b->type;
}
bool fruit_equals_type(Fruit* a, FruitType type) {
    return a->type == type;
}
bool fruit_equals_id(Fruit* a, u16 id) {
    return a->id == id;
}
bool fruit_can_combine(Fruit* a, Fruit* b) {
    return fruit_equals_type(a, b->type) && a->r == b->r;
}
const char* fruit_get_name(FruitType type) {
    return FRUITS_INFOS[type].name;
}
FruitType fruit_get_random_fruitType() {
    return (FruitType)(rand() % MAX_RANDOM_FRUIT_TYPE);
}
void fruit_upgrade(Fruit *f) {
    f->type++;
    f->r = FRUITS_INFOS[f->type].r;
}