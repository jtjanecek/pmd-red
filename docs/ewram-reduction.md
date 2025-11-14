# EWRAM Reduction Plan

This document tracks the largest EWRAM consumers in the current ROM and outlines where we can claw memory back—especially by stripping most of Pokémon Square (Town Center) content while keeping the Kecleon shop, Felicity Bank, and Kangaskhan Storage accessible elsewhere.

## 1. Budget Snapshot

- `pmd_red.map:6646` reports `ewram` at `0x3d738` bytes used out of the `0x40000` budget, leaving only `0x28C8` (~10.4 KiB) free headroom.
- EWRAM is split between static blocks described in the map file and dynamic allocations pulled from the main heap (`sMainHeap` in `src/memory.c:7-65`).

| Block | Size | Source | Notes |
| --- | --- | --- | --- |
| Main heap (`sMainHeap`) | `0x24348` (148 KiB) | `src/memory.c:7-66` | Every `MemoryAlloc` (Ground systems, cutscenes, etc.) comes from here. Reducing Town systems directly lowers the heap requirement so `HEAP_SIZE` can drop from `0x24000`. |
| Pokémon roster (`gRecruitedPokemonRef`, level cache, shadows) | `0x95C0` (38 KiB) | `src/pokemon.c:18-34`, `pmd_red.map:6954-7010` | Holds every recruit plus team slots. If the roguelike never stores hundreds of recruits, the struct can be trimmed or moved to SRAM. |
| Window/text buffers (`gWindows`, `gBgTilemaps`, formatting caches) | `0x5CC8` (23 KiB) | `src/text_1.c`, `src/string_format.c`, `pmd_red.map:6704-6810` | Text UI is always resident. We can drop unused fonts and shrink `gWindows` once Town UIs are gone. |
| Dungeon seed overrides cache | `0x18E2` (6.2 KiB) | `src/dungeon_seed_overrides.c:56-143`, `pmd_red.map:6656-6666` | `sSeededDungeonName1/2` + validity bits. If seeded dungeons are optional, gate this behind a build flag. |
| Sprite pools (`SpriteList`, OAM mirrors) | `0x17C8` (6 KiB) | `src/sprite.c:8-40`, `pmd_red.map:6695-6703` | Needed in dungeons, but the unused padding words (see §4) can be dropped. |
| Sound driver track state | `0x870` (2.1 KiB) | `data/sound_data.o(.bss)`, `pmd_red.map:6649-6658` | Fixed size, but still worth noting; the roguelike soundtrack could run with fewer simultaneous players. |

## 2. Town Center Footprint

### 2.1 Ground map renderer (`GroundBg`)

`GroundBg_Init` (`src/ground_bg.c:36-70`) allocates several large buffers from the main heap based on the `SubStruct_52C` preset chosen in `GroundMap_Select` (`src/ground_map.c:17-120`, `src/ground_map.c:368-460`):

| Preset | Used by | Layers | Allocations | Bytes |
| --- | --- | --- | --- | --- |
| `gUnknown_8117324` | Most outdoor maps including `MAP_SQUARE`/`MAP_POKEMON_SQUARE` (`src/ground_map_conversion_table.c:1-40`) | 1 | `tileMappings = unk8 * 18`, `chunkMappings = unk10 * 128`, `unk544 = unkE * 256` | 0x4B0*18 = **21,600**, 0x5E*128 = **12,032**, 0xBC*256 = **48,128** → **~80 KiB per load** |
| `gUnknown_811733C` | Pelipper Post Office + other multi-layer scenes (`unk0 = 5..9`) | 2 | Same as above but `numLayers = 2` | Adds a second `chunkMappings`, pushing the total to **~94 KiB** |

Because these buffers stay live while the player stands in town, Pokémon Square alone consumes over half of `sMainHeap`. Eliminating Square entirely (or forcing it to reuse the lighter base map) frees most of this memory.

**Actionable steps**

