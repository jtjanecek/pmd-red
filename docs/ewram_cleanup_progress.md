# EWRAM Cleanup Progress

## Current Status

**Build Output:**
```
EWRAM: 261,324 B / 256 KB (99.69% used)
Free: ~786 B (OVER LIMIT by ~5 KB)
```

**Heap Size:** `0x24000` (147,456 bytes / 144 KB)

---

## What We've Done

### 1. Reduced Ground Pool Sizes (Heap Allocations)
These don't show in static EWRAM%, but free heap space at runtime:

| Pool | Old Size | New Size | Heap Savings |
|------|----------|----------|--------------|
| `GroundLives` (NPCs) | 24 slots | 4 slots | ~9.7 KB |
| `GroundObjects` | 16 objects | 4 objects | ~5.4 KB |
| `GroundEffects` | 8 effects | 2 effects | ~2.7 KB |
| `GroundEvents` | 16 events | 4 events | ~0.4 KB |
| **Total Heap Freed** | | | **~18.2 KB** |

### 2. Freed Effect Context Before Dungeon Allocation
- Temporarily frees 6.7 KB from heap before allocating 118 KB dungeon struct
- Allows dungeon allocation to succeed despite limited heap space

### 3. Deleted Unused Static EWRAM Variables

| Variable | File | Bytes Saved |
|----------|------|-------------|
| `sOAMSpriteCount` | `src/sprite.c:9` | 2 B |
| `sUnknownUnused` | `src/music_util.c:7` | 4 B |
| `sUnusedCounter` | `src/music_util.c:10` | 2 B |
| `sUnusedScrambledInputJunk[3]` | `src/input.c:11` | 12 B |
| **Total Static EWRAM Freed** | | **20 B** |

---

## The Problem

With `HEAP_SIZE = 0x24000` (144 KB):
- **Static EWRAM:** 261,324 bytes (99.69%)
- **Over limit by:** ~5.3 KB

With `HEAP_SIZE = 0x22000` (139 KB):
- **Static EWRAM:** 253,148 bytes (96.57%)
- **Free EWRAM:** ~9 KB
- **But:** Dungeon allocation fails due to insufficient heap space

**We're caught between:**
- ❌ Smaller heap (0x22000) = fits EWRAM but dungeon won't allocate
- ❌ Larger heap (0x24000) = dungeon allocates but exceeds EWRAM limit

---

## Options Moving Forward

### Option A: Delete More Static EWRAM (Recommended)
Need to free **~6-10 KB** to stay under EWRAM limit with larger heap.

**High-Value Targets:**

| Target | Location | Size | Difficulty | Risk |
|--------|----------|------|------------|------|
| `sUnknown_20274B4[0xEC0]` | `src/text_1.c:24` | 15 KB | Hard | High |
| `sLevelCurrentData[0x64]` | `src/pokemon.c:27` | ~1.5 KB | Medium | Medium |
| `gFormatBuffer_Monsters[10]` | `src/string_format.c:68` | Can reduce to [4] | Easy | Low |
| `gFormatBuffer_Names[10]` | `src/string_format.c:70` | Can reduce to [4] | Easy | Low |
| Format buffers (shrink size) | `src/string_format.c` | ~2-4 KB | Medium | Medium |

**Quick Win:** Reduce format buffer counts from 10 to 4 (roguelike only needs active party):
- `gFormatBuffer_Monsters[10]` → `[4]` (saves ~960 bytes if FORMAT_BUFFER_LEN=160)
- `gFormatBuffer_Names[10]` → `[4]` (saves ~960 bytes)
- **Total:** ~2 KB with minimal risk

### Option B: Keep Smaller Heap + Find More Heap Savings
Stay at `HEAP_SIZE = 0x22000` but need to free MORE heap space at runtime:

**Already freed:**
- Ground pools: ~18 KB
- Effect context: ~7 KB
- **Total: ~25 KB**

**Still not enough!** Would need to find what else is allocated in the heap when dungeon loads.

### Option C: Hybrid - Reduce Heap Slightly
- Set `HEAP_SIZE = 0x23000` (143 KB, middle ground)
- Saves 4 KB static EWRAM vs 0x24000
- **New EWRAM:** ~257 KB (still ~1 KB over limit)
- Still need to delete ~2 KB static data

---

## Recommended Next Steps

**Phase 1: Quick Format Buffer Reduction** (2 hours, ~2 KB savings)
1. Reduce `gFormatBuffer_Monsters[10]` → `[4]`
2. Reduce `gFormatBuffer_Names[10]` → `[4]`
3. Test all menus/dialogue to ensure 4 buffers is enough

**Phase 2: Investigate sUnknown_20274B4** (4-6 hours, potential 5-10 KB savings)
1. Understand what this 15 KB buffer does
2. Determine if it can be reduced (might be tile cache for MAX_WINDOWS)
3. Calculate minimum safe size
4. Test window rendering with smaller buffer

**Expected Outcome After Phase 1:**
```
EWRAM: ~259 KB / 256 KB (97.7%)
Free: ~3 KB (still over by ~3 KB)
```

**Expected Outcome After Phase 1 + 2:**
```
EWRAM: ~252 KB / 256 KB (95.8%)
Free: ~10 KB
Heap: 144 KB (enough for dungeon)
Status: ✅ Under limit with working dungeon allocation
```

---

## Testing Required

Before shipping any EWRAM reductions:
1. ✅ Boot to title screen
2. ✅ Start new game
3. ✅ Enter first dungeon (Tiny Woods)
4. Test all menus (Start menu, dungeon menu, shops)
5. Test dialogue boxes
6. Test multiple simultaneous windows (if any)

---

## Files Modified

### Heap/Pool Reductions:
- `src/memory.c` - HEAP_SIZE reverted to 0x24000
- `src/ground_effect.c` - NUM_GROUND_EFFECTS: 8 → 2
- `src/ground_event.c` - NUM_GROUND_EVENTS: 16 → 4
- `src/ground_object.c` - NUM_GROUND_OBJECTS: 16 → 4
- `src/ground_lives.c` - UNK_3001B84_ARR_COUNT: 24 → 4
- `src/main_loops.c` - Free effect context before dungeon allocation

### Static EWRAM Deletions:
- `src/sprite.c` - Deleted `sOAMSpriteCount`
- `src/music_util.c` - Deleted `sUnknownUnused` and `sUnusedCounter`
- `src/input.c` - Deleted `sUnusedScrambledInputJunk`

---

## Build/Test Command

```bash
make && mgba-qt -l 1 pmd_red.gba > execution.log 2>&1
```

Then try entering Tiny Woods dungeon to test allocation.
