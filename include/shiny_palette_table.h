#ifndef GUARD_SHINY_PALETTE_TABLE_H
#define GUARD_SHINY_PALETTE_TABLE_H

#include "global.h"
#include "constants/monster.h"

#define SHINY_PALETTE_NONE 0xFF

extern const u8 gMonsterShinyPalette[MONSTER_MAX];
extern const bool8 gMonsterShinyIndexRemapActive[MONSTER_MAX];
extern const u8 gMonsterShinyIndexRemap[MONSTER_MAX][16];

u8 GetMonsterShinyPalette(s16 species);
const u8 *GetMonsterShinyIndexRemap(s16 species);

#endif // GUARD_SHINY_PALETTE_TABLE_H
