# Room-Wide Moves

This document lists all moves that affect the entire room or multiple targets within a room in Pokemon Mystery Dungeon: Red Rescue Team.

## Move Range Types

### MOVE_RANGE_ROOM (2 moves)
Affects the entire room (all Pokemon)

| Move | Type | Power | Description |
|------|------|-------|-------------|
| Haze | Ice | 2 | Removes stat changes from all Pokemon in room |
| Baton Pass | Normal | 2 | Switches user with ally, transferring stat changes |

### MOVE_RANGE_ROOM_FOES (16 moves)
Affects all enemies in the room

| Move | Type | Power | Description |
|------|------|-------|-------------|
| Sweet Scent | Normal | 2 | Lowers evasion of all enemies in room |
| Memento | Dark | 2 | User faints, lowers Attack and Special Attack of all enemies |
| Odor Sleuth | Normal | 2 | Makes Ghost-type enemies able to be hit by Normal/Fighting moves |
| Spore | Grass | 2 | Puts all enemies in room to sleep |
| Powder Snow | Ice | 4 | Damages all enemies, may freeze |
| Growl | Normal | 2 | Lowers Attack of all enemies |
| Heat Wave | Fire | 10 | Damages all enemies, may burn |
| Warp | None | 2 | Warps all enemies to random locations |
| Slow Down | None | 2 | Lowers speed of all enemies |
| Petrify | None | 2 | Petrifies all enemies |
| Siesta | None | 2 | Puts all enemies to sleep |
| Totter | None | 2 | Confuses all enemies |
| Two Edge | None | 2 | Damages all enemies and user |
| Vacuum Cut | None | 10 | Damages all enemies |
| Famish | None | 2 | Lowers belly of all enemies |

### MOVE_RANGE_ROOM_ALLIES (7 moves)
Affects all allies in the room (including user)

| Move | Type | Power | Description |
|------|------|-------|-------------|
| Aromatherapy | Grass | 2 | Cures all status conditions of allies |
| Heal Bell | Normal | 2 | Cures all status conditions of allies |
| Uproar | Normal | 10 | Damages enemies and prevents sleep for allies |
| Safeguard | Normal | 2 | Protects all allies from status conditions |
| Softboiled | Normal | 2 | Heals all allies |
| Refresh | Normal | 2 | Cures status conditions of all allies |
| Speed Boost | None | 2 | Raises Speed of all allies |

### MOVE_RANGE_ROOM_EXCEPT_USER (3 moves)
Affects all Pokemon in the room except the user

| Move | Type | Power | Description |
|------|------|-------|-------------|
| Earthquake | Ground | 5 | Damages all Pokemon in room except user |
| Magnitude | Ground | 2 | Damages all Pokemon in room except user with variable power |
| Silver Wind | Bug | 4 | Damages all Pokemon in room except user, chance to raise all user's stats |

### MOVE_RANGE_ROOM_ONLY (1 move)
Special room-targeting mechanics

| Move | Type | Power | Description |
|------|------|-------|-------------|
| Helping Hand | Normal | 2 | Boosts power of allies' next attacks |

## Strategy Notes

### Offensive Room-Wide Moves
- **Heat Wave** (Power 10) is the strongest room-wide damaging move
- **Vacuum Cut** (Power 10) is a powerful room-wide move with no type
- **Earthquake** (Power 5) damages all Pokemon except the user, making it safe for the user but dangerous for allies

### Defensive Room-Wide Moves
- **Aromatherapy**, **Heal Bell**, and **Refresh** all cure status conditions for all allies
- **Safeguard** prevents status conditions from being applied to allies
- **Softboiled** provides room-wide healing

### Stat Modification
- **Speed Boost** raises Speed for all allies
- **Growl** lowers Attack for all enemies
- **Memento** drastically lowers Attack and Special Attack of all enemies (but causes user to faint)
- **Silver Wind** has a chance to raise all of the user's stats

### Status Moves
- **Spore** and **Siesta** put all enemies to sleep (100% accuracy for Spore)
- **Totter** confuses all enemies
- **Petrify** petrifies all enemies
- **Warp** warps all enemies to random locations

### Utility Moves
- **Haze** removes all stat changes from all Pokemon
- **Baton Pass** allows switching while preserving stat changes
- **Odor Sleuth** makes Ghost-types vulnerable to Normal/Fighting moves
- **Famish** lowers the belly of all enemies

## Combat Applications

Room-wide moves are particularly effective in:
1. **Boss fights with minions** - Moves like Heat Wave, Silver Wind, or Earthquake can damage multiple enemies simultaneously
2. **Monster Houses** - Room-wide status moves like Spore or Totter can disable entire groups of enemies
3. **Team support** - Aromatherapy and Safeguard keep your entire team buffed or healed
4. **Crowd control** - Warp, Petrify, and sleep moves can neutralize threats

Be cautious with moves like Earthquake that damage allies, and avoid Haze if you've built up beneficial stat changes.
