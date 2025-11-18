# Text Buffer Investigation Results

## Summary

Investigated the 24 KB of text/window static EWRAM to determine what can be safely reduced.

---

## Findings

### 1. Window Tile Buffer: `sUnknown_20274B4[0xEC0]`

**Location:** `src/text_1.c:24`
**Current Size:** 3,776 elements × 4 bytes = **15,104 bytes (14.8 KB)**
**Purpose:** Shared tile buffer for all window rendering

**How it works:**
- Passed to `AddWindow()` function as parameter `a2`
- Each window allocates space: `&a2[windowOffset * 8]`
- Stores tile data for up to 4 windows (MAX_WINDOWS)
- Window offset accumulates based on window dimensions

**Reduction Analysis:**

| Size | Elements | Bytes | Savings | Risk | Notes |
|------|----------|-------|---------|------|-------|
| Current | 0xEC0 (3,776) | 15,104 B | - | - | Original size |
| Conservative | 0x800 (2,048) | 8,192 B | **6.9 KB** | Medium | Enough for 4 windows @ 30x20 tiles |
| Aggressive | 0x600 (1,536) | 6,144 B | **8.8 KB** | High | Tight fit, may overflow |

**Worst Case Calculation:**
- Screen: 240×160 pixels = 30×20 tiles (8×8 each)
- 4 windows @ 30×20 tiles = 2,400 tiles
- With 50% overhead: 3,600 elements (still under current 3,776)

**Recommendation:**
- **Try 0x800 first** (saves 6.9 KB, medium risk)
- If that works, test 0x600 (saves 8.8 KB, higher risk)
- Add overflow detection logging to catch issues early

**Code Location:**
```c
// src/text_1.c:24
EWRAM_DATA static u32 sUnknown_20274B4[0xEC0] = {0};
```

---

### 2. Format Buffers: Monster/Name Arrays

**Location:** `src/string_format.c:68-70`
**Current Size:**
- `gFormatBuffer_Monsters[10][80]` = 800 bytes
- `gFormatBuffer_Names[10][80]` = 800 bytes
- **Total: 1,600 bytes (1.6 KB)**

**Purpose:**
- Store formatted strings for displaying monster names/stats
- Indexed m0-m9 and n0-n9 in dialogue scripts
- Comment says "apparently only i0 and i1 are actually used" for items

**Reduction Analysis:**

| Array | Current | Reduced | Savings | Justification |
|-------|---------|---------|---------|---------------|
| `gFormatBuffer_Monsters` | [10][80] | [4][80] | 480 B | Roguelike only needs active party (~4 max) |
| `gFormatBuffer_Names` | [10][80] | [4][80] | 480 B | Same reasoning |
| **Total** | | | **960 B** | Low risk - roguelike doesn't need 10 simultaneous names |

**Additional Observations:**
- `gFormatBuffer_Items[4]` - Already optimized, keep as-is
- `sFormatBuffer_TeamName[80]` - Small, keep as-is
- Comment suggests items only use slots 0-1, but array is already [4]

**Recommendation:**
- ✅ **Safe to reduce both to [4]** (saves ~1 KB)
- Low risk: Town Square had many NPCs (needed 10), roguelike has small party
- Test: Ensure all dialogue/menus work with 4 slots

**Code Locations:**
```c
// src/string_format.c:68-70
EWRAM_DATA u8 gFormatBuffer_Monsters[10][FORMAT_BUFFER_LEN] = {0};
EWRAM_DATA u8 gFormatBuffer_Names[10][FORMAT_BUFFER_LEN] = {0};

// Also update header: include/string_format.h:15-16
extern u8 gFormatBuffer_Monsters[10][FORMAT_BUFFER_LEN];
extern u8 gFormatBuffer_Names[10][FORMAT_BUFFER_LEN];
```

---

### 3. Other Text Buffers (Investigated but NOT Reducing)

| Buffer | Size | Purpose | Keep? |
|--------|------|---------|-------|
| `gWindows[4]` | 288 B (4×72) | Window state structs | ✅ Keep - need all 4 |
| `gBgTilemaps[4][32][32]` | 8,192 B | Background tilemaps | ✅ Keep - hardware requirement |
| `sDialogueTextBuffer[1000]` | 1,000 B | Current dialogue text | ✅ Keep - needed for long text |
| `gFormatBuffer_FriendArea[184]` | 184 B | Friend area names | ✅ Keep - small |

---

## Recommended Implementation Plan

### Phase 1: Format Buffers (Low Risk, ~1 KB)
1. Reduce `gFormatBuffer_Monsters[10]` → `[4]`
2. Reduce `gFormatBuffer_Names[10]` → `[4]`
3. Update header file declarations
4. Test all dialogue/menus

**Expected savings:** ~960 bytes
**Risk:** Low
**Effort:** 30 minutes

### Phase 2: Window Tile Buffer (Medium Risk, ~7 KB)
1. Add logging to detect max buffer usage
2. Reduce `sUnknown_20274B4[0xEC0]` → `[0x800]`
3. Test all windows (title, menus, dungeons, dialogue)
4. If no issues, consider further reduction to `[0x600]`

**Expected savings:** 6.9 KB (conservative) to 8.8 KB (aggressive)
**Risk:** Medium (buffer overflow possible)
**Effort:** 2-3 hours with testing

---

## Total Potential Savings

| Reduction | Savings | Risk | Recommendation |
|-----------|---------|------|----------------|
| Format buffers | ~1 KB | Low | ✅ Do immediately |
| Window buffer (0x800) | ~7 KB | Medium | ✅ Do with testing |
| Window buffer (0x600) | ~9 KB | High | ⚠️ Only if desperate |
| **Total (conservative)** | **~8 KB** | Low-Med | **Recommended path** |

---

## Testing Checklist

After reductions, test:
- [ ] Title screen displays correctly
- [ ] All menus (Start menu, dungeon menu)
- [ ] Dialogue boxes (NPCs, narrator)
- [ ] Multiple simultaneous windows
- [ ] Long text passages
- [ ] Monster/item names in menus
- [ ] Team names
- [ ] Status displays

---

## Alternative: Delete Ground Script EWRAM (~1 KB)

From `ground_script.o(ewram_data)`: 890 bytes of Town Square orphaned data

**Files to check:**
- `src/ground_script.c:115-121` - Script lock arrays
- These are only used when Ground mode is active
- Since we removed Town Square, less Ground mode usage
- Could reduce `SCRIPT_LOCKS_ARR_COUNT` or delete entirely

**Recommendation:** Investigate if these can be deleted entirely (Option 3)

---

## Next Steps

1. ✅ Phase 1: Reduce format buffers (30 min, low risk)
2. ✅ Phase 2: Add buffer overflow detection, then reduce window buffer (2-3 hours)
3. ✅ Option 3: Investigate deleting Ground script orphaned data (~1 KB)

**Total expected savings: 8-10 KB** (enough to get under EWRAM limit!)
