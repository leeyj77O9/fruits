#include "state.h"
#include "fruit.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const size_t STATE_FRUIT_LIST_DEFAULT_SIZE = 10;
/* 배열이 가득 찼을 때 늘릴 배수 */
static const size_t STATE_FRUIT_LIST_GROWTH = 2;
/* 찾지 못했을 때 반환하는 값 */
static const size_t STATE_INDEX_NONE = (size_t)-1;
/* 두 과일 중심 거리가 반지름 합보다 이만큼 안이면 닿은 것으로 본다 */
static const float STATE_MERGE_DISTANCE = 1.0f;
/* 한 스텝에 연쇄 병합이 무한히 이어지지 않도록 제한 */
static const int STATE_MERGE_MAX_PER_STEP = 64;

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

/* 병합으로 얻는 점수. 단계가 높을수록 크다 */
static u64 state_merge_score(FruitType type) {
    return (u64)(type + 1) * 10;
}

/* 주어진 위치 주변에서 자고 있는 과일을 깨운다 */
static void state_wake_around(PlayerState *state, vec2 pos, float radius) {
    for (size_t i = 0; i < state->fruitsCount; ++i) {
        Fruit *fruit = &state->fruits[i];

        if (!fruit->sleeping) {
            continue;
        }

        float dx = fruit->pos.x - pos.x;
        float dy = fruit->pos.y - pos.y;
        float limit = radius + fruit->r + 2.0f;

        if ((dx * dx) + (dy * dy) <= limit * limit) {
            fruit->sleeping = false;
            fruit->sleepFrames = 0;
        }
    }
}

/* 두 과일이 실제로 닿아 있는지 (솔버가 남기는 미세한 겹침은 허용) */
static bool state_fruits_touch(Fruit *a, Fruit *b) {
    float dx = b->pos.x - a->pos.x;
    float dy = b->pos.y - a->pos.y;
    float limit = a->r + b->r + STATE_MERGE_DISTANCE;

    return (dx * dx) + (dy * dy) <= limit * limit;
}

bool state_merge_touching(PlayerState *state) {
    if (state == NULL) {
        return false;
    }

    bool merged = false;

    /* 연쇄 병합(A+B가 C가 되고 C가 D와 합쳐짐)을 허용하되 무한 반복은 막는다 */
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
    /* 수박은 더 큰 단계가 없으므로 합치지 않는다 */
    if (a->type >= FRUIT_TYPE_WATERMELON) {
        return;
    }

    /* 합쳐진 과일은 두 과일의 한가운데에서 태어난다 */
    vec2 pos = { (a->pos.x + b->pos.x) * 0.5f, (a->pos.y + b->pos.y) * 0.5f };
    vec2 vel = { (a->vel.x + b->vel.x) * 0.5f, (a->vel.y + b->vel.y) * 0.5f };
    FruitType nextType = (FruitType)(a->type + 1);

    /* 합쳐질 자리는 더 위(작은 y)에 있는 과일로 정한다 */
    u16 survivorId = state->fruits[idx1].pos.y < state->fruits[idx2].pos.y ? id1 : id2;
    u16 removedId = survivorId == id1 ? id2 : id1;

    state_remove_fruit(state, removedId);

    /* 제거로 배열이 당겨져 인덱스가 바뀌었을 수 있으므로 id로 다시 찾는다 */
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

    /* 커진 과일과 겹치는 이웃은 깨워야 한다.
       자고 있는 과일은 무한 질량으로 취급되므로 그대로 두면 끼어서 못 움직인다 */
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
