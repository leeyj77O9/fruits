#pragma once

#include <stddef.h>

#include "game/fruit.h"
#include "util/type.h"

typedef struct PlayerState {
    u64    score;
    Fruit *fruits;
    size_t fruitsCount;
    size_t fruitsCapacity;
} PlayerState;

bool state_init(PlayerState *state);
void state_free(PlayerState *state);

void state_add_fruit(PlayerState *state, vec2 pos, FruitType fruitType);
bool state_merge_touching(PlayerState *state);
