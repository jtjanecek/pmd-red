#include "global.h"
#include "debug_overlay.h"

#include "text_1.h"
#include "text_3.h"
#include "graphics_memory.h"
#include "bg_palette_buffer.h"
#include "gba/defines.h"
#include "cpu.h"

// Keep overlay state in EWRAM to match project patterns
EWRAM_DATA static bool8 sOverlayEnabled = TRUE;
EWRAM_DATA static char sOverlayText[64] = { 'T','E','S','T','\0' };
EWRAM_DATA static bool8 sObjInit = FALSE;

#define DBG_OBJ_PALNUM 15              // Use last OBJ palette row
#define DBG_OBJ_PAL_BASE ((0x10 + DBG_OBJ_PALNUM) * 16)
#define DBG_TILE_BASE 0x3F0            // Near end of OBJ VRAM, avoid engine usage

static inline void PutNibble(u8 *dst, s32 x, s32 y, u8 color)
{
    u32 idx = (y * 4) + (x >> 1);
    if ((x & 1) == 0) {
        dst[idx] = (dst[idx] & 0xF0) | (color & 0x0F);
    }
    else {
        dst[idx] = (dst[idx] & 0x0F) | ((color & 0x0F) << 4);
    }
}

static void BuildGlyph(u8 ch, u8 *out32)
{
    s32 x, y;
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
            PutNibble(out32, x, y, 0); // transparent

    // Simple 1px strokes using color index 1
    switch (ch) {
        case 'T':
        default:
            for (x = 0; x < 8; x++) PutNibble(out32, x, 0, 1);
            for (y = 0; y < 8; y++) PutNibble(out32, 3, y, 1);
            break;
        case 'E':
            for (y = 0; y < 8; y++) PutNibble(out32, 0, y, 1);
            for (x = 0; x < 8; x++) PutNibble(out32, x, 0, 1);
            for (x = 0; x < 6; x++) PutNibble(out32, x, 3, 1);
            for (x = 0; x < 8; x++) PutNibble(out32, x, 7, 1);
            break;
        case 'S':
            for (x = 0; x < 8; x++) PutNibble(out32, x, 0, 1);
            for (x = 0; x < 8; x++) PutNibble(out32, x, 7, 1);
            for (x = 0; x < 8; x++) PutNibble(out32, x, 3, 1);
            for (y = 1; y < 3; y++) PutNibble(out32, 0, y, 1);
            for (y = 4; y < 7; y++) PutNibble(out32, 7, y, 1);
            break;
    }
}

static void DebugOverlay_InitObjOnce(void)
{
    if (sObjInit)
        return;

    // Set a bright white color in OBJ palette row 15, color 1
    {
        RGB white = { .r = 0xFF, .g = 0xFF, .b = 0xFF };
        SetBGPaletteBufferColorArray(DBG_OBJ_PAL_BASE + 1, &white);
        TransferBGPaletteBuffer();
    }

    // Build and copy glyph tiles for "TEST" into OBJ VRAM near the end
    {
        u8 buf[32];
        const char *p = "TEST";
        s32 i;
        for (i = 0; p[i] != '\0'; i++) {
            u8 *vram = (u8 *)(OBJ_VRAM0 + ((DBG_TILE_BASE + i) * 32));
            s32 j;
            for (j = 0; j < 32; j++) buf[j] = 0;
            BuildGlyph(p[i], buf);
            CpuCopy(vram, buf, sizeof(buf));
        }
    }

    sObjInit = TRUE;
}

