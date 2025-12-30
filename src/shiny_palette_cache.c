#include "global.h"
#include "shiny_palette_cache.h"
#include "bg_palette_buffer.h"

typedef struct ShinyPaletteCacheEntry
{
    const RGB *paletteData;
    const RGB *ramp;
    s32 brightness;
    u8 basePalette;
    u16 lastUsed;
} ShinyPaletteCacheEntry;

static EWRAM_DATA ShinyPaletteCacheEntry sShinyPaletteCache[SHINY_PALETTE_SLOT_COUNT] = {0};
static EWRAM_DATA u16 sShinyPaletteCacheCounter = 0;

static RGB MakeShinyColor(const RGB *src)
{
    RGB out;

    out.r = src->g;
    out.g = src->b;
    out.b = src->r;
    out.unk4 = src->unk4;
    return out;
}

void ResetShinyPaletteCache(void)
{
    s32 i;

    for (i = 0; i < SHINY_PALETTE_SLOT_COUNT; i++) {
        sShinyPaletteCache[i].paletteData = NULL;
        sShinyPaletteCache[i].ramp = NULL;
        sShinyPaletteCache[i].brightness = -1;
        sShinyPaletteCache[i].basePalette = 0xFF;
        sShinyPaletteCache[i].lastUsed = 0;
    }
    sShinyPaletteCacheCounter = 0;
}

u8 ShinyPaletteCache_GetSlot(const RGB *paletteData, u8 basePalette, s32 brightness, const RGB *ramp)
{
    s32 i;
    s32 slot = -1;
    u16 oldestTick = 0xFFFF;
    const RGB *paletteRow;
    u16 paletteIndex;

    if (paletteData == NULL) {
        return basePalette;
    }
    if (basePalette >= SHINY_PALETTE_SLOT_BASE) {
        return basePalette;
    }

    sShinyPaletteCacheCounter++;
    for (i = 0; i < SHINY_PALETTE_SLOT_COUNT; i++) {
        ShinyPaletteCacheEntry *entry = &sShinyPaletteCache[i];
        if (entry->paletteData == paletteData
            && entry->basePalette == basePalette
            && entry->brightness == brightness
            && entry->ramp == ramp) {
            entry->lastUsed = sShinyPaletteCacheCounter;
            return SHINY_PALETTE_SLOT_BASE + i;
        }
    }

    for (i = 0; i < SHINY_PALETTE_SLOT_COUNT; i++) {
        ShinyPaletteCacheEntry *entry = &sShinyPaletteCache[i];
        if (entry->paletteData == NULL || entry->basePalette == 0xFF) {
            slot = i;
            break;
        }
        if (entry->lastUsed < oldestTick) {
            oldestTick = entry->lastUsed;
            slot = i;
        }
    }

    paletteRow = paletteData + (basePalette * 16);
    paletteIndex = 0x100 + (SHINY_PALETTE_SLOT_BASE + slot) * 16;
    for (i = 0; i < 16; i++) {
        RGB shiny = MakeShinyColor(&paletteRow[i]);

        SetBGPaletteBufferColorRGB(paletteIndex + i, &shiny, brightness, ramp);
    }

    sShinyPaletteCache[slot].paletteData = paletteData;
    sShinyPaletteCache[slot].ramp = ramp;
    sShinyPaletteCache[slot].brightness = brightness;
    sShinyPaletteCache[slot].basePalette = basePalette;
    sShinyPaletteCache[slot].lastUsed = sShinyPaletteCacheCounter;

    return SHINY_PALETTE_SLOT_BASE + slot;
}
