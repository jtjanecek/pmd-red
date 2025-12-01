# Boss Fight Dungeon Tileset IDs

This document catalogs the tileset IDs used for boss fight dungeons in Pokemon Mystery Dungeon Red. These IDs determine the visual appearance (graphics, tiles, terrain) of the dungeon floor.

## Currently Used Tileset

**Skarmory Boss Fight (Custom)**: Tileset ID **64** (0x40)
- Location: `src/dungeon_seed_overrides.c:936` and `src/dungeon_seed_overrides.c:993`
- Used for all procedurally generated boss fights in the custom boss system

## Main Story Boss Fight Tilesets

| Boss | Dungeon | Tileset ID | Hex | Data File |
|------|---------|-----------|-----|-----------|
| Rayquaza | Sky Tower Summit | 71 | 0x47 | `data/dungeon/SkyTowerSummit/main_data.inc` |
| Kyogre | Stormy Sea Floor 40 | 74 | 0x4a | `data/dungeon/StormySea/main_data.inc` |
| Groudon | Magma Cavern Pit (Floor 23) | 70 | 0x46 | `data/dungeon/MagmaCavernPit/main_data.inc` |
| Zapdos | Mt. Thunder Peak | 66 | 0x42 | `data/dungeon/MtThunderPeak/main_data.inc` |
| Moltres | Mt. Blaze Peak | 67 | 0x43 | `data/dungeon/MtBlazePeak/main_data.inc` |
| Articuno | Frosty Grotto | 68 | 0x44 | `data/dungeon/FrostyGrotto/main_data.inc` |
| Ninetales | Mt. Freeze Peak | 69 | 0x45 | `data/dungeon/MtFreezePeak/main_data.inc` |
| Lugia | Silver Trench (Deep) | 73 | 0x49 | `data/dungeon/SilverTrench/main_data.inc` |

## Post-Game Boss Fight Tilesets

| Boss | Dungeon | Tileset ID | Hex | Data Source |
|------|---------|-----------|-----|-------------|
| Latios | Pitfall Valley (Floor 25) | 48 | 0x30 | `data/dungeon/main_data.inc` entry 702 |
| Latias | Southern Cavern (Floor 50) | 48 | 0x30 | `data/dungeon/main_data.inc` entry 809 |
| Mewtwo | Western Cave | TBD | - | No main_data.inc file found |
| Regirock/Regice/Registeel | Buried Relic | TBD | - | No main_data.inc file found |
| Various Legendaries | Fiery Field | TBD | - | No main_data.inc file found |
| Various Legendaries | Lightning Field | TBD | - | No main_data.inc file found |
| Various Legendaries | Northwind Field | TBD | - | No main_data.inc file found |

## How Tilesets Work

### Loading Process

1. **Floor Properties Loading** (`src/dungeon_floor_spawns.c:161`)
   - Loads `FloorProperties` structure from dungeon mapparam data
   - Includes tileset ID as part of floor properties

2. **Tileset Assignment** (`src/run_dungeon.c:372`)
   ```c
   gDungeon->tileset = gDungeon->floorProperties.tileset;
   ```

3. **Graphics Loading** (`src/dungeon_map_access.c:79`)
   - Uses tileset ID to load the appropriate graphics assets
   - Each tileset ID maps to specific tile graphics, palettes, and terrain behavior

### Binary Data Structure

Each `main_data.inc` file contains floor properties as binary data (28 bytes per floor):
- **Byte 0**: Unknown property
- **Byte 1**: Unknown property
- **Byte 2**: **Tileset ID** ← This is what we need
- **Bytes 3-27**: Other floor properties (layout algorithm, spawn rates, items, etc.)

### Boss Fight Override System

For custom boss fights using the seed override system:
- Tileset is set in `src/dungeon_seed_overrides.c:993`
- Currently hardcoded to tileset **64** (Skarmory's room)
- Can be changed per-boss by modifying `result->bossFight.roomTileset`

## Tileset Type System

The game has a type-to-tileset mapping system (`src/data/type_tileset_table.c`) that maps Pokemon types to appropriate tileset IDs for themed dungeons. This is used for procedurally generated dungeons.

### Special Tileset Properties

- **Water vs Lava**: `src/dungeon_data.c:58` contains `gDungeonWaterType[76]` array
  - Determines if a tileset uses water or lava terrain
  - Affects terrain damage and movement

## Usage Examples

### To use Rayquaza's Sky Tower tileset for a boss:
```c
result->bossFight.roomTileset = 71;  // Sky Tower Summit
```

### To use Kyogre's Stormy Sea tileset for a boss:
```c
result->bossFight.roomTileset = 74;  // Stormy Sea
```

### To use Groudon's Magma Cavern tileset for a boss:
```c
result->bossFight.roomTileset = 70;  // Magma Cavern Pit
```

### To use Latios/Latias tileset for a boss:
```c
result->bossFight.roomTileset = 48;  // Pitfall Valley / Southern Cavern
```

## Finding Additional Tileset IDs

If you need to find tileset IDs for other dungeons:

1. Locate the dungeon's `main_data.inc` file in `data/dungeon/{DungeonName}/`
2. The tileset ID is at **byte 2** of each floor's property data
3. Each floor entry is 28 bytes long
4. For multi-floor dungeons, boss floors are typically the last entry

## Visual References

To see what each tileset looks like:
- Boot the game and visit the corresponding dungeon
- The tileset determines:
  - Floor/wall tile graphics
  - Water/lava appearance
  - Terrain type visuals
  - Overall aesthetic theme

## Notes

- Tileset ID 64 (Skarmory) is currently used as the default for all custom boss fights
- Original boss dungeons use unique tilesets that match their thematic location
- Some dungeons (Western Cave, Buried Relic, etc.) may use dynamically assigned tilesets or share tilesets with other dungeons
- The tileset ID only affects visuals, not gameplay mechanics (except for water/lava terrain type)