// Fallback: draw text directly on BG0 tilemap at (0,0) when no windows exist.
static void Overlay_DrawOnBg0TilemapTopLeft(void)
{
    s32 i;
    u16 tile0;
    u16 tile1;
    u8 c;
    // Top-left tile position (slight offset from absolute corner)
    s32 tx = 1;
    s32 ty = 1;

    for (i = 0; sOverlayText[i] != '\0' && i < 32; i++) {
        c = (u8)sOverlayText[i];
        if (c >= 'a' && c <= 'z')
            c -= 32; // uppercase
        // Map ASCII to font tiles starting at 0x258 as seen in text_3.c helpers
        tile0 = TILEMAP_PAL(15) | (0x258 + c);
        tile1 = TILEMAP_PAL(15) | (0x258 + c);
        // Write to both BG0 and BG1 so the text is visible even
        // when UI uses paired BG layers.
        sub_8008DC8(tx + i, ty, tile0, tile1);
    }
    // Ensure BG tilemaps get copied regardless of loop order.
    ScheduleBgTilemapCopy(0);
    ScheduleBgTilemapCopy(1);
    gUnknown_20274A5 = TRUE;
}

void DebugOverlay_SetEnabled(bool8 enabled)
{
    sOverlayEnabled = enabled ? TRUE : FALSE;
}

void DebugOverlay_SetText(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(sOverlayText, fmt, args);
    va_end(args);
}

void DebugOverlay_Draw(void)
{
    // Draw after normal UI work. Prefer the window text pipeline
    // when available; otherwise fall back to writing BG0 tiles so
    // the overlay is visible even when no windows are active.
    Window *w;
    s32 windowId;
    s32 x;
    s32 y;

    if (!sOverlayEnabled)
        return;

    // Ensure OBJ resources exist (for sprite fallback overlay)
    DebugOverlay_InitObjOnce();

    // Also draw in every active window so at least one instance is visible
    // regardless of which window(s) are on screen.
    for (windowId = 0; windowId < MAX_WINDOWS; windowId++) {
        w = &gWindows[windowId];
        if (w->width == 0 || w->unk8 == 0)
            continue;

        x = 0;
        y = 2; // small offset to avoid touching the window's border
        sub_80073B8(windowId);
        PrintStringOnWindow(x, y, (const u8 *)sOverlayText, windowId, 0);
        sub_80073E0(windowId);
    }

    // If no windows were active, fall back to BG tilemaps.
    {
        bool8 any = FALSE;
        s32 i;
        for (i = 0; i < MAX_WINDOWS; i++) {
            if (gWindows[i].width != 0 && gWindows[i].unk8 != 0) {
                any = TRUE;
                break;
            }
        }
        if (!any)
            Overlay_DrawOnBg0TilemapTopLeft();
    }

    // Finally, stamp 4 OBJ sprites spelling "TEST" at the very end of the
    // frame, writing directly to OAM so they appear above everything.
    // Small 8x8 glyphs, palette 15, tiles DBG_TILE_BASE..+3
    {
        vu16 *oam = (vu16 *)OAM; // start of OAM, we overwrite first 4 entries
        s32 i;
        s32 x = 4; // near top-left
        s32 y = 4;
        for (i = 0; i < 4; i++) {
            u16 attr0 = y;                 // y + 4bpp + square
            u16 attr1 = x & 0x1FF;         // x, size=0 (8x8)
            u16 attr2 = (DBG_OBJ_PALNUM << 12) | ((DBG_TILE_BASE + i) & 0x3FF);
            *oam++ = attr0;
            *oam++ = attr1;
            *oam++ = attr2;
            *oam++ = 0;
            x += 8; // next 8x8 to the right
        }
    }
}

void DebugOverlay_StampObjOam(void)
{
    if (!sOverlayEnabled)
        return;

    DebugOverlay_InitObjOnce();

    // Write four 8x8 OBJ entries spelling TEST at top-left
    {
        vu16 *oam = (vu16 *)OAM; // start of OAM
        s32 i;
        s32 x = 4;
        s32 y = 4;
        for (i = 0; i < 4; i++) {
            u16 attr0 = (u16)y;                 // y, 4bpp, square
            u16 attr1 = (u16)(x & 0x1FF);       // x, size 0
            u16 attr2 = (DBG_OBJ_PALNUM << 12) | ((DBG_TILE_BASE + i) & 0x3FF);
            *oam++ = attr0;
            *oam++ = attr1;
            *oam++ = attr2;
            *oam++ = 0;
            x += 8;
        }
    }
}
