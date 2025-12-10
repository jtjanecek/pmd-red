# Stalling Moves

This document lists all moves that cause stalling behavior - moves that prevent, delay, or impair the opponent's ability to take actions.

## Move Categories

### Infatuation/Attract (1 move)
Causes the target to become infatuated, potentially preventing them from attacking.

| Move | Type | Range | Power | AI Weight |
|------|------|-------|-------|-----------|
| Attract | Normal | MOVE_RANGE_CUTS_CORNERS_FOE | 2 | 20 |

**Effect:** Causes infatuation, which can prevent the target from taking actions.

---

### Sleep Moves (7 moves)
Puts the target to sleep, preventing all actions until they wake up.

| Move | Type | Range | Power | AI Weight |
|------|------|-------|-------|-----------|
| Hypnosis | Psychic | MOVE_RANGE_CUTS_CORNERS_FOE | 2 | 15 |
| Grasswhistle | Grass | MOVE_RANGE_CUTS_CORNERS_FOE | 2 | 15 |
| Sleep Powder | Grass | MOVE_RANGE_ONE_TILE_FOE | 2 | 15 |
| Spore | Grass | MOVE_RANGE_FRONT_FOE | 2 | 10 |
| Lovely Kiss | Normal | MOVE_RANGE_ONE_TILE_FOE | 0 | 15 |
| Sing | Normal | MOVE_RANGE_CUTS_CORNERS_FOE | 2 | 15 |
| Rest | Psychic | MOVE_RANGE_USER | 2 | 30 |

**Effect:** Sleep prevents all actions. Most sleep moves have reduced accuracy.
- **Spore** has the highest accuracy (100%) but now only affects the foe directly in front
- **Rest** puts the user to sleep to heal HP

---

### Freeze Moves (7 moves)
Freezes the target, preventing movement and actions.

| Move | Type | Range | Power | AI Weight |
|------|------|-------|-------|-----------|
| Blizzard | Ice | MOVE_RANGE_CUTS_CORNERS_FOE | 24 | 20 |
| Ice Beam | Ice | MOVE_RANGE_STRAIGHT_LINE | 12 | 20 |
| Ice Punch | Ice | MOVE_RANGE_FRONT_FOE | 14 | 20 |
| Powder Snow | Ice | MOVE_RANGE_ROOM_EXCEPT_USER | 4 | 20 |
| Ice Ball | Ice | MOVE_RANGE_CUTS_CORNERS_FOE | 1 | 10 |
| Icy Wind | Ice | MOVE_RANGE_FRONT_FOE | 8 | 10 |
| Aurora Beam | Ice | MOVE_RANGE_STRAIGHT_LINE | 8 | 10 |

**Effect:** Freeze prevents all actions. These moves have a chance to freeze on hit.
- **Blizzard** is the strongest freeze move
- **Powder Snow** now affects all Pokemon except the user

---

### Confusion Moves (7 moves)
Confuses the target, causing them to potentially hurt themselves or use random moves.

| Move | Type | Range | Power | AI Weight |
|------|------|-------|-------|-----------|
| Confuse Ray | Ghost | MOVE_RANGE_FRONT_FOE | 2 | 15 |
| Confusion | Psychic | MOVE_RANGE_FRONT_FOE | 5 | 10 |
| Dizzy Punch | Normal | MOVE_RANGE_FRONT_FOE | 6 | 20 |
| Psybeam | Psychic | MOVE_RANGE_STRAIGHT_LINE | 12 | 10 |
| Signal Beam | Bug | MOVE_RANGE_STRAIGHT_LINE | 14 | 10 |
| Supersonic | Normal | MOVE_RANGE_CUTS_CORNERS_FOE | 2 | 15 |
| Sweet Kiss | Normal | MOVE_RANGE_FRONT_FOE | 2 | 15 |

**Effect:** Confusion causes erratic behavior - the Pokemon may hurt itself or use random moves.
- Damaging moves have a chance to confuse
- Pure confusion moves guarantee the effect (if they hit)

---

