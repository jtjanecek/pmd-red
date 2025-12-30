#ifndef GUARD_SHINY_PALETTE_TABLE_H
#define GUARD_SHINY_PALETTE_TABLE_H

#include "global.h"
#include "constants/monster.h"

extern const u8 gMonsterShinyPalette[MONSTER_MAX];

u8 GetMonsterShinyPalette(s16 species);

#endif // GUARD_SHINY_PALETTE_TABLE_H
