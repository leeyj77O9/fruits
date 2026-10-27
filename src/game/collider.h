#pragma once

#include <stdbool.h>
#include "fruit.h"
#include "state.h"
#include "../util/rect.h"

extern const f32 GRAVITY;       
extern const f32 RESTITUTION;    
extern const f32 FLOOR_FRICTION;
extern const f32 PAIR_FRICTION;
extern const f32 POSITION_SLOP;

bool collider_overlap(Fruit *a, Fruit *b);      
bool collider_overlap_wall(Fruit *f, rect bounds);

void collider_resolve_pair(Fruit *a, Fruit *b); 
void collider_resolve_wall(Fruit *f, rect bounds);

void collider_step(PlayerState *state, rect bounds, f32 dt);