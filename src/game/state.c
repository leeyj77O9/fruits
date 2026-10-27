#include "state.h"
#include "fruit.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const size_t STATE_FRUIT_LIST_DEFAULT_SIZE = 10;

bool state_init(PlayerState* state) {
    if (state == NULL) {
        return false;
    }

    u32 cap = sizeof(Fruit) * STATE_FRUIT_LIST_DEFAULT_SIZE;
    state->fruits = malloc(cap);
    state->fruitsCapacity = cap;
    state->fruitsCount = 0;
    state->score = 0;

    return true;
}
void state_resize(PlayerState* state) {
    if (!state_fruit_is_full(state)) {
        return;
    }

    u32 newCap = state->fruitsCapacity * 9 / 4;
    Fruit* newFruits = (Fruit*)malloc(newCap);
    memcpy(newFruits, state->fruits, state->fruitsCapacity);
    free(state->fruits);
    state->fruits = newFruits;
    state->fruitsCapacity = newCap;
}
void state_clear(PlayerState* state) {
    free(state->fruits);

    u32 cap = sizeof(Fruit) * STATE_FRUIT_LIST_DEFAULT_SIZE;
    state->fruits = malloc(cap);
    state->fruitsCapacity = cap;
    state->fruitsCount = 0;
    state->score = 0;
}
void state_add_fruit(PlayerState* state, vec2 pos, FruitType fruitType) {
    if (state == NULL) {
        return;
    }
    
    if (state_fruit_is_full(state)) {
        state_resize(state);
    } 

    Fruit fruit = fruit_make(pos, fruitType);
    state->fruits[state->fruitsCount] = fruit;
    state->fruitsCount++;
}
bool state_remove_fruit(PlayerState* state, u16 id) {
    size_t idx = state_index_of_fruit_by_id(state, id);

    if (idx == -1) {
        return false;
    }

    if (idx != state->fruitsCount - 1) {
        memcpy(state->fruits + idx, state->fruits + idx + 1, sizeof(Fruit) * (state->fruitsCount - idx - 1));
    } 
    state->fruitsCount--;
    return true;
}
void state_merge_fruit(PlayerState* state, u16 id1, u16 id2) {
    size_t idx1 = state_index_of_fruit_by_id(state, id1);
    size_t idx2 = state_index_of_fruit_by_id(state, id2);
    size_t idx = state->fruits[idx1].pos.y < state->fruits[idx2].pos.y ? idx1 : idx2;

    if (idx1 == idx) {
        state_remove_fruit(state, idx2);
    } else {
        state_remove_fruit(state, idx1);
    }

    fruit_upgrade(&state->fruits[idx]);
}
size_t state_index_of_fruit(PlayerState *state, FruitType fruitType) {
    for (size_t i = 0; i < state->fruitsCount; i++) {
        if (fruit_equals_type(&state->fruits[i], fruitType)) {
            return i;
        }
    }
    return -1;
}
size_t state_index_of_fruit_by_id(PlayerState* state, u16 id) {
    for (size_t i = 0; i < state->fruitsCount; i++) {
        if (fruit_equals_id(&state->fruits[i], id)) {
            return i;
        }
    }
    return -1;
}
bool state_fruit_is_full(PlayerState *state) {
    return sizeof(Fruit) * state->fruitsCount == state->fruitsCapacity;
}