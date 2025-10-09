#ifndef GUARD_DEBUG_OVERLAY_H
#define GUARD_DEBUG_OVERLAY_H

#include "global.h"

// Simple debugging overlay that draws text in the top-right corner
// over the current window contents. Implemented to be lightweight
// and non-intrusive to the existing UI system.

// Enable/disable the overlay globally.
void DebugOverlay_SetEnabled(bool8 enabled);

// Set the text to display (printf-style formatting supported).
void DebugOverlay_SetText(const char *fmt, ...);

// Draw the overlay for the current frame (called from the main frame loop).
void DebugOverlay_Draw(void);

// Stamps the OBJ overlay directly into OAM at the end of the OAM copy.
// Safe to call each frame after the engine has populated OAM.
void DebugOverlay_StampObjOam(void);

#endif // GUARD_DEBUG_OVERLAY_H
