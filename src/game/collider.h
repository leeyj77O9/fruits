#pragma once

#include <stdbool.h>
#include "fruit.h"
#include "state.h"
#include "../util/rect.h"

extern const float GRAVITY;       
extern const float RESTITUTION;    
extern const float FLOOR_FRICTION;
extern const float PAIR_FRICTION;
extern const float POSITION_SLOP;

bool collider_overlap(Fruit *a, Fruit *b);      
bool collider_overlap_wall(Fruit *f, rect bounds);

void collider_resolve_pair(Fruit *a, Fruit *b); 
void collider_resolve_wall(Fruit *f, rect bounds);

void collider_step(PlayerState *state, rect bounds, float dt);