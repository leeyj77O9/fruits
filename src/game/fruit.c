#include "fruit.h"
#include <stdlib.h>
#include "../util/vec2.h"

const char* const FRUIT_TYPE_NAMES[] = {
    "Grape", "Strawbarry", "Lemon", "Apple", "Orange", "Pear",
    "Peach", "Kiwi", "Melon", "Watermelon",
};

const float FRUIT_RADIUSES[] = {
    1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5,   
};

const FruitInfo FRUITS_INFOS[] = {
    {
        "Grape",
        1.0,
        {0, 0, 0, 0},
    }, 
    {
        "Strawberry",
        1.5,
        {0, 0, 0, 0},
    }
}
const int MAX_RANDOM_FRUIT_TYPE = 5;

Fruit* fruit_create(vec2 pos, FruitType type) {
    static u16 id = 0;
    Fruit* f = malloc(sizeof(Fruit));
    *f = (Fruit){ .id = id++, .pos = pos, .r = FRUIT_RADIUSES[type], .type = type };
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
    return FRUIT_TYPE_NAMES[type];
}
FruitType fruit_get_random_fruitType() {
    return (FruitType)(rand() % MAX_RANDOM_FRUIT_TYPE);
}
void fruit_upgrade(Fruit *f) {
    f->type++;
    f->r = FRUIT_RADIUSES[f->type];
}