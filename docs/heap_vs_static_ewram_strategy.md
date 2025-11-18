# Heap vs Static EWRAM Reduction Strategy

## Current Situation

After removing Town Square logic, we have:
- **Static EWRAM usage**: 253,148 / 262,144 bytes (96.57%)
- **Free static EWRAM**: ~9 KB
- **Heap size**: 139,264 bytes (`HEAP_SIZE = 0x22000`)
- **Problem**: Dungeon allocation (118 KB) fails due to insufficient contiguous heap space

## The Two Approaches

### Option A: Reduce HEAP_SIZE
Shrink the heap to free up static EWRAM for other uses.

#### Pros
✅ **Simple**: One `#define` change in `src/memory.c:7`
✅ **Immediate EWRAM savings**: Shows up in build output
✅ **Makes room for future features**: Free EWRAM can be used for new static structures

#### Cons
❌ **Runtime allocation failures**: If we shrink too much, `MemoryAlloc()` fails → black screens
❌ **Hard to measure peak usage**: Different code paths have different heap requirements
❌ **Difficult to debug**: Failures only appear when specific scenarios trigger peak allocations
❌ **Game-state dependent**: Title screen, Ground mode, and Dungeon mode all use heap differently
❌ **Requires extensive testing**: Need to test every dungeon, every scenario, every menu combination

**Risk Assessment: HIGH** ⚠️
As you noted, this could cause freezing in untested scenarios. A dungeon that works today might crash tomorrow if a different code path allocates more.

---

### Option B: Eliminate Unused Static EWRAM
Remove or shrink static arrays/structures that Town Square needed but the roguelike doesn't.

#### Pros
✅ **Permanent and safe**: Once removed, it's gone forever (shows in build)
✅ **Deterministic**: If it compiles and links, it works
✅ **No runtime risk**: Can't cause allocation failures
✅ **Measurable progress**: Each reduction shows immediately in build output
✅ **Self-documenting**: Removing unused code makes the codebase cleaner

#### Cons
❌ **More work**: Need to identify, verify, and remove each unused structure
❌ **Requires code analysis**: Must ensure nothing still references deleted data
❌ **Multiple files**: Changes spread across many source files
❌ **Incremental**: Can't reclaim all EWRAM at once

**Risk Assessment: LOW** ✅
If the game compiles, links, and the deleted data truly was unused, it will work reliably.

---

## Recommended Strategy: **Option B** (Eliminate Static EWRAM)

### Why This Makes Sense

1. **Town Square is gone**: We've already removed the logic, so the data structures are orphaned
2. **Heap is opaque**: We can't easily see what's allocated at runtime without extensive instrumentation
3. **Static is transparent**: Build output tells us exactly how much we've saved
4. **Roguelike needs are simpler**: No town NPCs, fewer windows, smaller format buffers
5. **Future-proof**: If we later add features, we know exactly how much EWRAM we have

### High-Value Targets (from `docs/ewram-reduction.md`)

| Target | Current Size | Reduction Potential | Difficulty | File |
|--------|--------------|---------------------|------------|------|
| **Text/Window buffers** | ~23 KB | ~10-15 KB | Medium | `src/text_1.c`, `src/string_format.c` |
| - `gWindows[MAX_WINDOWS]` | Variable | Reduce `MAX_WINDOWS` from 128 to 32 | Easy | `src/text_1.c:16` |
| - `gBgTilemaps[4][32][32]` | 8 KB | Can't reduce (needed for display) | N/A | `src/text_1.c:33` |
| - `sUnknown_20274B4[0xEC0]` | 15 KB | Purpose unknown, investigate | Hard | `src/text_1.c:24` |
| - Format buffers | Multiple | Shrink monster/item name arrays | Medium | `src/string_format.c:66-74` |
| **Recruited Pokémon storage** | ~38 KB | ~20-30 KB | Hard | `src/pokemon.c:18-34` |
| - If roguelike doesn't need full rosters, could reduce dramatically | | | |
| **Sprite pools** | ~6 KB | ~2-3 KB | Medium | `src/sprite.c:15-20` |
| - With fewer town NPCs, reduce `sUnknown_20266B0[160]` to `[80]` | | | |
| **Unused EWRAM (marked `UNUSED`)** | ~1 KB | ~1 KB | Easy | Multiple files |
| - Delete `sUnknownUnusedEwram[0x140]` and similar | | | |

**Conservative Estimate: 15-20 KB** can be reclaimed with moderate effort
**Aggressive Estimate: 30-40 KB** if we're willing to refactor recruit storage

