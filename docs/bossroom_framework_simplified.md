# Boss Room Framework - Quick Start Guide

**TL;DR:** You can easily change boss room visuals and layouts by modifying two numbers in the code.

## The Two Magic Numbers

Every boss room has two settings:

1. **Tileset** (how it looks) - Number 64-74
2. **Layout** (arena shape) - Number 1-55

Change these in `src/dungeon_seed_overrides.c:993`:

```c
result->bossFight.roomTileset = 64;      // ← Change this for different VISUALS
result->bossFight.fixedRoomNumber = 1;   // ← Change this for different LAYOUT
```

## Visual Tilesets (How It Looks)

| Number | Theme | Dungeon |
|--------|-------|---------|
| **64** | Steel/metallic | Mt. Steel (Skarmory) |
| **65** | Dark forest | Sinister Woods |
| 66 | Electric | Mt. Thunder Peak |
| 67 | Fire/lava | Mt. Blaze Peak |
| 68 | Ice/snow | Frosty Grotto |
| 69 | Ice cave | Mt. Freeze Peak |
| 70 | Deep lava | Magma Cavern |
| 71 | Sky/clouds | Sky Tower Summit |
| 74 | Ocean/water | Stormy Sea |

## Arena Layouts (Arena Shape)

Browse all 55 layouts visually:
```bash
cat extracted_patterns/room_visualizations.txt
```

**Popular layouts:**

| Number | Size | Description |
|--------|------|-------------|
| **1** | 9×17 | Rectangular (Skarmory) |
| **2** | 13×13 | Square with pillars (Team Meanies) |
| 3 | 9×10 | Compact (Zapdos) |
| 4 | 14×17 | Large with hazards (Moltres) |
| 5 | 12×12 | Medium square (Articuno) |

## Quick Examples

### Switch to Sinister Woods (Bug-type bosses)

```c
result->bossFight.roomTileset = 65;      // Dark forest visuals
result->bossFight.fixedRoomNumber = 2;   // Team Meanies layout
```

### Switch to Electric Theme

```c
result->bossFight.roomTileset = 66;      // Electric visuals
result->bossFight.fixedRoomNumber = 3;   // Zapdos layout
```

### Mix and Match!

```c
// Fire visuals + Ice layout
result->bossFight.roomTileset = 67;      // Fire theme
result->bossFight.fixedRoomNumber = 5;   // Articuno arena shape
```

## Type-Based Auto-Selection

Want bosses to automatically get themed rooms based on their type?

Add this function to `src/dungeon_seed_overrides.c`:

```c
static u8 GetBossRoomTileset(s16 bossSpecies) {
    u8 type = GetPokemonType(bossSpecies, 0);  // Get primary type

    switch (type) {
        case TYPE_STEEL:    return 64;  // Steel theme
        case TYPE_BUG:      return 65;  // Forest theme
        case TYPE_ELECTRIC: return 66;  // Electric theme
        case TYPE_FIRE:     return 67;  // Fire theme
        case TYPE_ICE:      return 68;  // Ice theme
        case TYPE_WATER:    return 74;  // Ocean theme
        default:            return 64;  // Default
    }
}
```

Then use it at line 993:
```c
result->bossFight.roomTileset = GetBossRoomTileset(result->bossFight.bossSpecies);
```

Done! Now Bug bosses get dark forest rooms, Fire bosses get lava rooms, etc.

## How The Extracted Patterns Work

The **extracted patterns** are the arena layouts stored as simple C arrays instead of compressed ROM data.

**Where they come from:**
1. All 55 original boss arenas were extracted from the game
2. Converted to C arrays (see `extracted_patterns/fixed_rooms.c`)
3. Visualized with ASCII art (see `extracted_patterns/room_visualizations.txt`)

**Why use extracted patterns?**
- ✓ Simpler code (no decompression)
- ✓ Easy to modify layouts
- ✓ Can visualize with ASCII art
- ✓ Can create new custom arenas

**Current system uses extracted patterns** via `LoadCustomFixedRoom()` in `src/custom_fixed_rooms.c`.

### Viewing Extracted Patterns

```bash
# See all 55 arena layouts with ASCII art
cat extracted_patterns/room_visualizations.txt

# Example output:
# Room 1 (17x9)
# +#######+
# +#######+
# #########
# ###T!T###
# #+++++++#
# T+++++++T
# ~~~~~~~~~
# ~.......~
# ....P....
#
# Legend:
#   . = Floor    # = Wall    ~ = Water    P = Player spawn
```

### Adding More Layouts to Your Code

Currently only Room 1 is compiled. To add Room 2:

1. Open `extracted_patterns/fixed_rooms.c`
2. Copy the `fixed_room_2_tiles` array
3. Paste into `src/custom_fixed_rooms.c`
4. Add case to `LoadCustomFixedRoom()` function
5. Use it: `result->bossFight.fixedRoomNumber = 2;`

See `docs/bossroom_framework.md` for detailed instructions.

## Summary

- **Tilesets** control visuals (steel, forest, fire, ice, etc.)
- **Layouts** control arena shape (rectangular, square, with pillars, etc.)
- Change two numbers to get different rooms
- 55 layouts × 10+ tilesets = **550+ combinations!**
- All layouts extracted and available in `extracted_patterns/`
- Can browse patterns with ASCII art before using them

## Full Documentation

For complete details, see:
- `docs/bossroom_framework.md` - Complete framework documentation
- `extracted_patterns/room_visualizations.txt` - Visual browser of all 55 layouts
- `extracted_patterns/fixed_rooms.c` - C code for all patterns
- `src/custom_fixed_rooms.c` - Current implementation
