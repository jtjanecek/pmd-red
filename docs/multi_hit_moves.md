# Multi-Hit Moves

This document lists all moves that hit multiple times in a single turn in Pokemon Mystery Dungeon: Red Rescue Team.

## Move Categories

### Fixed 5-Hit Moves (2 moves)
These moves always hit 5 times consecutively, continuing until they miss.

| Move | Type | Power (per hit) | Range | AI Weight | Notes |
|------|------|-----------------|-------|-----------|-------|
| Ice Ball | Ice | 1 | MOVE_RANGE_CUTS_CORNERS_FOE | 10 | Hits 5 times or until miss |
| Rollout | Rock | 1 | MOVE_RANGE_FRONT_FOE | 15 | Hits 5 times or until miss |

**Total Damage:** Up to 5 damage (if all hits land)

---

### Fixed 3-Hit Moves (2 moves)
These moves always hit exactly 3 times.

| Move | Type | Power (per hit) | Range | AI Weight | Notes |
|------|------|-----------------|-------|-----------|-------|
| Thrash | Normal | 18 | MOVE_RANGE_SIDE | 15 | Hits 3 times, causes confusion after |
| Triple Kick | Fighting | 8 | MOVE_RANGE_FRONT_FOE | 25 | Hits 3 times with increasing power |

**Total Damage:**
- Thrash: 54 damage (18 × 3)
- Triple Kick: 24 damage (8 × 3)

---

### Fixed 2-Hit Moves (4 moves)
These moves always hit exactly twice.

| Move | Type | Power (per hit) | Range | AI Weight | Notes |
|------|------|-----------------|-------|-----------|-------|
| Bonemerang | Ground | 10 | MOVE_RANGE_STRAIGHT_LINE | 10 | Hits twice in a line |
| Double Kick | Fighting | 6 | MOVE_RANGE_FRONT_FOE | 20 | Hits twice |
| Fury Cutter | Bug | 2 | MOVE_RANGE_FRONT_FOE | 15 | Hits twice, power increases |
| Twineedle | Bug | 6 | MOVE_RANGE_STRAIGHT_LINE | 30 | Hits twice, may poison |

**Total Damage:**
- Bonemerang: 20 damage (10 × 2)
- Double Kick: 12 damage (6 × 2)
- Fury Cutter: 4 damage (2 × 2) initially
- Twineedle: 12 damage (6 × 2)

---

### Random Multi-Hit Moves (2-5 hits) (7 moves)
These moves hit a random number of times (typically 2-5 hits).

| Move | Type | Power (per hit) | Range | AI Weight | Notes |
|------|------|-----------------|-------|-----------|-------|
| Barrage | Normal | 1 | MOVE_RANGE_CUTS_CORNERS_FOE | 10 | Random 2-5 hits |
| Bone Rush | Ground | 12 | MOVE_RANGE_CUTS_CORNERS_FOE | 25 | Random 2-5 hits |
| Bullet Seed | Grass | 10 | MOVE_RANGE_STRAIGHT_LINE | 10 | Random 2-5 hits |
| Comet Punch | Normal | 3 | MOVE_RANGE_FRONT_FOE | 30 | Random 2-5 hits |
| Doubleslap | Normal | 4 | MOVE_RANGE_FRONT_FOE | 15 | Random 2-5 hits |
| Fury Attack | Normal | 1 | MOVE_RANGE_FRONT_FOE | 20 | Random 2-5 hits |
| Fury Swipes | Normal | 4 | MOVE_RANGE_FRONT_FOE | 25 | Random 2-5 hits |
| Icicle Spear | Ice | 2 | MOVE_RANGE_CUTS_CORNERS_FOE | 20 | Random 2-5 hits |
| Spike Cannon | Normal | 3 | MOVE_RANGE_STRAIGHT_LINE | 10 | Random 2-5 hits |

**Average Damage (assuming 3.5 hits average):**
- Barrage: ~3.5 damage (1 × 3.5)
- Bone Rush: ~42 damage (12 × 3.5) **Highest average**
- Bullet Seed: ~35 damage (10 × 3.5)
- Comet Punch: ~10.5 damage (3 × 3.5)
- Doubleslap: ~14 damage (4 × 3.5)
- Fury Attack: ~3.5 damage (1 × 3.5)
- Fury Swipes: ~14 damage (4 × 3.5)
- Icicle Spear: ~7 damage (2 × 3.5)
- Spike Cannon: ~10.5 damage (3 × 3.5)

