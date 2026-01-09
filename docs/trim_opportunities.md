# Trim Opportunities (EWRAM/ROM)

## Current budget (pmd_red.map)
- EWRAM used ~0x3D8D8 with HEAP_SIZE 0x24000 => +0x81C (2,076 bytes) over 256 KB.
- IWRAM used ~0x78F0 => ~0x710 free.

## Highest EWRAM blocks (from pmd_red.map)
- src/memory.c: sMainHeap + allocator (0x24340 total in src/memory.o).
- src/pokemon.c: sRecruitedPokemon (0x90E8 inside src/pokemon.o).
- src/text_1.c: window scratch + tilemaps (0x5CC8 in src/text_1.o).
- src/sprite.c: sprite link + copy tables (0x1A3C in src/sprite.o).
- src/string_format.c: format buffers + dialogue scratch (0x0E00 in src/string_format.o).
- src/code_8094F88.c: mail arrays (0x7B8).
- src/code_80958E8.c: mail/news arrays (0x32C).
- src/event_flag.c: gScriptVarBuffer (0x400).

## EWRAM trims (no feature removals, behavior risk)
- src/string_format.c: reduce DIALOGUE_TEXT_BUFFER_SIZE (1000 -> 640/720) to save 280-360 bytes.
- include/string_format.h: reduce FORMAT_BUFFER_LEN (80 -> 64) to save ~416 bytes across gFormatBuffer_* and speaker/team name buffers.
- src/string_format.c: reduce gFormatBuffer_Monsters/gFormatBuffer_Names count from 10 -> 6 if scripts never use m6+/n6+ (saves 640 bytes).
- src/text_1.c: shrink sUnknown_20274B4 (0xEC0 u32 scratch) once max window area is known; 25% cut saves ~3.8 KB.
- src/sprite.c: reduce UNK_20266B0_ARR_COUNT 160 -> 128 if sprite part registrations never exceed 128 (saves 384 bytes).
- src/event_flag.c: reduce SCRIPT_VAR_BUFFER_LEN if script vars are no longer used (0x400 -> 0x200 saves 512 bytes).
- IWRAM can take ~0x710 bytes: move one small buffer (gScriptVarBuffer or sDialogueTextBuffer), then still trim EWRAM by ~0x110.

## EWRAM trims aligned to removed features (Pelipper/Post Office/Wonder Mail)
- src/code_8094F88.c: mail arrays (0x7B8). If mail is removed, shrink or replace:
  - gUnknown_2038C88[0x20] (unkStruct_203B480 slots) in src/code_8094F88.c.
  - gUnknown_20392E8[0x36] and related mail bookkeeping in src/code_8094F88.c.
- src/code_80958E8.c: mail/news arrays (0x32C). Shrink via constants:
  - include/constants/mailbox.h: NUM_MAILBOX_SLOTS, MAX_ACCEPTED_JOBS.
  - include/constants/wonder_mail.h: NUM_POKEMON_NEWS.
- If fully removing mail, delete or stub:
  - src/mailbox.c, src/pelipper_board.c, src/wonder_mail*.c, src/pokemon_news*.c,
    src/friend_rescue.c, src/rescue_password_menu.c, src/thank_you_wonder_mail.c.
- Removing src/code_8094F88.c + src/code_80958E8.c EWRAM alone (~0xAE4) fixes the current 0x81C overflow.

## ROM-only trims (story + removed NPCs)
- Story cutscene code: src/dungeon_cutscene*.c, src/dungeon_cutscene.c, src/rescue_scenario.c,
  src/dungeon_boss_dialogue.c.
- Story ground scripts/text: data/ground/ground_data_*_station.h, src/ground_script.c,
  data/ground/ground_data_* maps tied to story flow.
- Whiscash NPC removal: drop GROUND_PLACE_WHISCASH_POND in src/ground_place.c, its map asset in
  data/ground, and any ground script stations referencing it.
- Pelipper assets/text: data/pokemon_mail*.h, data/pokemon_mail_pre.h, data/pokemon_news*,
  plus any menu strings that reference the Post Office.

## Notes
- Removing story scripts frees ROM, not EWRAM, but helps the 32 MB cap.
- Keep monster assets if the species is still recruitable/spawnable; remove only story-specific
  cutscene scripts and map assets.
