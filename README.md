# Pokémon Mystery Dungeon: Red Rescue Team

[![build](https://github.com/pret/pmd-red/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/pret/pmd-red/actions/workflows/build.yml)

This is a decompilation of Pokémon Mystery Dungeon: Red Rescue Team.

It builds the following rom:

* pmd_red.gba `sha1: 9f4cfc5b5f4859d17169a485462e977c7aac2b89`

To set up the repository, see [INSTALL.md](INSTALL.md).

For contacts and other pret projects, see [pret.github.io](https://pret.github.io/).

# Changelog
## beta-v0.0.3 (in progress)
* Update bossrooms to require minions to be killed for stair spawn

## beta-v0.0.2
* Basic shinies added (5%, fixed palette)
* Increase max elixir spawns
* Remove healing moves from bosses
* Lower chance of missing kecleon
* Unable to enter friend area fixed
* Changed autopilot hotkey to R+A
* Add auto-leader swap to R+B

## beta-v0.0.1
* Difficulty settings -> Normal, Hard, Nightmare (this affects many things)
* All Friend areas are auto-unlocked
* Players select a single item from 5 randomized items to start the journey
* Play as any pokemon from the start, and any partner (including legendaries if you’d like)
* Recruitment no longer tied to levels, base % chance based on difficulty selected
* Weather effects are implemented in boss fights for certain bosses (based on difficulty as well)
* Weather now appears in dungeons that should have weather (e.g. sometimes rain in water dungeons for example)
* Option to pick the randomization seed or randomize it so that you can test different seeds, or attempt to beat another person’s seed
* Recruitment can be set to “never recruit” if you want an extra challenge
* Interactable NPCs between dungeons to give hints for money, or buy a pokemon recruit
* NPC to delete moves (like splash) so that you can use struggle more effectively
* Move fixes so that every pokemon is viable at Level 1
* Item limits on entering dungeons based on difficulty
* “Super trap” rooms: Rooms that exist that have a huge amount of traps (not overdoing this don’t worry). You can pay an NPC money to learn which floor is “Super trapped”
* Kecleon shops that sometimes have no NPC shopkeeper. This means that you will have to strategize if you want to steal, as you cannot pay (more likely on Nightmare)
* Bosses have customized movesets so that they aren’t pushovers. HP based on difficulty
* Loot drops on Boss defeats (get better over time as you beat harder dungeons)per
* Silver wind, heat wave, powder snow have reduced damage and now damage allies.
* Spore now hits foes in front. Agility only affects the user.
* Attract only has 2 turn effect.
* Bullet seed, bone rush, doubleslap, fury swipes power decreased
* HMs now function as normal TMs and can be found in-dungeon
* Frustration and Return now both do a flat 30 damage (nerf frustration, buff return)
* Regular Attack is now strongest of physical/special (typeless still)
* Munchlax and decoy (glitch pokemon) are playable. I’ve created movesets for them that I think are appropriate
* Limited total # of recruitable pokemon at a time based on difficulty
* Added “Outer rooms” dungeon generation to be included in dungeons (previously only used in unused dungeons, so not seen in vanilla gameplay)
* Dungeon generation now includes more secondary mazes (maze/plus/checker/pool/island/divider) 
* Dungeon generation now much more randomized than vanilla dungeons, removed “tiny” dungeon floors (e.g. 3 rooms)
* Visibility chance affected by difficulty selection
* Item spawn stickiness based on difficulty selection
* All pokemon now have at least 2 attacking moves at level 1 (including ditto)
* Remoraid, spheal, sealeo, remoraid, mantine, magikarp, gyrados, ho-oh, unown, feebas, milotic, azumarill, raichu now get extra XP after each pokemon because they have very high EXP needed to level up
* Increased EXP gain by ~20% to decrease exp grinds
* Players select a single item to start from a randomly generated list
* Set all item sell/buy prices in shops to be 4000 if they had prices over 4000. For example fire blast is now 4000 instead of 9000. Banana price set to 50

