#include "state.h"
#include "fruit.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const size_t STATE_FRUIT_LIST_DEFAULT_SIZE = 10;
static const size_t STATE_FRUIT_LIST_GROWTH = 2;
static const size_t STATE_INDEX_NONE = (size_t)-1;
static const f32 STATE_MERGE_DISTANCE = 1.0f;
static const int STATE_MERGE_MAX_PER_STEP = 64;

static bool state_allocate(PlayerState *state, size_t capacity) {
    Fruit *fruits = malloc(sizeof(Fruit) * capacity);

    if (fruits == NULL) {
        return false;
    }

    free(state->fruits);
    state->fruits = fruits;
    state->fruitsCapacity = capacity;

    return true;
}

bool state_init(PlayerState* state) {
    if (state == NULL) {
        return false;
    }

    state->fruits = NULL;
    state->fruitsCapacity = 0;
    state->fruitsCount = 0;
    state->score = 0;

    return state_allocate(state, STATE_FRUIT_LIST_DEFAULT_SIZE);
}
void state_resize(PlayerState* state) {
    if (!state_fruit_is_full(state)) {
        return;
    }

    size_t newCapacity = state->fruitsCapacity * STATE_FRUIT_LIST_GROWTH;

    if (newCapacity <= state->fruitsCapacity) {
        return;
    }

    Fruit *newFruits = malloc(sizeof(Fruit) * newCapacity);

    if (newFruits == NULL) {
        return;
    }

    if (state->fruitsCount > 0) {
        memcpy(newFruits, state->fruits, sizeof(Fruit) * state->fruitsCount);
    }

    free(state->fruits);
    state->fruits = newFruits;
    state->fruitsCapacity = newCapacity;
}
void state_clear(PlayerState* state) {
    if (state == NULL) {
        return;
    }

    state->fruitsCount = 0;
    state->score = 0;

    if (state->fruitsCapacity >= STATE_FRUIT_LIST_DEFAULT_SIZE) {
        return;
    }

    state_allocate(state, STATE_FRUIT_LIST_DEFAULT_SIZE);
}
void state_add_fruit(PlayerState* state, vec2 pos, FruitType fruitType) {
    if (state == NULL) {
        return;
    }

    if (state_fruit_is_full(state)) {
        state_resize(state);
    }

    if (state_fruit_is_full(state)) {
        return;
    }

    state->fruits[state->fruitsCount] = fruit_make(pos, fruitType);
    state->fruitsCount++;
}
bool state_remove_fruit(PlayerState* state, u16 id) {
    if (state == NULL) {
        return false;
    }

    size_t idx = state_index_of_fruit_by_id(state, id);

    if (idx == STATE_INDEX_NONE) {
        return false;
    }

    if (idx != state->fruitsCount - 1) {
        memmove(state->fruits + idx, state->fruits + idx + 1, sizeof(Fruit) * (state->fruitsCount - idx - 1));
    } 
    state->fruitsCount--;
    return true;
}

static u64 state_merge_score(FruitType type) {
    return (u64)(type + 1) * 10;
}

static void state_wake_around(PlayerState *state, vec2 pos, f32 radius) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];

        if (!fruit->sleeping) {
            continue;
        }

        f32 dx = fruit->pos.x - pos.x;
        f32 dy = fruit->pos.y - pos.y;
        f32 limit = radius + fruit->r + 2.0f;

        if ((dx * dx) + (dy * dy) <= limit * limit) {
            fruit->sleeping = false;
            fruit->sleepFrames = 0;
        }
    }
}

static bool state_fruits_touch(Fruit *a, Fruit *b) {
    f32 dx = b->pos.x - a->pos.x;
    f32 dy = b->pos.y - a->pos.y;
    f32 limit = a->r + b->r + STATE_MERGE_DISTANCE;

    return (dx * dx) + (dy * dy) <= limit * limit;
}

bool state_merge_touching(PlayerState *state) {
    if (state == NULL) {
        return false;
    }

    bool merged = false;

    for (int attempt = 0; attempt < STATE_MERGE_MAX_PER_STEP; ++attempt) {
        bool found = false;

        for (size_t i = 0; i < state->fruitsCount && !found; ++i) {
            for (size_t j = i + 1; j < state->fruitsCount; ++j) {
                Fruit *a = &state->fruits[i];
                Fruit *b = &state->fruits[j];

                if (a->type != b->type) {
                    continue;
                }
                if (a->type >= FRUIT_TYPE_WATERMELON) {
                    continue;
                }
                if (!state_fruits_touch(a, b)) {
                    continue;
                }

                state_merge_fruit(state, a->id, b->id);
                found = true;
                merged = true;
                break;
            }
        }

        if (!found) {
            break;
        }
    }

    return merged;
}
void state_merge_fruit(PlayerState* state, u16 id1, u16 id2) {
    if (state == NULL || id1 == id2) {
        return;
    }

    size_t idx1 = state_index_of_fruit_by_id(state, id1);
    size_t idx2 = state_index_of_fruit_by_id(state, id2);

    if (idx1 == STATE_INDEX_NONE || idx2 == STATE_INDEX_NONE) {
        return;
    }

    Fruit *a = &state->fruits[idx1];
    Fruit *b = &state->fruits[idx2];

    if (!fruit_can_combine(a, b)) {
        return;
    }

    if (a->type >= FRUIT_TYPE_WATERMELON) {
        return;
    }

    vec2 pos = { (a->pos.x + b->pos.x) * 0.5f, (a->pos.y + b->pos.y) * 0.5f };
    vec2 vel = { (a->vel.x + b->vel.x) * 0.5f, (a->vel.y + b->vel.y) * 0.5f };
    FruitType nextType = (FruitType)(a->type + 1);

    u16 survivorId = state->fruits[idx1].pos.y < state->fruits[idx2].pos.y ? id1 : id2;
    u16 removedId = survivorId == id1 ? id2 : id1;

    state_remove_fruit(state, removedId);

    size_t survivorIdx = state_index_of_fruit_by_id(state, survivorId);

    if (survivorIdx == STATE_INDEX_NONE) {
        return;
    }

    Fruit *survivor = &state->fruits[survivorIdx];

    survivor->pos = pos;
    survivor->vel = vel;
    survivor->type = nextType;
    survivor->r = FRUITS_INFOS[nextType].r * FRUIT_PIXELS_PER_UNIT;
    survivor->sleeping = false;
    survivor->sleepFrames = 0;

    state_wake_around(state, pos, survivor->r);

    state->score += state_merge_score(nextType);
}
size_t state_index_of_fruit(PlayerState *state, FruitType fruitType) {
    if (state == NULL) {
        return STATE_INDEX_NONE;
    }

    for (size_t i = 0; i < state->fruitsCount; i++) {
        if (fruit_equals_type(&state->fruits[i], fruitType)) {
            return i;
        }
    }
    return STATE_INDEX_NONE;
}
size_t state_index_of_fruit_by_id(PlayerState* state, u16 id) {
    if (state == NULL) {
        return STATE_INDEX_NONE;
    }

    for (size_t i = 0; i < state->fruitsCount; i++) {
        if (fruit_equals_id(&state->fruits[i], id)) {
            return i;
        }
    }
    return STATE_INDEX_NONE;
}
bool state_fruit_is_full(PlayerState *state) {
    return state->fruitsCount >= state->fruitsCapacity;
}
