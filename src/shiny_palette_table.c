#include "global.h"
#include "constants/monster.h"
#include "shiny_palette_table.h"

u8 GetMonsterShinyPalette(s16 species)
{
    if (species < 0 || species >= MONSTER_MAX) {
        return 0;
    }
    return gMonsterShinyPalette[species];
}
