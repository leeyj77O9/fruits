#pragma once

#include "fruit.h"
#include "../util/type.h"
#include <stddef.h>

typedef struct PlayerState {
    u64 score;
    Fruit* fruits;
    size_t fruitsCount;
    size_t fruitsCapacity;
} PlayerState;

extern const size_t STATE_FRUIT_LIST_DEFAULT_SIZE;

bool state_init(PlayerState* state);
void state_resize(PlayerState* state); 
void state_clear(PlayerState* state);
void state_add_fruit(PlayerState* state, vec2 pos, FruitType fruitType);
bool state_remove_fruit(PlayerState* state, u16 id);
size_t state_index_of_fruit(PlayerState* state, FruitType fruitType);
size_t state_index_of_fruit_by_id(PlayerState* state, u16 id);
void state_merge_fruit(PlayerState* state, u16 id1, u16 id2);
bool state_fruit_is_full(PlayerState* state);