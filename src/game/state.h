#pragma once

#include <stddef.h>

#include "game/fruit.h"
#include "util/type.h"

enum {
    STATE_MERGE_EVENT_MAX = 16,
};

typedef struct MergeEvent {
    vec2      pos;
    FruitType type;
} MergeEvent;

typedef struct PlayerState {
    u64        score;
    f32        scoreShown;
    Fruit     *fruits;
    size_t     fruitsCount;
    size_t     fruitsCapacity;
    MergeEvent mergeEvents[STATE_MERGE_EVENT_MAX];
    size_t     mergeEventsCount;
} PlayerState;

bool state_init(PlayerState *state);
void state_free(PlayerState *state);

void state_add_fruit(PlayerState *state, vec2 pos, FruitType fruitType);
bool state_merge_touching(PlayerState *state);
