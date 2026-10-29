#include "state.h"
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

static bool state_fruit_is_full(const PlayerState *state) {
    return state->fruitsCount >= state->fruitsCapacity;
}

static void state_resize(PlayerState *state) {
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

static size_t state_index_of_fruit_by_id(const PlayerState *state, u32 id) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        if (fruit_equals_id(&state->fruits[i], id)) {
            return i;
        }
    }

    return STATE_INDEX_NONE;
}

static bool state_remove_fruit(PlayerState *state, u32 id) {
    size_t index = state_index_of_fruit_by_id(state, id);

    if (index == STATE_INDEX_NONE) {
        return false;
    }

    if (index != state->fruitsCount - 1) {
        memmove(state->fruits + index, state->fruits + index + 1,
                sizeof(Fruit) * (state->fruitsCount - index - 1));
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

static bool state_fruits_touch(const Fruit *a, const Fruit *b) {
    f32 dx = b->pos.x - a->pos.x;
    f32 dy = b->pos.y - a->pos.y;
    f32 limit = a->r + b->r + STATE_MERGE_DISTANCE;

    return (dx * dx) + (dy * dy) <= limit * limit;
}

static void state_merge_fruit(PlayerState *state, u32 id1, u32 id2) {
    if (id1 == id2) {
        return;
    }

    size_t index1 = state_index_of_fruit_by_id(state, id1);
    size_t index2 = state_index_of_fruit_by_id(state, id2);

    if (index1 == STATE_INDEX_NONE || index2 == STATE_INDEX_NONE) {
        return;
    }

    Fruit *a = &state->fruits[index1];
    Fruit *b = &state->fruits[index2];

    if (!fruit_can_combine(a, b)) {
        return;
    }

    vec2 pos = vec2_scale(vec2_add(a->pos, b->pos), 0.5f);
    vec2 vel = vec2_scale(vec2_add(a->vel, b->vel), 0.5f);
    FruitType nextType = (FruitType)(a->type + 1);

    u32 survivorId = a->pos.y < b->pos.y ? id1 : id2;
    u32 removedId = survivorId == id1 ? id2 : id1;

    if (!state_remove_fruit(state, removedId)) {
        return;
    }

    size_t survivorIndex = state_index_of_fruit_by_id(state, survivorId);

    if (survivorIndex == STATE_INDEX_NONE) {
        return;
    }

    Fruit *survivor = &state->fruits[survivorIndex];

    survivor->pos = pos;
    survivor->vel = vel;
    survivor->type = nextType;
    survivor->r = fruit_radius(nextType);
    survivor->sleeping = false;
    survivor->sleepFrames = 0;

    state_wake_around(state, pos, survivor->r);

    state->score += state_merge_score(nextType);

    if (state->mergeEventsCount < STATE_MERGE_EVENT_MAX) {
        state->mergeEvents[state->mergeEventsCount++] = (MergeEvent){pos, nextType};
    }
}

bool state_init(PlayerState *state) {
    if (state == NULL) {
        return false;
    }

    state->fruits = NULL;
    state->fruitsCapacity = 0;
    state->fruitsCount = 0;
    state->score = 0;
    state->scoreShown = 0.0f;
    state->mergeEventsCount = 0;

    return state_allocate(state, STATE_FRUIT_LIST_DEFAULT_SIZE);
}

void state_free(PlayerState *state) {
    if (state == NULL) {
        return;
    }

    free(state->fruits);
    state->fruits = NULL;
    state->fruitsCapacity = 0;
    state->fruitsCount = 0;
    state->score = 0;
}

void state_add_fruit(PlayerState *state, vec2 pos, FruitType fruitType) {
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

                if (!fruit_can_combine(a, b) || !state_fruits_touch(a, b)) {
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
