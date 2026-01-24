#include "global.h"
#include "constants/monster.h"
#include "shiny_palette_table.h"

u8 GetMonsterShinyPalette(s16 species)
{
    if (species < 0 || species >= MONSTER_MAX) {
        return SHINY_PALETTE_NONE;
    }
    return gMonsterShinyPalette[species];
}

const u8 *GetMonsterShinyIndexRemap(s16 species)
{
    if (species < 0 || species >= MONSTER_MAX) {
        return NULL;
    }
    if (!gMonsterShinyIndexRemapActive[species]) {
        return NULL;
    }
    return gMonsterShinyIndexRemap[species];
}
