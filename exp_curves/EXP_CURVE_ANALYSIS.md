# Pokemon EXP Curve Analysis for PMD Red

This directory contains Python scripts to extract and analyze Pokemon level-up EXP curves from Pokemon Mystery Dungeon: Red Rescue Team.

## Scripts

### 1. `analyze_exp_curves.py`
Extracts raw EXP curve data from the ROM and outputs a detailed CSV.

**Usage:**
```bash
python3 analyze_exp_curves.py
```

**Output:** `pokemon_exp_curves.csv`
- Contains level-by-level data for all 421 Pokemon species
- Includes: species ID, name, level, EXP required, and stat gains per level
- Total: 42,100 rows (421 Pokemon × 100 levels each)

**CSV Columns:**
- `species_id` - Internal Pokemon species ID
- `species_name` - Pokemon name (e.g., "Bulbasaur")
- `level` - Level (1-100)
- `exp_required` - Total EXP needed to reach this level
- `gain_hp` - HP gained when leveling to this level
- `gain_att` - Normal Attack stat gain
- `gain_sp_att` - Special Attack stat gain
- `gain_def` - Normal Defense stat gain
- `gain_sp_def` - Special Defense stat gain

### 2. `analyze_exp_summary.py`
Analyzes the EXP curve data and generates summary statistics.

**Usage:**
```bash
python3 analyze_exp_summary.py
```

**Output:** `pokemon_exp_summary.csv` + console analysis

**Summary Includes:**
- Growth rate classification for each Pokemon
- Fastest/slowest growing Pokemon rankings
- Total stat gains from level 1-100
- EXP curve distribution analysis

## Key Findings

### Growth Rate Distribution
- **Fluctuating (419 Pokemon):** Most Pokemon follow a "Fluctuating" growth curve
- **Erratic (2 Pokemon):** Only Munchlax and Decoy have the fastest growth rate

### EXP to Level 100 Range
- **Fastest:** Munchlax/Decoy - 114,728 EXP
- **Slowest:** Nidoran♂ - 3,924,000 EXP
- **Average:** ~2,800,000 EXP

### Interesting Examples
| Pokemon    | EXP to 100 | Growth Rate |
|------------|------------|-------------|
| Mewtwo     | 2,185,190  | Fluctuating |
| Pikachu    | 3,466,815  | Fluctuating |
| Bulbasaur  | 3,685,190  | Fluctuating |
| Mudkip     | 3,700,230  | Fluctuating |
| Munchlax   | 114,728    | Erratic     |

## Technical Details

### Data Source
Data is extracted from the ROM's system file archive (`gSystemFileArchive` at 0x8300500):
- Files named `lvmp001` through `lvmp600` contain level data
- Each file stores 100 `LevelData` structures (12 bytes each)
- Data is compressed using AT compression (custom format)

### LevelData Structure
```c
typedef struct LevelData {
    s32 expRequired;  // Total EXP required to reach this level
    u16 gainHP;       // HP gained at this level
    u8 gainAtt[2];    // [Normal Attack, Special Attack] gains
    u8 gainDef[2];    // [Normal Defense, Special Defense] gains
    u16 fillA;        // Padding
} LevelData;  // 12 bytes (0xC)
```

### Decompression
The script implements the AT (Anthropic Technology) decompression algorithm used by PMD:
- Supports AT3P and AT4P compressed formats
- Handles back-references and special nibble patterns
- See `decompress_at()` function in `analyze_exp_curves.py`

## Files Generated

1. **pokemon_exp_curves.csv** (1.3 MB)
   - Complete level-by-level data for all Pokemon
   - 42,100 rows of detailed progression data

2. **pokemon_exp_summary.csv** (20 KB)
   - One row per Pokemon with summary statistics
   - 421 rows of aggregate data

## Notes

- 178 species IDs failed to extract (likely unused/invalid IDs)
- Level 1 always shows 0 EXP required and 0 stat gains
- PMD uses different growth curves than main series Pokemon games
- Most legendaries have faster growth rates than starters

## Requirements

- Python 3.6+
- Standard library only (no external dependencies)
- ROM file: `pmd_red.gba` or `baserom.gba` in project root
- Monster names: `data/monster/monster_data.json`
