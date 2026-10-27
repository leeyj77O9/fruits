#pragma once
#include "type.h"

typedef struct rgba {
    u8 r; u8 g; u8 b; u8 a;
} rgba;

#define RGBA_TRANSPARENT ((rgba){0, 0, 0, 0})

#define RGBA_BLACK ((rgba){0, 0, 0, 255})
#define RGBA_WHITE ((rgba){255, 255, 255, 255})

#define RGBA_RED ((rgba){220, 60, 60, 255})
#define RGBA_ORANGE ((rgba){240, 140, 30, 255})
#define RGBA_YELLOW ((rgba){240, 220, 60, 255})
#define RGBA_GREEN ((rgba){90, 180, 70, 255})
#define RGBA_DARK_GREEN ((rgba){40, 110, 60, 255})
#define RGBA_YELLOW_GREEN ((rgba){180, 200, 70, 255})
#define RGBA_PURPLE ((rgba){130, 70, 160, 255})
#define RGBA_PINK ((rgba){240, 140, 170, 255})
#define RGBA_BROWN ((rgba){140, 90, 50, 255})