---

## Implementation Plan (Recommended)

### Phase 1: Low-Hanging Fruit (Easy, ~2-3 KB)
1. Delete explicitly unused variables:
   - `sUnknownUnusedEwram[0x140]` in `src/bg_control.c:8` (320 bytes)
   - `sUnused1/2/3` in `src/sprite.c` (12 bytes)
   - `sUnusedEwram1[4]` in `src/string_format.c` (4 bytes)
   - Other `UNUSED` markers scattered throughout

**Risk: None** | **Effort: 1 hour** | **Testing: Compile + boot**

### Phase 2: Window System Reduction (Medium, ~5-8 KB)
1. Reduce `MAX_WINDOWS` constant:
   - Current: 128 windows
   - Roguelike needs: ~16-32 (title, menus, dungeon UI)
   - Savings: `(128 - 32) * sizeof(Window)` ≈ several KB

2. Audit `gFormatBuffer_*` arrays:
   - `gFormatBuffer_Monsters[10][FORMAT_BUFFER_LEN]` → reduce to `[4]`
   - `gFormatBuffer_Names[10][FORMAT_BUFFER_LEN]` → reduce to `[4]`
   - Only need buffers for active party, not full roster

**Risk: Low** (roguelike has fewer simultaneous UI elements)
**Effort: 4-6 hours** (need to verify window usage)
**Testing: Navigate all menus, dungeons, title screen**

### Phase 3: Investigate Large Unknowns (Hard, ~10-15 KB)
1. `sUnknown_20274B4[0xEC0]` (15 KB) in `src/text_1.c:24`:
   - Purpose unclear, might be text rendering cache
   - Need to trace all references
   - Possibly reducible or deletable

2. `gRecruitedPokemonRef` (38 KB) in `src/pokemon.c`:
   - If roguelike only needs active party + storage box
   - Could refactor to store in SRAM/Flash instead of EWRAM
   - Major undertaking but huge savings

**Risk: Medium-High** (unknown purpose structures are risky)
**Effort: 10-20 hours** (deep code analysis required)
**Testing: Full playthrough of roguelike loop**

---

## Why NOT to Reduce Heap (Yet)

The heap should be sized based on **actual runtime peak usage**, not guessed. Here's what we'd need to do it safely:

1. **Instrument every `MemoryAlloc` call** to log:
   - Allocation size
   - Current heap usage
   - Caller location

2. **Test every scenario**:
   - Title screen loop
   - New game personality quiz
   - Every dungeon (50+ dungeons!)
   - Every boss fight
   - Every shop interaction
   - Mail system
   - Friend areas
   - Evolution scenes

3. **Find peak usage** across all paths

4. **Add safety margin** (20%+)

5. **Only then** reduce `HEAP_SIZE`

**Estimated effort: 20-40 hours** of testing + instrumentation
**Risk: High** - easy to miss edge cases

---

## Recommended Action

**Start with Phase 1 + Phase 2** (Static EWRAM reduction):

```bash
# Expected outcome after Phase 1+2:
# EWRAM: ~243 KB / 256 KB (92-93% used)
# Free EWRAM: ~13-19 KB
# Heap size: Still 139 KB (unchanged)
# Risk: Very low
# Effort: ~6-8 hours
```

This buys us breathing room without touching the heap. The dungeon allocation should succeed because:
1. We freed heap space at runtime (Ground pools reduced, effect context freed)
2. If we still hit issues, we can increase `HEAP_SIZE` slightly (the static EWRAM savings make this possible)

**Defer heap reduction until we have**:
- Stable roguelike loop working end-to-end
- Heap instrumentation in place
- Full test coverage of all dungeons

---

## Emergency Fallback: Hybrid Approach

If the dungeon STILL doesn't allocate after our current changes:

1. **Reclaim 10 KB static EWRAM** (Phase 1 + easy Phase 2 items)
2. **Increase `HEAP_SIZE` by 10 KB**: `0x22000` → `0x24800` (149 KB heap)
3. **Monitor build output**: Should stay under 256 KB EWRAM

This gives us a bigger heap without reducing it (which is risky). The static EWRAM savings "pay for" the heap increase.

---

## Conclusion

**Recommended**: Focus on **eliminating static EWRAM** (Option B)
**Rationale**: Safer, measurable, permanent
**Next Step**: Implement Phase 1 (unused variables) to prove the approach
**Heap Reduction**: Defer until we have runtime instrumentation and full test coverage

The goal is **reliability first, optimization second**. Static EWRAM reduction gives us both.
