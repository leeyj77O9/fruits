#include "state.h"
#include "fruit.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const size_t STATE_FRUIT_LIST_DEFAULT_SIZE = 10;
/* 배열이 가득 찼을 때 늘릴 배수 */
static const size_t STATE_FRUIT_LIST_GROWTH = 2;
/* 찾지 못했을 때 반환하는 값 */
static const size_t STATE_INDEX_NONE = (size_t)-1;

/* fruitsCapacity는 "과일 개수" 단위다. 바이트 단위로 섞으면 용량 계산이 깨진다 */
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

    /* 메모리가 부족해 늘리지 못했다면 배열을 넘어 쓰지 않고 포기한다 */
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
        /* 앞으로 당기는 복사는 원본과 대상이 겹치므로 memmove를 써야 한다 */
        memmove(state->fruits + idx, state->fruits + idx + 1, sizeof(Fruit) * (state->fruitsCount - idx - 1));
    } 
    state->fruitsCount--;
    return true;
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

    /* 합쳐질 자리는 더 위(작은 y)에 있는 과일로 정한다 */
    u16 survivorId = state->fruits[idx1].pos.y < state->fruits[idx2].pos.y ? id1 : id2;
    u16 removedId = survivorId == id1 ? id2 : id1;

    state_remove_fruit(state, removedId);

    /* 제거로 배열이 당겨져 인덱스가 바뀌었을 수 있으므로 id로 다시 찾는다 */
    size_t survivorIdx = state_index_of_fruit_by_id(state, survivorId);

    if (survivorIdx == STATE_INDEX_NONE) {
        return;
    }

    fruit_upgrade(&state->fruits[survivorIdx]);
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