1. **Redirect Square to the base exterior.** Change `gGroundMapConversionTable[MAP_POKEMON_SQUARE].mapFileTableId` to the same entry as `MAP_SQUARE`, or have `GroundMap_Select()` treat map type `1` as a no-op that immediately warps to the base when `mapId == MAP_POKEMON_SQUARE`. All NPC logic can then live on the base map.
2. **Block the old Square entrance.** Edit the base’s `.bma` (`ground_data_t01p0*_station.c`) to add the right-side barricade plus a Kangaskhan statue; no Square BMA needs to be in the ROM anymore.
3. **Reuse the simple preset.** If you still keep a cut-down Square, keep its `ground_map_conversion_table` entry on `unk0 == 1` so it uses the single-layer preset. Avoid any maps with `unk0` in `5..9` unless you absolutely need parallax.

### 2.2 Town entity pools (`GroundLives`, `GroundObjects`, `GroundEffects`, `GroundEvents`)

These modules all allocate fixed-size pools out of the heap during `Alloc*` and never release them until you leave town. Most of them only exist to animate Square NPCs, so bypassing the Square lets us shrink or delete the pools.

| System | Code reference | Allocation | Bytes today | Removal plan |
| --- | --- | --- | --- | --- |
| GroundLives (NPCs) | `src/ground_lives.c:198-235` | `sizeof(GroundLives)` holds `0x18` slots of `GroundLive` (`0x1F0` bytes each) plus `gGroundLivesMeta` (`0x338`). | `0x2E80 + 0x338 ≈ 12.7 KiB` | Reduce `UNK_3001B84_ARR_COUNT` to 4 and only spawn Kecleon, Felicity, Kangaskhan, and the statue helper on the base. Everyone else can be hard-removed from scripts. |
| GroundObjects (buildings, doors) | `src/ground_object.c:119-153` | `MemoryAlloc(sizeof(GroundObject) * 0x10, 6)` with `sizeof` `0x1C4`. | `0x1C40 ≈ 7.1 KiB` | NPC interaction is minimal once Square is gone. Replace the dynamic pool with a single `GroundObject` for the Kangaskhan statue and kill the rest. |
| GroundEffects | `src/ground_effect.c:121-158` | Same struct/size as objects (`0x1C4`) ×16. | `~7.1 KiB` | No longer needed if we remove Square FX (waterfalls, sparkles). Stub out `AllocGroundEffects`. |
| GroundEvents | `src/ground_event.c:24-58` | `sizeof(GroundEvent) * 0x20` (32 scripted triggers). | ~1–2 KiB | Once Square scripts vanish, only a handful of warp triggers remain. Replace with static door code or fold into dungeon entrance logic. |
| GroundScripts cache | `src/ground_script.c:170-217` | `gUnknown_203B4B0 = MemoryAlloc(0x400, 6);` for per-map locks. | 1 KiB | If scripts only run in Base, shrink or delete the lock arrays and rely on the dungeon UI for choices. |
| Ground sprites | `src/ground_sprite.c:63-104` | `gUnknown_3001B7C = MemoryAlloc(0x110, 6);` + sprite book-keeping; also reuses the main sprite OAM pool. | 272 B (struct) + per-sprite VRAM | With just three shopkeepers, stop loading the `ground_sprite_data` archive and draw them using the dungeon sprite system. |

Taken together, Square-specific pools chew through ~28 KiB of the heap before counting the map renderer. When the roguelike never enters Square, you can either delete the allocation calls or guard them behind `if (mapId == MAP_POKEMON_SQUARE)` so they never run.

### 2.3 Keeping the essential shops

To keep Kecleon, Felicity Bank, and Kangaskhan Storage reachable without Square:

1. **Spawn shopkeepers on the base map.** Add three `GroundLivesData` entries to the base script (`ground_data_t01p0x_station.c`) and reuse the existing shop scripts. Their memory footprint becomes three `GroundLive` structs (~1.5 KiB total) instead of the full pool.
2. **Wire shop scripts to dungeon menus.** The existing script IDs still point to `ground_script` routines for each shopkeeper (`src/ground_script.c`). As long as the NPC exists, the menus (bank, storage, shop) still function even if Square is absent.
3. **Handle map transitions locally.** Block the right-hand tunnel, remove warp scripts to Square (`GroundEvent_Select`), and add a Kangaskhan statue `GroundObject` that opens storage outside the base.