### Constriction/Wrap Moves (5 moves)
Traps and damages the target over multiple turns, preventing movement.

| Move | Type | Range | Power | AI Weight |
|------|------|-------|-------|-----------|
| Bind | Normal | MOVE_RANGE_FRONT_FOE | 4 | 20 |
| Clamp | Water | MOVE_RANGE_FRONT_FOE | 6 | 20 |
| Constrict | Normal | MOVE_RANGE_FRONT_FOE | 2 | 30 |
| Sand Tomb | Ground | MOVE_RANGE_FRONT_FOE | 4 | 20 |
| Wrap | Normal | MOVE_RANGE_FRONT_FOE | 4 | 15 |

**Effect:** Constriction prevents the target from moving and deals damage over time.
- Target is trapped and takes damage each turn
- Effect lasts for several turns
- Target cannot move but can still attack

---

### Petrification (1 move)
Turns the target to stone, completely immobilizing them.

| Move | Type | Range | Power | AI Weight |
|------|------|-------|-------|-----------|
| Petrify | None | MOVE_RANGE_ROOM_FOES | 2 | 8 |

**Effect:** Petrification completely prevents all actions and movement. Very powerful stalling effect.
- Affects all enemies in the room
- One of the strongest stalling effects in the game

---

### Yawn (Delayed Sleep) (1 move)
Causes the target to fall asleep after a delay.

| Move | Type | Range | Power | AI Weight |
|------|------|-------|-------|-----------|
| Yawn | Normal | MOVE_RANGE_FRONT_FOE | 4 | 15 |

**Effect:** Target becomes drowsy and falls asleep after a few turns.
- Delayed effect - gives opponent warning
- Difficult to avoid once inflicted

---

## Strategy Notes

### Most Reliable Stalling Moves
1. **Spore** - 100% accuracy sleep (now front-foe only)
2. **Petrify** - Room-wide petrification
3. **Confuse Ray** - Guaranteed confusion
4. **Attract** - High AI weight (20), causes infatuation

### Room-Wide Stalling
- **Petrify** - Only room-wide pure stalling move
- **Powder Snow** - Room-wide with freeze chance (now hits all Pokemon except user)

### Most Damaging Stalling Moves
1. **Blizzard** (24 power) - Freeze chance
2. **Signal Beam** (14 power) - Confusion chance
3. **Ice Punch** (14 power) - Freeze chance
4. **Psybeam** (12 power) - Confusion chance

### AI Behavior
Moves with higher `aiWeight` are more likely to be used by AI:
- **Constrict** (30) - Highest AI priority
- **Rest** (30) - AI will use to heal
- **Attract** (20) - High priority infatuation
- **Bind, Clamp, Dizzy Punch, Ice moves** (20) - High priority

### Counters and Resistance
- **Vital Spirit ability** - Prevents sleep
- **Own Tempo ability** - Prevents confusion
- **Oblivious ability** - Prevents infatuation
- **Ice-type Pokemon** - Cannot be frozen
- **Grass-type Pokemon** - Immune to spore moves

### Combo Potential
1. **Petrify + Room-wide damage** - Immobilize all enemies then damage them
2. **Confuse Ray + Constrict** - Confuse then trap
3. **Yawn + Switching** - Force sleep then switch to advantage
4. **Attract + High damage moves** - Prevent counter-attacks while dealing damage

## Balance Changes
Recent changes to prevent stalling abuse:
- **Spore**: Changed from MOVE_RANGE_ROOM_FOES to MOVE_RANGE_FRONT_FOE (front foe only)
- **Powder Snow**: Reduced power from 4 to 4, changed to MOVE_RANGE_ROOM_EXCEPT_USER
- **Silver Wind**: Reduced power from 12 to 4, changed to MOVE_RANGE_ROOM_EXCEPT_USER
- **Heat Wave**: Reduced power from 10 to 4, changed to MOVE_RANGE_ROOM_EXCEPT_USER
- **Agility**: Changed from MOVE_RANGE_ROOM_ALLIES to MOVE_RANGE_USER (user only)
