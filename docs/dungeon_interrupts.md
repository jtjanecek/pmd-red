# Dungeon Floor Interrupt Hook (Frosty Forest baseline)

When you climb from 5F to 6F in Frosty Forest the game pauses on a black screen and shows Articuno's warning text. That flow is useful as a template for any future "floor interrupt" and is spread across a few files.

## Trigger logic
- `RunDungeon_Async` (`src/run_dungeon.c:588`) advances `gDungeon->unk644.dungeonLocation.floor` each time you successfully leave a floor. Right after incrementing it, the code checks whether:
  - the active dungeon is `DUNGEON_FROSTY_FOREST`,
  - the new floor equals `6`, and
  - scenario flag `0x1F` has not been set yet (`sub_8098100(0x1F)` returns `FALSE`).
- If those checks pass, the function sets the flag (`sub_8097FA8(0x1F)`), runs the cutscene entry point (`sub_8086130()`), and then persists the updated flags (`sub_8097FF8()`).

## Flag helpers
- `sub_8098100` / `sub_8097FA8` / `sub_8097FF8` live in `src/exclusive_pokemon.c:40-78`. They are a lightweight bitfield system that the game already uses for "scenario" progress.
  - `sub_8098100(flagId)` → test whether a flag has already been set (avoids repeating the interrupt).
  - `sub_8097FA8(flagId)` → set the pending flag bit.
  - `sub_8097FF8()` → mirror every pending bit into the saved "seen" bitset and clear the pending buffer. Always call this after mutating the flags or the game will treat the change as temporary.

Pick an unused flag ID if you add more interrupts so you do not collide with existing cutscenes.

## Cutscene sequence
- `sub_8086130` (`src/dungeon_cutscene.c:1424`) owns the Articuno warning sequence. It takes care of fading out the BGM, inserting short waits (`sub_803E708`), and calling the black-screen text renderer five times with different strings.
- The strings themselves (`gUnknown_810665C` through `gUnknown_810671C`) live in `src/dungeon_boss_dialogue.c:3063-3071`. You can replace them or point `sub_8086130` at a different array when designing another interrupt.

## Black-screen text renderer
- `sub_8052FB8` (`src/dungeon_message.c:703`) is the helper responsible for the "black screen with centered text" effect. The comment above it even says “Used only for displaying Frosty Forest's text at floor 6.”
- The function clears BG tiles, animates the palette fade, shows the passed-in string inside the cutscene dialogue box, waits for player input, and then restores the dungeon HUD. Reusing it elsewhere will automatically give you the same look.

## Reusing the pattern
1. Decide on the trigger (dungeon + floor check) and add a new conditional block inside `RunDungeon_Async` right after the floor increment. Gate it with `sub_8098100` so it runs once.
2. Reserve a new flag ID and use the `sub_8097FA8` / `sub_8097FF8` pair to mark it as seen.
3. Either reuse `sub_8086130` with new text arrays or create a sibling function that calls `sub_8052FB8` for whatever dialogue you need.
4. Provide the dialogue strings (use `src/dungeon_boss_dialogue.c` or another shared table) so localization helpers can find them.

Following those four files (`run_dungeon.c`, `exclusive_pokemon.c`, `dungeon_cutscene.c`, and `dungeon_message.c`) is enough to bolt in future dungeon interrupts without reverse-engineering the entire dungeon engine again.