## 3. Other High-Value Targets

Even after deleting Square, a few static blocks dominate EWRAM. Addressing them yields additional savings:

| Area | Reference | Why it’s large | Trim idea |
| --- | --- | --- | --- |
| `HEAP_SIZE` | `src/memory.c:7-66` | Currently `0x24000` to support Square + cutscenes. Without those systems, you can drop it to `0x18000` (~96 KiB) immediately and iterate. |
| Recruited Pokémon store | `src/pokemon.c:18-66` | `RecruitedMon` tracks every monster + team slots even if you never recruit. | Reduce `NUM_MONSTERS` or store the roster in SRAM/Flash instead of EWRAM. For a roguelike, a small active-party array plus storage chest is enough. |
| Text / formatting buffers | `src/text_1.c`, `src/string_format.c:24-80` | Multiple global arrays (`gWindows`, `gCharmaps`, `gFormatBuffer_*`) total 23 KiB. | Remove Square-only windows, keep only 2–3 window slots, and shrink the formatting buffers to the lengths you actually print (items/team names). |
| Dungeon seed overrides | `src/dungeon_seed_overrides.c:56-143` | `sSeededDungeonName1/2[NUM_DUNGEONS][32]` + validity flags. | If the roguelike doesn’t randomize names, compile this file out or drop the caches to `NUM_DUNGEONS = 32`. |
| Sprite buffers | `src/sprite.c:8-40` | `sUnknown_20266B0[160]`, `SpriteList`, `SpriteOAM[128]` live permanently. | In a pure dungeon crawler you can cut the sprite list to 96 entries and remove the unused words called out below. |

## 4. Reclaimable / Unused Blocks

Many EWRAM symbols are explicitly marked `UNUSED`; deleting them or turning them into constants frees a few hundred bytes each. Highlights:

| Symbol | Location | Bytes | Notes |
| --- | --- | --- | --- |
| `sUnused1/2/3` | `src/sprite.c:14-26` | 12 | Never read; delete the declarations. |
| `sUnknownUnusedEwram[0x140]` | `src/bg_control.c:8-15` | 320 | Truly unused filler next to the BG blend registers. |
| `sUnusedEwram1[4]` | `src/string_format.c:24-40` | 4 | Remove; formatting still works. |
| `sUnused` | `src/other_menus2.c:24` & `src/dungeon_menu_items.c:36`, `src/dungeon_menu_team.c:50`, `src/dungeon_menu_others.c:58` | 4–8 each | Leftover padding from menu ports. |
| `sUnusedEwram` | `src/dungeon_generation.c:5862` | 4 | Safe to drop. |
| `gUnknown_20398BB` | `src/ground_main.c:44` | 1 | A remnant flag that’s never read once scripts are disabled. |
| `sUnknown_2039DBC` | `src/ground_sprite.c:48` | 4 | Alignment filler; delete after tightening the struct. |

While each is small, removing them buys back ~0.6 KiB and—more importantly—keeps the map file clean so genuine regressions stand out.

## 5. Recommended Next Steps

1. **Delete Pokémon Square assets** (BMA/BPL/BPC, scripts, and conversions) so the build never allocates the 80 KiB renderer buffers. Redirect every warp to stay on the base map.
2. **Trim the Ground pools** by rewriting `AllocGroundLives/Objects/Effects/Events` to allocate only what the base scene needs (three shopkeepers + one statue). Once verified, lower `HEAP_SIZE` and reclaim the slack permanently.
3. **Audit `MemoryAlloc` callers** under the new flow and record the peak heap usage with MGBA logs. Use that data to pick an aggressive new heap size and catch any remaining Square dependencies.
4. **Chop unused globals** listed in §4 and rerun the linker to confirm the `ewram` section falls well below `0x3a000`. This gives headroom for dungeon features without touching IWRAM.

Following this plan removes the Town Center’s ~110 KiB footprint, drops the heap ceiling, and exposes several other easy wins so EWRAM stops being the limiting factor for the roguelike.
