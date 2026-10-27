#pragma once

#include "game/state.h"
#include "util/rect.h"

void collider_step(PlayerState *state, rect bounds, f32 dt);
