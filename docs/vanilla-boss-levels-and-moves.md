# Vanilla Boss Levels & Moves

Levels and moves for every vanilla story and postgame boss fight. Levels are pulled from each boss floor’s `pokemon_found.json` entry in `data/dungeon/<dungeon>/pokemon_found.json` (scripted boss rows). Moves are the last four level-up moves the species knows at that level from `data/monster/learnset/learnset_data.json` (same logic the game uses when spawning enemies).

## Story bosses
- **Skarmory** — Mt Steel 9F — Lv10 — `Peck`, `Leer`, `Swift`, `Sand Attack`
- **Zapdos** — Mt Thunder Peak 3F — Lv25 — `Peck`, `Thundershock`, `Thunder Wave`, `Agility`
- **Moltres** — Mt Blaze Peak 3F — Lv32 — `Wing Attack`, `Ember`, `Fire Spin`, `Agility`
- **Articuno** — Frosty Grotto 5F — Lv33 — `Gust`, `Powder Snow`, `Mist`, `Agility`
- **Groudon** — Magma Cavern Pit 3F — Lv27 — `Mud Shot`, `AncientPower`, `Scary Face`, `Slash`
- **Rayquaza** — Sky Tower Summit 9F — Lv35 — `Scary Face`, `Dragon Claw`, `Dragon Dance`, `Crunch`

## Postgame bosses
- **Entei** — Fiery Field — Lv45 — `Ember`, `Roar`, `Fire Spin`, `Stomp`
- **Raikou** — Lightning Field — Lv45 — `Thundershock`, `Roar`, `Quick Attack`, `Spark`
- **Suicune** — Northwind Field — Lv45 — `BubbleBeam`, `Rain Dance`, `Gust`, `Aurora Beam`
- **Regirock** — Buried Relic (first golem) — Lv18 — `Explosion`, `Rock Throw`, `Curse`
- **Regice** — Buried Relic — Lv23 — `Explosion`, `Icy Wind`, `Curse`
- **Registeel** — Buried Relic — Lv25 — `Explosion`, `Metal Claw`, `Curse`, `Superpower`
- **Kyogre** — Stormy Sea — Lv25 — `Water Pulse`, `AncientPower`, `Scary Face`, `Body Slam`
- **Ho-Oh** — Mt Faraway — Lv50 — `Gust`, `Safeguard`, `Recover`, `Fire Blast`
- **Lugia** — Silver Trench — Lv30 — `Whirlwind`, `Gust`, `Safeguard`
- **Deoxys** — Meteor Cave — Lv35 — `Pursuit`, `Psychic`, `Snatch`, `Cosmic Power`
- **Mewtwo** — Western Cave — Lv40 — `Confusion`, `Swift`, `Barrier`, `Mist`
- **Jirachi** — Wish Cave — Lv40 — `Refresh`, `Rest`, `Double-Edge`, `Future Sight`
- **Celebi** — Purity Forest — Lv45 — `Heal Bell`, `Safeguard`, `Future Sight`, `Baton Pass`
- **Mew** — Buried Relic (roaming) — Lv40 — `Mega Punch`, `Transform`, `Metronome`, `Psychic`
- **Latias** — Northern Range — Lv28 — `Wish`, `Safeguard`, `DragonBreath`, `Water Sport`
- **Latios** — Northern Range — Lv30 — `Safeguard`, `DragonBreath`, `Protect`, `Refresh`

## Notes
- Regi floors in Buried Relic use separate boss entries per golem; levels above reflect the scripted fights (first appearance of each golem).
- Purity Forest and Buried Relic encounters (Celebi, Mew) are treated as bosses even though they use special spawn rules; their levels/moves still come from the same data sources.