---

## Strategy and Analysis

### Most Powerful Multi-Hit Moves
1. **Thrash** (54 total) - Highest guaranteed damage, but causes confusion
2. **Bone Rush** (~42 average) - Best random multi-hit move
3. **Bullet Seed** (~35 average) - Strong ranged multi-hit
4. **Triple Kick** (24 total) - Solid fixed 3-hit

### Highest AI Priority
Multi-hit moves that AI uses most frequently (by AI Weight):
1. **Comet Punch** (30) - Random 2-5 hits
2. **Twineedle** (30) - Fixed 2 hits + poison chance
3. **Constrict** (30) - Not multi-hit, but for comparison
4. **Triple Kick** (25) - Fixed 3 hits
5. **Bone Rush** (25) - Random 2-5 hits
6. **Fury Swipes** (25) - Random 2-5 hits

### Range Advantages
- **Straight Line:** Bonemerang, Bullet Seed, Spike Cannon, Twineedle
- **Cuts Corners:** Barrage, Bone Rush, Ice Ball, Icicle Spear
- **Front Foe:** Most others (close range)
- **Side:** Thrash (unique side-hitting)

### Special Properties

#### Ice Ball & Rollout
- Hit 5 times consecutively
- Stop if they miss
- Power increases with each consecutive hit in the series
- Can be boosted by Defense Curl

#### Thrash
- Extremely powerful (18 damage per hit, 3 hits)
- User becomes confused after using
- Risky but high reward

#### Fury Cutter
- Power increases with consecutive successful uses
- Resets if it misses
- Becomes very powerful over time

#### Twineedle
- Fixed 2 hits
- Each hit has a chance to poison
- High AI weight (30) makes it commonly used

### Skill Link Ability
Some Pokemon may have the Skill Link ability, which guarantees multi-hit moves always hit the maximum number of times (5 hits for random multi-hit moves).

## Damage Comparison

**If all hits land (assuming 5 hits for random moves):**

| Move | Minimum Damage | Maximum Damage | Average Damage |
|------|----------------|----------------|----------------|
| Thrash | 54 | 54 | 54 |
| Bone Rush | 24 (2 hits) | 60 (5 hits) | 42 |
| Bullet Seed | 20 (2 hits) | 50 (5 hits) | 35 |
| Triple Kick | 24 | 24 | 24 |
| Bonemerang | 20 | 20 | 20 |
| Doubleslap | 8 (2 hits) | 20 (5 hits) | 14 |
| Fury Swipes | 8 (2 hits) | 20 (5 hits) | 14 |
| Double Kick | 12 | 12 | 12 |
| Twineedle | 12 | 12 | 12 |
| Comet Punch | 6 (2 hits) | 15 (5 hits) | 10.5 |
| Spike Cannon | 6 (2 hits) | 15 (5 hits) | 10.5 |
| Icicle Spear | 4 (2 hits) | 10 (5 hits) | 7 |
| Ice Ball | 1 (miss early) | 5 (5 hits) | 5 |
| Rollout | 1 (miss early) | 5 (5 hits) | 5 |
| Fury Cutter | 4 | 4 | 4 |
| Barrage | 2 (2 hits) | 5 (5 hits) | 3.5 |
| Fury Attack | 2 (2 hits) | 5 (5 hits) | 3.5 |

## Combat Applications

### Boss Fights
- **Thrash** deals massive damage but risky due to confusion
- **Bone Rush** and **Bullet Seed** provide excellent sustained damage

### Multi-Enemy Situations
- Straight-line moves (Bonemerang, Bullet Seed) can hit multiple enemies
- Ice Ball and Barrage cut corners to hit around obstacles

### Breaking Through Substitutes
- Multi-hit moves are excellent for breaking through Substitute/Focus Sash effects
- Each hit triggers separately

### Type Coverage
- **Fighting type:** Double Kick, Triple Kick
- **Ground type:** Bonemerang, Bone Rush
- **Grass type:** Bullet Seed
- **Bug type:** Fury Cutter, Twineedle
- **Ice type:** Ice Ball, Icicle Spear
- **Normal type:** Many options for neutral coverage
