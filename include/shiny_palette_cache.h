#ifndef GUARD_SHINY_PALETTE_CACHE_H
#define GUARD_SHINY_PALETTE_CACHE_H

#include "structs/rgb.h"

#define SHINY_PALETTE_SLOT_BASE 13
#define SHINY_PALETTE_SLOT_COUNT 3
void ResetShinyPaletteCache(void);
u8 ShinyPaletteCache_GetSlot(const RGB *paletteData, u8 basePalette, s32 brightness, const RGB *ramp);

#endif // GUARD_SHINY_PALETTE_CACHE_H
