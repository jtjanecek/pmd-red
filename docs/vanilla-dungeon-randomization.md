# Vanilla Dungeon Randomization Seeds

How vanilla PMD: Red builds random dungeon floors. Data is pulled from `data/dungeon/main_data.inc` (the floor generation seeds) and `data/dungeon/*/floor_id.json` (which floors reference which seed). Numbers below ignore runtime overrides; they reflect base ROM behavior.

## Floor generation knobs
- `layout` chooses the generator: Small/Medium/Large grids use the standard room+hallway flow; other layouts are bespoke (outer ring, line, beetle, etc.) handled in `GenerateFloor()`/`src/dungeon_generation.c:280-379`.
- `roomDensity` is the target room count before validity culls: negative = exact value, positive = that value plus a random 0-2 rooms; always at least 2 survive. See `AssignRooms()`.
- `floorConnectivity` controls extra random-walk hallway links across the grid (`AssignRandomGridCellConnections`).
- `numExtraHallways` adds free-form random walks after the main connections are built (`GenerateExtraHallways`).
- `allowDeadEnds` lets the generator keep dead-end corridors; if false, extra hallways try to avoid dead ends.
- `secondaryStructuresBudget` caps how many secondary terrain formations (maze/plus/checker/pool/island/divider) can be stamped into rooms.
- `roomFlags` toggles whether rooms allow secondary terrain (`0x1`) and imperfections (`0x4`).
- `kecleonShopChance` / `monsterHouseChance` / `mazeRoomChance` are per-floor percentages checked during generation.
- Other fields that influence layouts: `fixedRoomNumber` (forces a fixed map), `visibilityRange`, `itemDensity`, `trapDensity`, `standaloneLakeDensity`, `buriedItemDensity`, and `moneyUpperBound` (money cap x40).

## Layout behaviors (vanilla)
- `Large` / `Large (0x8 cap)`: Random grid 2-8 x 2-7 (capped to 6x4 overall; 0x8 uses 2-5 x 2-4) feeding the standard room/hallway generator.
- `Medium` / `Small`: Standard generator on a 4 x (2-3) grid; marked as medium/small for enemy/item scaling logic.
- `One-room Monster House`: Single-room layout that forces a monster house.
- `Two rooms + Monster House`: Two-room layout with a forced monster house.
- `Outer ring`: 6x4 grid where the perimeter is hallways and the 4x2 center cells become rooms.
- `Crossroads`: Plus-shaped hallway spine with rooms on each arm (6x4 grid with preset anchors).
- `Line`: Straight line of rooms connected by halls.
- `Cross`: Four cardinal arms around a central hub.
- `Beetle`: Symmetric layout with three horizontal arms (resembles a beetle).
- `Outer rooms`: Outer perimeter rooms on a variable grid; standard connections inside.
- `Unused 0xC-0xF`: Present in data but the switch falls back to the Large handler unless overridden.

## Seeds used per dungeon (floor ranges that share a seed)
| Dungeon | Floors | Seed ID | Layout | Rooms | Conn. | Extra halls | Dead ends | Shops/MH% | Secondary budget | Room flags |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Autopilot | 1F | 1129 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 2F | 1130 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 3F | 1131 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 4F | 1132 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 5F | 1133 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 6F | 1134 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 7F | 1135 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 8F | 1136 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 9F | 1137 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Autopilot | 10F | 1138 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0 | none |
| Boss3 | 1F | 449 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 2F | 450 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 3F | 451 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 4F | 452 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 5F | 453 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 6F | 454 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 7F | 455 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 8F | 456 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 9F | 457 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 10F | 458 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 11F | 459 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 12F | 460 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 13F | 461 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 14F | 462 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 15F | 463 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 16F | 464 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 17F | 465 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 18F | 466 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 19F | 467 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss3 | 20F | 468 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1 | none |
| Boss4 | 1F | 469 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss4 | 2F | 470 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss4 | 3F | 471 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss4 | 4F | 472 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| Boss4 | 5F | 473 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| Boss4 | 6F | 474 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0 | Secondary terrain |
| Boss4 | 7F | 475 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0 | Secondary terrain |
| Boss4 | 8F | 476 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0 | Secondary terrain |
| Boss4 | 9F | 477 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0 | Secondary terrain |
| Boss4 | 10F | 478 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0 | Secondary terrain |
| Boss4 | 11F | 479 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0 | Secondary terrain |
| Boss9 | 1F | 728 | Unused 0xF | 15-17 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| Boss9 | 2F | 729 | Unused 0xF | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| Boss9 | 3F | 730 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 4F | 731 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 5F | 732 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 6F | 733 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 7F | 734 | Unused 0xF | 15-17 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 8F | 735 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 9F | 736 | Unused 0xF | 15-17 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 10F | 737 | Unused 0xF | 15-17 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 11F | 738 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| Boss9 | 12F | 739 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| BuriedRelic | 1F | 579 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| BuriedRelic | 2F | 580 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| BuriedRelic | 3F | 581 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| BuriedRelic | 4F | 582 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| BuriedRelic | 5F | 583 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/0% | 0 | Secondary terrain |
| BuriedRelic | 6F | 584 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| BuriedRelic | 7F | 585 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| BuriedRelic | 8F | 586 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| BuriedRelic | 9F | 587 | Small | 5-7 rooms (random) | 50 | 15 | yes | 0%/0% | 0 | none |
| BuriedRelic | 10F | 588 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| BuriedRelic | 11F | 589 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| BuriedRelic | 12F | 590 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| BuriedRelic | 13F | 591 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| BuriedRelic | 14F | 592 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 15F | 593 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 16F | 594 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 17F | 595 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 18F | 596 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 19F | 597 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 20F | 598 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 21F | 599 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 22F | 600 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 23F | 601 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 24F | 602 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 25F | 603 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 26F | 604 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 27F | 605 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 28F | 606 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 29F | 607 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 30F | 608 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 31F | 609 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 32F | 610 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 33F | 611 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 34F | 612 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 35F | 613 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 36F | 614 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 37F | 615 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 38F | 616 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 39F | 617 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 40F | 618 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 41F | 619 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 42F | 620 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 43F | 621 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 44F | 622 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 45F | 623 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 46F | 624 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 47F | 625 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 48F | 626 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 49F | 627 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 50F | 628 | Small | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 51F | 629 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 52F | 630 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 53F | 631 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 54F | 632 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 55F | 633 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 56F | 634 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 57F | 635 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 58F | 636 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 59F | 637 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 60F | 638 | Large | 8-10 rooms (random) | 15 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 61F | 639 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 62F | 640 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 63F | 641 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 64F | 642 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 65F | 643 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 66F | 644 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 67F | 645 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 68F | 646 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 69F | 647 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 70F | 648 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 71F | 649 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 72F | 650 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 73F | 651 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 74F | 652 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 75F | 653 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 76F | 654 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 77F | 655 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 78F | 656 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 79F | 657 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 80F | 658 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 81F | 659 | Medium | 8-10 rooms (random) | 10 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 82F | 660 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 83F | 661 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 84F | 662 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 85F | 663 | Small | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 86F | 664 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 87F | 665 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 88F | 666 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 89F | 667 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 90F | 668 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 91F | 669 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 92F | 670 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 93F | 671 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 94F | 672 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 95F | 673 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 96F | 674 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| BuriedRelic | 97F | 675 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| BuriedRelic | 98F | 676 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | Secondary terrain |
| BuriedRelic | 99F | 677 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 0 | none |
| D50 | 1F | 1139 | Large | 8-10 rooms (random) | 20 | 0 | no | 0%/0% | 0 | none |
| D50 | 2F | 1140 | Large | 8-10 rooms (random) | 20 | 0 | no | 0%/0% | 0 | none |
| D51 | 1F | 1141 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| D51 | 2F | 1142 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D51 | 3F | 1143 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D51 | 4F | 1144 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 5F | 1145 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 6F | 1146 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| D51 | 7F | 1147 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 8F | 1148 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 9F | 1149 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D51 | 10F | 1150 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 11F | 1151 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 12F | 1152 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D51 | 13F | 1153 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 14F | 1154 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 15F | 1155 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D51 | 16F | 1156 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| D51 | 17F | 1157 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D51 | 18F | 1158 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D51 | 19F | 1159 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 20F | 1160 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D51 | 21F | 1161 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| D51 | 22F | 1162 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 23F | 1163 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 24F | 1164 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D51 | 25F | 1165 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 26F | 1166 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 27F | 1167 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D51 | 28F | 1168 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 29F | 1169 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 30F | 1170 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D51 | 31F | 1171 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 32F | 1172 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D51 | 33F | 1173 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D51 | 34F | 1174 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 35F | 1175 | One-room Monster House | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D51 | 36F | 1176 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| D51 | 37F | 1177 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 38F | 1178 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 39F | 1179 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D51 | 40F | 1180 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 41F | 1181 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 42F | 1182 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D51 | 43F | 1183 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 44F | 1184 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D51 | 45F | 1185 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D51 | 46F | 1186 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| D51 | 47F | 1187 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D51 | 48F | 1188 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D51 | 49F | 1189 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D51 | 50F | 1190 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D54 | 1F | 1272 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 2F | 1273 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 3F | 1274 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 4F | 1275 | Large | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 5F | 1276 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 6F | 1277 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 7F | 1278 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 8F | 1279 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 9F | 1280 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 10F | 1281 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 11F | 1282 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 12F | 1283 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 13F | 1284 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 14F | 1285 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 15F | 1286 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 16F | 1287 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 17F | 1288 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 18F | 1289 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 19F | 1290 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 20F | 1291 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 21F | 1292 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 22F | 1293 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 23F | 1294 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 24F | 1295 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 25F | 1296 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 26F | 1297 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 27F | 1298 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 28F | 1299 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 29F | 1300 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D54 | 30F | 1301 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0 | none |
| D61 | 1F | 1565 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| D61 | 2F | 1566 | Large | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D61 | 3F | 1567 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 4F | 1568 | Large | 11-13 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| D61 | 5F | 1569 | Large | 12-14 rooms (random) | 40 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 6F | 1570 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| D61 | 7F | 1571 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 8F | 1572 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 9F | 1573 | Large | 8-10 rooms (random) | 35 | 15 | yes | 0%/0% | 0 | none |
| D61 | 10F | 1574 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| D61 | 11F | 1575 | Large | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 12F | 1576 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 13F | 1577 | Large | 11-13 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| D61 | 14F | 1578 | Large | 12-14 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D61 | 15F | 1579 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 16F | 1580 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 17F | 1581 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 18F | 1582 | Large | 8-10 rooms (random) | 35 | 15 | yes | 0%/0% | 0 | none |
| D61 | 19F | 1583 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 20F | 1584 | Large | 9-11 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D61 | 21F | 1585 | Large | 10-12 rooms (random) | 10 | 15 | yes | 0%/0% | 0 | none |
| D61 | 22F | 1586 | Large | 11-13 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 23F | 1587 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 24F | 1588 | Large | 11-13 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 25F | 1589 | Large | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D61 | 26F | 1590 | Large | 9-11 rooms (random) | 30 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 27F | 1591 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 28F | 1592 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 29F | 1593 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| D61 | 30F | 1594 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 31F | 1595 | Large | 12-14 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 32F | 1596 | Large | 11-13 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D61 | 33F | 1597 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 34F | 1598 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 35F | 1599 | Large | 8-10 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D61 | 36F | 1600 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| D61 | 37F | 1601 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 38F | 1602 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 39F | 1603 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 40F | 1604 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 41F | 1605 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 42F | 1606 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 43F | 1607 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 44F | 1608 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 45F | 1609 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 46F | 1610 | Large | 12-14 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| D61 | 47F | 1611 | Large | 11-13 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D61 | 48F | 1612 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 49F | 1613 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 50F | 1614 | Large | 8-10 rooms (random) | 40 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 51F | 1615 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| D61 | 52F | 1616 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 53F | 1617 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 54F | 1618 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 55F | 1619 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 56F | 1620 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 57F | 1621 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 58F | 1622 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 59F | 1623 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| D61 | 60F | 1624 | Large | 9-11 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| D61 | 61F | 1625 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 62F | 1626 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 63F | 1627 | Large | 12-14 rooms (random) | 40 | 15 | yes | 0%/0% | 0 | none |
| D61 | 64F | 1628 | Large | 11-13 rooms (random) | 30 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 65F | 1629 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 66F | 1630 | Large | 9-11 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 67F | 1631 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 68F | 1632 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 69F | 1633 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 70F | 1634 | Large | 11-13 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 71F | 1635 | Large | 12-14 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 72F | 1636 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 73F | 1637 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 74F | 1638 | Large | 9-11 rooms (random) | 40 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 75F | 1639 | Large | 8-10 rooms (random) | 10 | 15 | yes | 0%/0% | 0 | none |
| D61 | 76F | 1640 | Large | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| D61 | 77F | 1641 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 78F | 1642 | Large | 11-13 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 79F | 1643 | Large | 12-14 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D61 | 80F | 1644 | Large | 11-13 rooms (random) | 30 | 15 | no | 0%/0% | 0 | none |
| D61 | 81F | 1645 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 82F | 1646 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 83F | 1647 | Large | 8-10 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| D61 | 84F | 1648 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 85F | 1649 | Large | 9-11 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| D61 | 86F | 1650 | Large | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 87F | 1651 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 88F | 1652 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 89F | 1653 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 90F | 1654 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| D61 | 91F | 1655 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 92F | 1656 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 93F | 1657 | Large | 9-11 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 94F | 1658 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 95F | 1659 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 96F | 1660 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D61 | 97F | 1661 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| D61 | 98F | 1662 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| D61 | 99F | 1663 | Large | 9-11 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| D63 | 1F | 1763 | Large | 6-8 rooms (random) | 40 | 20 | no | 0%/0% | 1 | none |
| DarknightRelic | 1F | 950 | Large | 5-7 rooms (random) | 10 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 2F | 951 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 3F | 952 | Large | 8-10 rooms (random) | 50 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 4F | 953 | Large | 4-6 rooms (random) | 50 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 5F | 954 | Large | 12-14 rooms (random) | 10 | 15 | no | 0%/10% | 0 | Secondary terrain |
| DarknightRelic | 6F | 955 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/10% | 0 | none |
| DarknightRelic | 7F | 956 | Large | 10-12 rooms (random) | 50 | 15 | no | 0%/10% | 0 | none |
| DarknightRelic | 8F | 957 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/10% | 0 | none |
| DarknightRelic | 9F | 958 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 10F | 959 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/5% | 0 | Secondary terrain |
| DarknightRelic | 11F | 960 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 12F | 961 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 13F | 962 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 14F | 963 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| DarknightRelic | 15F | 964 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0 | Secondary terrain |
| DesertRegion | 1F | 740 | Medium | 6-8 rooms (random) | 40 | 10 | no | 6%/3% | 0 | none |
| DesertRegion | 2F | 741 | Medium | 7-9 rooms (random) | 40 | 10 | yes | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 3F | 742 | Medium | 8-10 rooms (random) | 40 | 10 | no | 6%/3% | 0 | none |
| DesertRegion | 4F | 743 | Medium | 9-11 rooms (random) | 40 | 10 | no | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 5F | 744 | Large | 10-12 rooms (random) | 40 | 10 | yes | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 6F | 745 | Large | 11-13 rooms (random) | 40 | 10 | no | 6%/3% | 0 | none |
| DesertRegion | 7F | 746 | Large | 12-14 rooms (random) | 40 | 10 | no | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 8F | 747 | Large | 13-15 rooms (random) | 40 | 10 | yes | 6%/3% | 0 | none |
| DesertRegion | 9F | 748 | Cross | 14-16 rooms (random) | 40 | 10 | no | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 10F | 749 | Cross | 15-17 rooms (random) | 40 | 15 | no | 6%/3% | 0 | none |
| DesertRegion | 11F | 750 | Large | 6-8 rooms (random) | 40 | 15 | yes | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 12F | 751 | Cross | 7-9 rooms (random) | 40 | 15 | no | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 13F | 752 | Large | 8-10 rooms (random) | 40 | 15 | yes | 6%/3% | 0 | none |
| DesertRegion | 14F | 753 | Cross | 9-11 rooms (random) | 40 | 15 | yes | 6%/3% | 0 | none |
| DesertRegion | 15F | 754 | Large | 10-12 rooms (random) | 40 | 15 | no | 6%/3% | 0 | none |
| DesertRegion | 16F | 755 | Large | 11-13 rooms (random) | 40 | 15 | no | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 17F | 756 | Unused 0xF | 12-14 rooms (random) | 40 | 15 | yes | 6%/3% | 0 | none |
| DesertRegion | 18F | 757 | Large | 13-15 rooms (random) | 40 | 15 | no | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 19F | 758 | Unused 0xF | 14-16 rooms (random) | 40 | 15 | yes | 6%/3% | 0 | Secondary terrain |
| DesertRegion | 20F | 759 | Large | 15-17 rooms (random) | 30 | 15 | no | 6%/3% | 0 | none |
| DojoRegistration | 1F | 1191 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 2F | 1192 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 3F | 1193 | Medium | 9-11 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 4F | 1194 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 5F | 1195 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 6F | 1196 | Medium | 12-14 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 7F | 1197 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 8F | 1198 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 9F | 1199 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 10F | 1200 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 11F | 1201 | Medium | 15-17 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 12F | 1202 | Medium | 10-12 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 13F | 1203 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 14F | 1204 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 15F | 1205 | Medium | 9-11 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 16F | 1206 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 17F | 1207 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 18F | 1208 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 19F | 1209 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 20F | 1210 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 21F | 1211 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 22F | 1212 | Medium | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 23F | 1213 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 24F | 1214 | Medium | 20-22 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 25F | 1215 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 26F | 1216 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 27F | 1217 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 28F | 1218 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 29F | 1219 | Medium | 15-17 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 30F | 1220 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 31F | 1221 | Medium | 9-11 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 32F | 1222 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 33F | 1223 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 34F | 1224 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 35F | 1225 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 36F | 1226 | Medium | 15-17 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 37F | 1227 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 38F | 1228 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 39F | 1229 | Medium | 10-12 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 40F | 1230 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 41F | 1231 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 42F | 1232 | Medium | 12-14 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 43F, 49F | 1233 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 44F | 1234 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 45F | 1235 | Medium | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 46F | 1236 | Medium | 7-9 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 47F | 1237 | Medium | 15-17 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 48F | 1238 | Medium | 10-12 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 50F | 1239 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 51F | 1240 | Medium | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 52F | 1241 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| DojoRegistration | 53F | 1242 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 54F | 1243 | Medium | 9-11 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 55F | 1244 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 56F | 1245 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 57F | 1246 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 58F | 1247 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 59F | 1248 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 60F | 1249 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 61F, 64F | 1250 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 62F, 65F | 1251 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 63F | 1252 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 66F | 1253 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 67F | 1254 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| DojoRegistration | 68F | 1255 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| DojoRegistration | 69F | 1256 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| FantasyStrait | 1F | 1091 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 10 | none |
| FantasyStrait | 2F | 1092 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 3F | 1093 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 4F | 1094 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 5F | 1095 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 10 | Secondary terrain |
| FantasyStrait | 6F | 1096 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 0 | none |
| FantasyStrait | 7F | 1097 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 8F | 1098 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 9F | 1099 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/10% | 10 | none |
| FantasyStrait | 10F | 1100 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 11F | 1101 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 12F | 1102 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 13F | 1103 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| FantasyStrait | 14F | 1104 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 15F | 1105 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 16F | 1106 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 17F | 1107 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 10 | none |
| FantasyStrait | 18F | 1108 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| FantasyStrait | 19F | 1109 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 20F | 1110 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 21F | 1111 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 10 | none |
| FantasyStrait | 22F | 1112 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 23F | 1113 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 24F | 1114 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 25F | 1115 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| FantasyStrait | 26F | 1116 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| FantasyStrait | 27F | 1117 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/10% | 0 | none |
| FantasyStrait | 28F | 1118 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| FantasyStrait | 29F | 1119 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| FantasyStrait | 30F | 1120 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| FantasyStraitAlt | 1F | 1302 | Unused 0xE | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 2F | 1303 | Unused 0xE | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 3F | 1304 | Unused 0xE | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 4F | 1305 | Unused 0xE | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 5F | 1306 | Unused 0xE | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 6F | 1307 | Unused 0xE | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 7F | 1308 | Large | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 8F | 1309 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 9F | 1310 | Large | 15-17 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 10F | 1311 | Large | 11-13 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 11F | 1312 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 12F | 1313 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 13F | 1314 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 14F | 1315 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 15F | 1316 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 16F | 1317 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 17F | 1318 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 18F | 1319 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FantasyStraitAlt | 19F | 1320 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FaroffSea | 1F | 1450 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FaroffSea | 2F | 1451 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| FaroffSea | 3F | 1452 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| FaroffSea | 4F | 1453 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FaroffSea | 5F | 1454 | Small | 6-8 rooms (random) | 40 | 15 | no | 0%/0% | 1 | Secondary terrain |
| FaroffSea | 6F | 1455 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| FaroffSea | 7F | 1456 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| FaroffSea | 8F | 1457 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FaroffSea | 9F | 1458 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| FaroffSea | 10F | 1459 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 1 | none |
| FaroffSea | 11F | 1460 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| FaroffSea | 12F | 1461 | Large | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| FaroffSea | 13F | 1462 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| FaroffSea | 14F | 1463 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/100% | 0 | none |
| FaroffSea | 15F | 1464 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/6% | 1 | Secondary terrain |
| FaroffSea | 16F | 1465 | Large | 6-8 rooms (random) | 10 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 17F | 1466 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 18F | 1467 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 19F | 1468 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 20F | 1469 | Small | 8-10 rooms (random) | 40 | 15 | no | 0%/6% | 1 | none |
| FaroffSea | 21F | 1470 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 22F | 1471 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 23F | 1472 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 24F | 1473 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 25F | 1474 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 1 | none |
| FaroffSea | 26F | 1475 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 27F | 1476 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 28F | 1477 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 29F | 1478 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/100% | 0 | none |
| FaroffSea | 30F | 1479 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1 | Secondary terrain |
| FaroffSea | 31F | 1480 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 32F | 1481 | Large | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 33F | 1482 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 34F | 1483 | Small | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 35F | 1484 | Small | 5-7 rooms (random) | 40 | 15 | no | 0%/6% | 1 | none |
| FaroffSea | 36F | 1485 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 37F | 1486 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 38F | 1487 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 39F | 1488 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 40F | 1489 | Large | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 1 | none |
| FaroffSea | 41F | 1490 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 42F | 1491 | Medium | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 43F | 1492 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 44F | 1493 | Large | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 45F | 1494 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1 | Secondary terrain |
| FaroffSea | 46F | 1495 | Medium | 5-7 rooms (random) | 10 | 15 | no | 0%/100% | 0 | none |
| FaroffSea | 47F | 1496 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 48F | 1497 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 49F | 1498 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 50F | 1499 | Small | 5-7 rooms (random) | 40 | 15 | no | 0%/6% | 1 | Secondary terrain |
| FaroffSea | 51F | 1500 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 52F | 1501 | Small | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 53F | 1502 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 54F | 1503 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 55F | 1504 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 1 | none |
| FaroffSea | 56F | 1505 | Large | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 57F | 1506 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 58F | 1507 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 59F | 1508 | Medium | 5-7 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 60F | 1509 | Large | 8-10 rooms (random) | 15 | 15 | yes | 0%/6% | 1 | none |
| FaroffSea | 61F | 1510 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 62F | 1511 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 63F | 1512 | Medium | 8-10 rooms (random) | 40 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 64F | 1513 | Large | 8-10 rooms (random) | 30 | 15 | no | 0%/100% | 0 | Secondary terrain |
| FaroffSea | 65F | 1514 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 1 | none |
| FaroffSea | 66F | 1515 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 67F | 1516 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 68F | 1517 | Large | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 69F | 1518 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 70F | 1519 | Small | 5-7 rooms (random) | 20 | 15 | no | 0%/6% | 1 | Secondary terrain |
| FaroffSea | 71F | 1520 | Medium | 5-7 rooms (random) | 15 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 72F | 1521 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 0 | none |
| FaroffSea | 73F | 1522 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0 | none |
| FaroffSea | 74F | 1523 | Medium | 8-10 rooms (random) | 40 | 15 | no | 0%/6% | 0 | Secondary terrain |
| FaroffSea | 75F | 1524 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1 | none |
| FieryField | 1F | 840 | Medium | 4-6 rooms (random) | 15 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 2F | 841 | Medium | 8-10 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 3F | 842 | Medium | 6-8 rooms (random) | 13 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 4F | 843 | Medium | 5-7 rooms (random) | 15 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 5F | 844 | Medium | 4-6 rooms (random) | 10 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 6F | 845 | Medium | 8-10 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 7F | 846 | Medium | 9-11 rooms (random) | 10 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 8F | 847 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 9F | 848 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 10F | 849 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 11F | 850 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 12F | 851 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 13F | 852 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 14F | 853 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 15F | 854 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 16F | 855 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 17F | 856 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 18F | 857 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 19F | 858 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 20F | 859 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 21F | 860 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 22F | 861 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 23F | 862 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 24F | 863 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 25F | 864 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 26F | 865 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 27F | 866 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 28F | 867 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 29F | 868 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FieryField | 30F | 869 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| FrostyForest | 1F | 93 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 8%/0% | 0 | none |
| FrostyForest | 2F | 94 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 8%/0% | 0 | none |
| FrostyForest | 3F | 95 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 8%/0% | 0 | none |
| FrostyForest | 4F | 96 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 7%/0% | 0 | Secondary terrain |
| FrostyForest | 5F | 97 | Large | 6-8 rooms (random) | 20 | 15 | yes | 7%/0% | 0 | Secondary terrain |
| FrostyForest | 6F | 98 | Large | 6-8 rooms (random) | 20 | 15 | yes | 7%/0% | 0 | Secondary terrain |
| FrostyForest | 7F | 99 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 6%/0% | 0 | none |
| FrostyForest | 8F | 100 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 6%/0% | 0 | none |
| FrostyForest | 9F | 101 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 6%/0% | 0 | none |
| FrostyGrotto | 1F | 102 | Medium | 6-8 rooms (random) | 20 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| FrostyGrotto | 2F | 103 | Medium | 6-8 rooms (random) | 25 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| FrostyGrotto | 3F | 104 | Large | 8-10 rooms (random) | 30 | 40 | yes | 0%/0% | 0 | none |
| FrostyGrotto | 4F | 105 | Large | 8-10 rooms (random) | 20 | 40 | yes | 0%/0% | 0 | none |
| FrostyGrotto | 5F | 106 | Large | 8-10 rooms (random) | 10 | 40 | yes | 0%/0% | 0 | none |
| GrandSea | 1F | 996 | Small | 4-6 rooms (random) | 15 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 2F | 997 | Small | 5-7 rooms (random) | 15 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 3F | 998 | Small | 6-8 rooms (random) | 15 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 4F | 999 | Small | 7-9 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 5F | 1000 | Small | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 6F | 1001 | Small | 9-11 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 7F | 1002 | Small | 10-12 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 8F | 1003 | Small | 11-13 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 9F | 1004 | Small | 12-14 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 10F | 1005 | Small | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 11F | 1006 | Small | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 12F | 1007 | Small | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 13F | 1008 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 14F | 1009 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 15F | 1010 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 16F | 1011 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 17F | 1012 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 18F | 1013 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 19F | 1014 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 20F | 1015 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 21F | 1016 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 22F | 1017 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 23F | 1018 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 24F | 1019 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 25F | 1020 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 26F | 1021 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 27F | 1022 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 28F | 1023 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 29F | 1024 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GrandSea | 30F | 1025 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| GreatCanyon | 1F | 52 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 2F | 53 | Medium | 5-7 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 3F | 54 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 4F | 55 | Medium | 5-7 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 5F | 56 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 6F | 57 | Large | 12-14 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 7F | 58 | Large | 5-7 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 8F | 59 | Unused 0xD | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 9F | 60 | Unused 0xD | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 10F | 61 | Large | 8-10 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 11F | 62 | Large | 8-10 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| GreatCanyon | 12F | 63 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| HowlingForest | 1F | 1257 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| HowlingForest | 2F | 1258 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| HowlingForest | 3F | 1259 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| HowlingForest | 4F | 1260 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0 | none |
| HowlingForest | 5F | 1261 | Medium | 8-10 rooms (random) | 20 | 20 | no | 0%/5% | 0 | Secondary terrain |
| HowlingForest | 6F | 1262 | Medium | 8-10 rooms (random) | 30 | 20 | yes | 0%/5% | 1 | none |
| HowlingForest | 7F | 1263 | Medium | 8-10 rooms (random) | 30 | 20 | yes | 0%/5% | 1 | none |
| HowlingForest | 8F | 1264 | Medium | 10-12 rooms (random) | 30 | 30 | no | 0%/5% | 0 | Secondary terrain |
| HowlingForest | 9F | 1265 | Medium | 10-12 rooms (random) | 30 | 30 | yes | 0%/5% | 1 | Secondary terrain |
| HowlingForest | 10F | 1266 | Medium | 10-12 rooms (random) | 30 | 30 | no | 0%/5% | 1 | Secondary terrain |
| HowlingForest | 11F | 1267 | Medium | 10-12 rooms (random) | 30 | 30 | yes | 0%/5% | 0 | Secondary terrain |
| HowlingForest | 12F | 1268 | Medium | 10-12 rooms (random) | 30 | 40 | yes | 0%/5% | 1 | none |
| HowlingForest | 13F | 1269 | Medium | 10-12 rooms (random) | 30 | 40 | no | 0%/5% | 1 | Secondary terrain |
| HowlingForest | 14F | 1270 | Medium | 10-12 rooms (random) | 30 | 40 | yes | 0%/5% | 0 | none |
| HowlingForest | 15F | 1271 | Medium | 10-12 rooms (random) | 30 | 40 | yes | 0%/5% | 1 | none |
| JoyousTower | 1F | 1351 | Small | 8-10 rooms (random) | 10 | 15 | no | 8%/0% | 0 | none |
| JoyousTower | 2F | 1352 | Small | 9-11 rooms (random) | 15 | 15 | no | 8%/0% | 0 | none |
| JoyousTower | 3F | 1353 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/0% | 0 | none |
| JoyousTower | 4F | 1354 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/0% | 0 | Secondary terrain |
| JoyousTower | 5F | 1355 | Small | 12-14 rooms (random) | 40 | 15 | no | 8%/0% | 0 | none |
| JoyousTower | 6F | 1356 | Medium | 11-13 rooms (random) | 30 | 15 | yes | 8%/0% | 0 | none |
| JoyousTower | 7F | 1357 | Large | 10-12 rooms (random) | 20 | 15 | no | 40%/0% | 0 | none |
| JoyousTower | 8F | 1358 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| JoyousTower | 9F | 1359 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/5% | 0 | none |
| JoyousTower | 10F | 1360 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| JoyousTower | 11F | 1361 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| JoyousTower | 12F | 1362 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | Secondary terrain |
| JoyousTower | 13F | 1363 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/5% | 0 | none |
| JoyousTower | 14F | 1364 | Large | 12-14 rooms (random) | 20 | 15 | no | 8%/40% | 0 | none |
| JoyousTower | 15F | 1365 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 16F | 1366 | Small | 10-12 rooms (random) | 10 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 17F | 1367 | Small | 9-11 rooms (random) | 15 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 18F | 1368 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 19F | 1369 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 20F | 1370 | Medium | 6-8 rooms (random) | 40 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 21F | 1371 | Large | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 22F | 1372 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 23F | 1373 | Small | 12-14 rooms (random) | 20 | 15 | no | 40%/6% | 0 | none |
| JoyousTower | 24F | 1374 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 25F | 1375 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 26F | 1376 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 27F | 1377 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 28F | 1378 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 29F | 1379 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/40% | 0 | none |
| JoyousTower | 30F | 1380 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 31F | 1381 | Small | 12-14 rooms (random) | 10 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 32F | 1382 | Small | 11-13 rooms (random) | 15 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 33F | 1383 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 34F | 1384 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 35F | 1385 | Medium | 6-8 rooms (random) | 40 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 36F | 1386 | Medium | 11-13 rooms (random) | 30 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 37F | 1387 | Small | 12-14 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 38F | 1388 | Small | 11-13 rooms (random) | 20 | 15 | no | 40%/6% | 0 | none |
| JoyousTower | 39F | 1389 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 40F | 1390 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 41F | 1391 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 42F | 1392 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 43F | 1393 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 44F | 1394 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 45F | 1395 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 46F | 1396 | Small | 12-14 rooms (random) | 10 | 15 | no | 8%/40% | 0 | none |
| JoyousTower | 47F | 1397 | Small | 11-13 rooms (random) | 15 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 48F | 1398 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 49F | 1399 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 50F | 1400 | Small | 6-8 rooms (random) | 40 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 51F | 1401 | Small | 8-10 rooms (random) | 30 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 52F | 1402 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 53F | 1403 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 54F | 1404 | Medium | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 55F | 1405 | Small | 12-14 rooms (random) | 20 | 15 | no | 40%/6% | 0 | none |
| JoyousTower | 56F | 1406 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 57F | 1407 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 58F | 1408 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 59F | 1409 | Small | 8-10 rooms (random) | 10 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 60F | 1410 | Small | 9-11 rooms (random) | 15 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 61F | 1411 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 62F | 1412 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 63F | 1413 | Small | 6-8 rooms (random) | 40 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 64F | 1414 | Small | 11-13 rooms (random) | 30 | 15 | no | 8%/40% | 0 | Secondary terrain |
| JoyousTower | 65F | 1415 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 66F | 1416 | Small | 9-11 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 67F | 1417 | Small | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 68F | 1418 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 69F | 1419 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 70F | 1420 | Medium | 11-13 rooms (random) | 10 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 71F | 1421 | Small | 12-14 rooms (random) | 15 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 72F | 1422 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 73F | 1423 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 74F | 1424 | Small | 9-11 rooms (random) | 40 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 75F | 1425 | Small | 8-10 rooms (random) | 30 | 15 | yes | 40%/6% | 0 | none |
| JoyousTower | 76F | 1426 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 77F | 1427 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 78F | 1428 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 79F | 1429 | Small | 12-14 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 80F | 1430 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 81F | 1431 | Small | 10-12 rooms (random) | 10 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 82F | 1432 | Small | 9-11 rooms (random) | 15 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 83F | 1433 | Small | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 84F | 1434 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 85F | 1435 | Small | 9-11 rooms (random) | 40 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 86F | 1436 | Small | 10-12 rooms (random) | 30 | 15 | no | 8%/40% | 0 | none |
| JoyousTower | 87F | 1437 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 88F | 1438 | Small | 12-14 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 89F | 1439 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 90F | 1440 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 91F | 1441 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 92F | 1442 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 93F | 1443 | Small | 9-11 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| JoyousTower | 94F | 1444 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 95F | 1445 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 96F | 1446 | Small | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| JoyousTower | 97F | 1447 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 98F | 1448 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| JoyousTower | 99F | 1449 | Small | 9-11 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| LapisCave | 1F | 64 | Small | 6-8 rooms (random) | 25 | 10 | yes | 0%/0% | 0 | none |
| LapisCave | 2F | 65 | Small | 6-8 rooms (random) | 25 | 10 | yes | 0%/0% | 0 | none |
| LapisCave | 3F | 66 | Small | 6-8 rooms (random) | 25 | 10 | yes | 0%/0% | 0 | none |
| LapisCave | 4F | 67 | Small | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | none |
| LapisCave | 5F | 68 | Small | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| LapisCave | 6F | 69 | Small | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| LapisCave | 7F | 70 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| LapisCave | 8F | 71 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| LapisCave | 9F | 72 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | none |
| LapisCave | 10F | 73 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 0 | none |
| LapisCave | 11F | 74 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 0 | none |
| LapisCave | 12F | 75 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 0 | none |
| LapisCave | 13F | 76 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 0 | none |
| LapisCave | 14F | 77 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 0 | Secondary terrain |
| LightningField | 1F | 920 | Small | 6-8 rooms (random) | 10 | 10 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 2F | 921 | Small | 8-10 rooms (random) | 5 | 5 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 3F | 922 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 4F | 923 | Small | 8-10 rooms (random) | 30 | 10 | no | 0%/0% | 0 | none |
| LightningField | 5F | 924 | Small | 9-11 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| LightningField | 6F | 925 | Small | 10-12 rooms (random) | 6 | 20 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 7F | 926 | Medium | 5-7 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| LightningField | 8F | 927 | Medium | 9-11 rooms (random) | 16 | 35 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 9F | 928 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 10F | 929 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 11F | 930 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 12F | 931 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| LightningField | 13F | 932 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| LightningField | 14F | 933 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 15F | 934 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 16F | 935 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 17F | 936 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| LightningField | 18F | 937 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 19F | 938 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 20F | 939 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| LightningField | 21F | 940 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 22F | 941 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 23F | 942 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| LightningField | 24F | 943 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 25F | 944 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 26F | 945 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| LightningField | 27F | 946 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 28F | 947 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| LightningField | 29F | 948 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| LightningField | 30F | 949 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| MagmaCavern | 1F | 127 | Large | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| MagmaCavern | 2F | 128 | Large | 7-9 rooms (random) | 20 | 15 | no | 5%/0% | 0 | none |
| MagmaCavern | 3F | 129 | Medium | 6-8 rooms (random) | 20 | 15 | no | 10%/0% | 0 | none |
| MagmaCavern | 4F | 130 | Medium | 6-8 rooms (random) | 25 | 15 | no | 10%/0% | 0 | none |
| MagmaCavern | 5F | 131 | Large | 8-10 rooms (random) | 20 | 15 | no | 5%/0% | 0 | Secondary terrain |
| MagmaCavern | 6F | 132 | Large | 8-10 rooms (random) | 20 | 15 | no | 5%/0% | 0 | Secondary terrain |
| MagmaCavern | 7F | 133 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MagmaCavern | 8F | 134 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/8% | 0 | Secondary terrain |
| MagmaCavern | 9F | 135 | Medium | 6-8 rooms (random) | 15 | 15 | no | 0%/8% | 0 | Secondary terrain |
| MagmaCavern | 10F | 136 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/8% | 0 | Secondary terrain |
| MagmaCavern | 11F | 137 | Large | 7-9 rooms (random) | 25 | 15 | no | 0%/8% | 0 | Secondary terrain |
| MagmaCavern | 12F | 138 | Large | 7-9 rooms (random) | 25 | 15 | no | 0%/8% | 0 | Secondary terrain |
| MagmaCavern | 13F | 139 | Large | 7-9 rooms (random) | 20 | 15 | no | 0%/8% | 0 | none |
| MagmaCavern | 14F | 140 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/8% | 0 | none |
| MagmaCavern | 15F | 141 | Medium | 6-8 rooms (random) | 15 | 15 | no | 0%/8% | 0 | none |
| MagmaCavern | 16F | 142 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/8% | 0 | none |
| MagmaCavern | 17F | 143 | Large | 8-10 rooms (random) | 25 | 15 | no | 0%/8% | 0 | none |
| MagmaCavern | 18F | 144 | Medium | 6-8 rooms (random) | 25 | 15 | no | 0%/8% | 0 | none |
| MagmaCavern | 19F | 145 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/8% | 0 | Secondary terrain |
| MagmaCavern | 20F | 146 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/8% | 0 | Secondary terrain |
| MagmaCavern | 21F | 147 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MagmaCavern | 22F | 148 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MagmaCavern | 23F | 149 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MagmaCavernPit | 1F | 150 | Unused 0xF | 8-10 rooms (random) | 20 | 10 | no | 0%/0% | 0 | none |
| MagmaCavernPit | 2F | 151 | Unused 0xF | 8-10 rooms (random) | 20 | 10 | yes | 0%/0% | 0 | none |
| MagmaCavernPit | 3F | 152 | Unused 0xF | 7-9 rooms (random) | 20 | 50 | no | 0%/0% | 0 | none |
| MarvelousSea | 1F | 1071 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | none |
| MarvelousSea | 2F | 1072 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| MarvelousSea | 3F | 1073 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| MarvelousSea | 4F | 1074 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| MarvelousSea | 5F | 1075 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | Secondary terrain |
| MarvelousSea | 6F | 1076 | Beetle | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| MarvelousSea | 7F | 1077 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| MarvelousSea | 8F | 1078 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| MarvelousSea | 9F | 1079 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | none |
| MarvelousSea | 10F | 1080 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| MarvelousSea | 11F | 1081 | Beetle | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| MarvelousSea | 12F | 1082 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| MarvelousSea | 13F | 1083 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | Secondary terrain |
| MarvelousSea | 14F | 1084 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| MarvelousSea | 15F | 1085 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| MarvelousSea | 16F | 1086 | Large | 10-12 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| MarvelousSea | 17F | 1087 | Large | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 10 | none |
| MarvelousSea | 18F | 1088 | Beetle | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0 | none |
| MarvelousSea | 19F | 1089 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MarvelousSea | 20F | 1090 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| MeteorCave | 1F | 326 | Large | 8-10 rooms (random) | 40 | 15 | yes | 0%/0% | 0 | none |
| MeteorCave | 2F | 327 | Large | 10-12 rooms (random) | 40 | 15 | yes | 0%/0% | 0 | none |
| MeteorCave | 3F | 328 | Large | 12-14 rooms (random) | 40 | 15 | yes | 0%/0% | 0 | none |
| MeteorCave | 4F | 329 | Large | 15-17 rooms (random) | 40 | 15 | yes | 0%/0% | 0 | none |
| MeteorCave | 5F | 330 | Large | 12-14 rooms (random) | 40 | 20 | yes | 0%/0% | 0 | none |
| MeteorCave | 6F | 331 | Large | 15-17 rooms (random) | 40 | 20 | yes | 0%/0% | 0 | none |
| MeteorCave | 7F | 332 | Large | 20-22 rooms (random) | 40 | 20 | yes | 0%/0% | 0 | none |
| MeteorCave | 8F | 333 | Large | 15-17 rooms (random) | 40 | 30 | yes | 0%/0% | 0 | none |
| MeteorCave | 9F | 334 | Large | 12-14 rooms (random) | 40 | 30 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 10F | 335 | Large | 10-12 rooms (random) | 40 | 30 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 11F | 336 | Large | 8-10 rooms (random) | 40 | 30 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 12F | 337 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 13F | 338 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 14F | 339 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 15F | 340 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 16F | 341 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 17F | 342 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 18F | 343 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 19F | 344 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MeteorCave | 20F | 345 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 0 | Secondary terrain |
| MtBlaze | 1F | 78 | Medium | 6-8 rooms (random) | 15 | 20 | no | 0%/0% | 0 | none |
| MtBlaze | 2F | 79 | Medium | 6-8 rooms (random) | 16 | 20 | no | 0%/0% | 0 | none |
| MtBlaze | 3F | 80 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0 | none |
| MtBlaze | 4F | 81 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0 | Secondary terrain |
| MtBlaze | 5F | 82 | Medium | 6-8 rooms (random) | 20 | 20 | no | 8%/0% | 0 | Secondary terrain |
| MtBlaze | 6F | 83 | Medium | 6-8 rooms (random) | 20 | 20 | no | 8%/0% | 0 | Secondary terrain |
| MtBlaze | 7F | 84 | Medium | 6-8 rooms (random) | 20 | 20 | no | 10%/0% | 0 | Secondary terrain |
| MtBlaze | 8F | 85 | Medium | 6-8 rooms (random) | 20 | 20 | no | 8%/0% | 0 | Secondary terrain |
| MtBlaze | 9F | 86 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0 | Secondary terrain |
| MtBlaze | 10F | 87 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0 | Secondary terrain |
| MtBlaze | 11F | 88 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0 | Secondary terrain |
| MtBlaze | 12F | 89 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0 | Secondary terrain |
| MtBlazePeak | 1F | 90 | Large | 6-8 rooms (random) | 20 | 20 | no | 0%/0% | 0 | none |
| MtBlazePeak | 2F | 91 | Large | 7-9 rooms (random) | 20 | 20 | no | 0%/0% | 0 | none |
| MtBlazePeak | 3F | 92 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0 | none |
| MtFaraway | 1F | 1525 | Medium | 8-10 rooms (random) | 15 | 15 | no | 3%/0% | 0 | none |
| MtFaraway | 2F | 1526 | Medium | 8-10 rooms (random) | 15 | 15 | no | 3%/0% | 0 | none |
| MtFaraway | 3F | 1527 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/0% | 0 | Secondary terrain |
| MtFaraway | 4F | 1528 | Small | 5-7 rooms (random) | 50 | 15 | no | 3%/0% | 0 | none |
| MtFaraway | 5F | 1529 | Small | 5-7 rooms (random) | 40 | 15 | no | 3%/0% | 0 | Secondary terrain |
| MtFaraway | 6F | 1530 | Small | 5-7 rooms (random) | 30 | 15 | yes | 3%/0% | 0 | none |
| MtFaraway | 7F | 1531 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0 | Secondary terrain |
| MtFaraway | 8F | 1532 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0 | none |
| MtFaraway | 9F | 1533 | Small | 8-10 rooms (random) | 20 | 15 | yes | 3%/0% | 0 | none |
| MtFaraway | 10F | 1534 | Small | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0 | none |
| MtFaraway | 11F | 1535 | Small | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0 | Secondary terrain |
| MtFaraway | 12F | 1536 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 3%/0% | 0 | Secondary terrain |
| MtFaraway | 13F | 1537 | Large | 5-7 rooms (random) | 50 | 15 | no | 3%/0% | 0 | none |
| MtFaraway | 14F | 1538 | Medium | 5-7 rooms (random) | 20 | 15 | no | 0%/50% | 0 | none |
| MtFaraway | 15F | 1539 | Small | 8-10 rooms (random) | 20 | 15 | yes | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 16F | 1540 | Small | 8-10 rooms (random) | 10 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 17F | 1541 | Small | 8-10 rooms (random) | 15 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 18F | 1542 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/6% | 0 | none |
| MtFaraway | 19F | 1543 | Medium | 5-7 rooms (random) | 20 | 15 | no | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 20F | 1544 | Medium | 5-7 rooms (random) | 40 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 21F | 1545 | Large | 5-7 rooms (random) | 30 | 15 | yes | 3%/6% | 0 | none |
| MtFaraway | 22F | 1546 | Medium | 15-17 rooms (random) | 20 | 15 | no | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 23F | 1547 | Medium | 15-17 rooms (random) | 50 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 24F | 1548 | Small | 15-17 rooms (random) | 20 | 15 | yes | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 25F | 1549 | Small | 5-7 rooms (random) | 20 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 26F | 1550 | Small | 5-7 rooms (random) | 20 | 15 | no | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 27F | 1551 | Small | 5-7 rooms (random) | 20 | 15 | yes | 3%/6% | 0 | none |
| MtFaraway | 28F | 1552 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 29F | 1553 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/50% | 0 | none |
| MtFaraway | 30F | 1554 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 31F | 1555 | Large | 5-7 rooms (random) | 50 | 15 | no | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 32F | 1556 | Large | 5-7 rooms (random) | 15 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 33F | 1557 | Large | 5-7 rooms (random) | 20 | 15 | yes | 3%/6% | 0 | none |
| MtFaraway | 34F | 1558 | Crossroads | 8-10 rooms (random) | 20 | 15 | no | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 35F | 1559 | Outer ring | 8-10 rooms (random) | 40 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 36F | 1560 | Crossroads | 8-10 rooms (random) | 30 | 15 | yes | 3%/6% | 0 | none |
| MtFaraway | 37F | 1561 | Outer ring | 15-17 rooms (random) | 20 | 15 | no | 3%/6% | 0 | none |
| MtFaraway | 38F | 1562 | Crossroads | 15-17 rooms (random) | 50 | 15 | no | 3%/6% | 0 | Secondary terrain |
| MtFaraway | 39F | 1563 | Outer ring | 15-17 rooms (random) | 20 | 15 | yes | 3%/6% | 0 | none |
| MtFaraway | 40F | 1564 | Small | 8-10 rooms (random) | 20 | 15 | no | 3%/6% | 0 | none |
| MtFreeze | 1F | 107 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 2F | 108 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 3F | 109 | Medium | 6-8 rooms (random) | 25 | 10 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 4F | 110 | Medium | 6-8 rooms (random) | 25 | 10 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 5F | 111 | Medium | 6-8 rooms (random) | 15 | 10 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 6F | 112 | Medium | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 7F | 113 | Medium | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 8F | 114 | Medium | 6-8 rooms (random) | 25 | 20 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 9F | 115 | Large | 6-8 rooms (random) | 15 | 20 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 10F | 116 | Large | 6-8 rooms (random) | 25 | 20 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 11F | 117 | Medium | 6-8 rooms (random) | 25 | 30 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 12F | 118 | Medium | 6-8 rooms (random) | 15 | 30 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 13F | 119 | Medium | 6-8 rooms (random) | 15 | 30 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 14F | 120 | Medium | 6-8 rooms (random) | 25 | 30 | no | 0%/0% | 0 | Secondary terrain |
| MtFreeze | 15F | 121 | Medium | 6-8 rooms (random) | 25 | 30 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeak | 1F | 122 | Medium | 6-8 rooms (random) | 15 | 5 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeak | 2F | 123 | Medium | 6-8 rooms (random) | 20 | 5 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeak | 3F | 124 | Large | 12-14 rooms (random) | 25 | 5 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeak | 4F | 125 | Large | 10-12 rooms (random) | 15 | 10 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeak | 5F | 126 | Large | 11-13 rooms (random) | 20 | 10 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeakAlt | 1F | 346 | Medium | 6-8 rooms (random) | 15 | 0 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeakAlt | 2F | 347 | Medium | 6-8 rooms (random) | 20 | 0 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeakAlt | 3F | 348 | Large | 12-14 rooms (random) | 25 | 0 | no | 0%/0% | 0 | Secondary terrain |
| MtFreezePeakAlt | 4F | 349 | Large | 10-12 rooms (random) | 15 | 0 | no | 0%/0% | 0 | Secondary terrain |
| MtSteel | 1F | 8 | Medium | 9-11 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 2F | 9 | Medium | 9-11 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 3F | 10 | Medium | 8-10 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 4F | 11 | Large (0x8 grid cap) | 8-10 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 5F | 12 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 6F | 13 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 7F | 14 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 8F | 15 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtSteel | 9F | 16 | Large | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0 | none |
| MtThunder | 1F | 39 | Medium | 11-13 rooms (random) | 25 | 10 | no | 0%/0% | 0 | none |
| MtThunder | 2F | 40 | Medium | 10-12 rooms (random) | 20 | 5 | no | 0%/0% | 0 | none |
| MtThunder | 3F | 41 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| MtThunder | 4F | 42 | Large | 8-10 rooms (random) | 30 | 10 | no | 0%/0% | 0 | none |
| MtThunder | 5F | 43 | Large | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| MtThunder | 6F | 44 | Large | 10-12 rooms (random) | 20 | 20 | no | 0%/0% | 0 | none |
| MtThunder | 7F | 45 | Outer ring | 5-7 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| MtThunder | 8F | 46 | Crossroads | 9-11 rooms (random) | 20 | 35 | no | 0%/0% | 0 | none |
| MtThunder | 9F | 47 | Outer ring | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| MtThunder | 10F | 48 | Crossroads | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| MtThunderPeak | 1F | 49 | Large | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MtThunderPeak | 2F | 50 | Large | 5-7 rooms (random) | 22 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MtThunderPeak | 3F | 51 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| MurkyCave | 1F | 977 | Medium | 10-12 rooms (random) | 15 | 10 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 2F | 978 | Medium | 10-12 rooms (random) | 15 | 10 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 3F | 979 | Medium | 10-12 rooms (random) | 15 | 10 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 4F | 980 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 5F | 981 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 6F | 982 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 7F | 983 | Medium | 6-8 rooms (random) | 50 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 8F | 984 | Medium | 9-11 rooms (random) | 20 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 9F | 985 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 10F | 986 | Medium | 11-13 rooms (random) | 20 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 11F | 987 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 12F | 988 | Medium | 11-13 rooms (random) | 25 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 13F | 989 | Medium | 8-10 rooms (random) | 50 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 14F | 990 | Medium | 11-13 rooms (random) | 25 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 15F | 991 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 16F | 992 | Medium | 12-14 rooms (random) | 25 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 17F | 993 | Medium | 14-16 rooms (random) | 25 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 18F | 994 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| MurkyCave | 19F | 995 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/5% | 0 | Secondary terrain |
| NorthernRange | 1F | 703 | Small | 7-9 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 2F | 704 | Small | 8-10 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 3F | 705 | Small | 9-11 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 4F | 706 | Small | 8-10 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 5F | 707 | Small | 10-12 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 6F | 708 | Small | 10-12 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 7F | 709 | Small | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 8F | 710 | Small | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 9F | 711 | Small | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 10F | 712 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 11F | 713 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 12F | 714 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 13F | 715 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 14F | 716 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 15F | 717 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | none |
| NorthernRange | 16F | 718 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | none |
| NorthernRange | 17F | 719 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | none |
| NorthernRange | 18F | 720 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | Secondary terrain |
| NorthernRange | 19F | 721 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 20F | 722 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | none |
| NorthernRange | 21F | 723 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | Secondary terrain |
| NorthernRange | 22F | 724 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | none |
| NorthernRange | 23F | 725 | Large | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | none |
| NorthernRange | 24F | 726 | Large | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 0 | none |
| NorthernRange | 25F | 727 | Large | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1 | none |
| NorthwindField | 1F | 870 | Medium | 5-7 rooms (random) | 8 | 6 | no | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 2F | 871 | Medium | 6-8 rooms (random) | 10 | 8 | no | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 3F | 872 | Medium | 10-12 rooms (random) | 12 | 8 | no | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 4F | 873 | Medium | 3-5 rooms (random) | 10 | 10 | no | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 5F | 874 | Medium | 4-6 rooms (random) | 6 | 10 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 6F | 875 | Medium | 10-12 rooms (random) | 10 | 10 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 7F | 876 | Medium | 6-8 rooms (random) | 8 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 8F | 877 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 9F | 878 | Large | 6-8 rooms (random) | 8 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 10F | 879 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 11F | 880 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 12F | 881 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 13F | 882 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 14F | 883 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 15F | 884 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 16F | 885 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 17F | 886 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 18F | 887 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 19F | 888 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 20F | 889 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 21F | 890 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 22F | 891 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 23F | 892 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 24F | 893 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 25F | 894 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 26F | 895 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 27F | 896 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 28F | 897 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 29F | 898 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| NorthwindField | 30F | 899 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| OddityCave | 1F | 1036 | Medium | 7-9 rooms (random) | 15 | 10 | yes | 0%/0% | 0 | none |
| OddityCave | 2F | 1037 | Medium | 7-9 rooms (random) | 15 | 10 | yes | 0%/0% | 0 | none |
| OddityCave | 3F | 1038 | Medium | 8-10 rooms (random) | 15 | 10 | yes | 0%/0% | 0 | none |
| OddityCave | 4F | 1039 | Medium | 8-10 rooms (random) | 15 | 10 | yes | 0%/0% | 0 | none |
| OddityCave | 5F | 1040 | Medium | 8-10 rooms (random) | 15 | 10 | yes | 0%/0% | 0 | none |
| OddityCave | 6F | 1041 | Small | 6-8 rooms (random) | 20 | 0 | yes | 7%/0% | 0 | none |
| OddityCave | 7F | 1042 | Large | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 0 | none |
| OddityCave | 8F | 1043 | Large | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 0 | none |
| OddityCave | 9F | 1044 | Small | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 0 | none |
| OddityCave | 10F | 1045 | Small | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 0 | Secondary terrain |
| OddityCave | 11F | 1046 | Large | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| OddityCave | 12F | 1047 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| OddityCave | 13F | 1048 | Large | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| OddityCave | 14F | 1049 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| OddityCave | 15F | 1050 | Small | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| PitfallValley | 1F | 678 | Medium | 6-8 rooms (random) | 20 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PitfallValley | 2F | 679 | Medium | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 3F | 680 | Medium | 6-8 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 4F | 681 | Medium | 10-12 rooms (random) | 20 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 5F | 682 | Medium | 12-14 rooms (random) | 30 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PitfallValley | 6F | 683 | Medium | 10-12 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 7F | 684 | Medium | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 8F | 685 | Medium | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 9F | 686 | Medium | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PitfallValley | 10F | 687 | Medium | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 11F | 688 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 12F | 689 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PitfallValley | 13F | 690 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 14F | 691 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 15F | 692 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PitfallValley | 16F | 693 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 17F | 694 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 18F | 695 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PitfallValley | 19F | 696 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 20F | 697 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 21F | 698 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PitfallValley | 22F | 699 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 23F | 700 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 24F | 701 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| PitfallValley | 25F | 702 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| PurityForest | 1F | 1664 | Medium | 6-8 rooms (random) | 10 | 15 | no | 8%/0% | 0 | none |
| PurityForest | 2F | 1665 | Medium | 7-9 rooms (random) | 15 | 15 | no | 8%/0% | 0 | none |
| PurityForest | 3F | 1666 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/0% | 0 | Secondary terrain |
| PurityForest | 4F | 1667 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/0% | 0 | none |
| PurityForest | 5F | 1668 | Medium | 10-12 rooms (random) | 40 | 15 | no | 8%/0% | 0 | Secondary terrain |
| PurityForest | 6F | 1669 | Medium | 10-12 rooms (random) | 30 | 15 | yes | 8%/5% | 0 | none |
| PurityForest | 7F | 1670 | Medium | 9-11 rooms (random) | 20 | 15 | no | 50%/5% | 0 | Secondary terrain |
| PurityForest | 8F | 1671 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| PurityForest | 9F | 1672 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | none |
| PurityForest | 10F | 1673 | Medium | 6-8 rooms (random) | 50 | 15 | no | 8%/5% | 0 | none |
| PurityForest | 11F | 1674 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| PurityForest | 12F | 1675 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | Secondary terrain |
| PurityForest | 13F | 1676 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| PurityForest | 14F | 1677 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/50% | 0 | none |
| PurityForest | 15F | 1678 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| PurityForest | 16F | 1679 | Small | 6-8 rooms (random) | 10 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 17F | 1680 | Small | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 18F | 1681 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 19F | 1682 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 20F | 1683 | Small | 10-12 rooms (random) | 40 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 21F | 1684 | Small | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 22F | 1685 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 23F | 1686 | Medium | 8-10 rooms (random) | 20 | 15 | no | 50%/6% | 0 | none |
| PurityForest | 24F | 1687 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| PurityForest | 25F | 1688 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 26F | 1689 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 27F | 1690 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 28F | 1691 | Medium | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 29F | 1692 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/50% | 0 | none |
| PurityForest | 30F | 1693 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| PurityForest | 31F | 1694 | Medium | 6-8 rooms (random) | 10 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 32F | 1695 | Medium | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 33F | 1696 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 34F | 1697 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 35F | 1698 | Medium | 10-12 rooms (random) | 40 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 36F | 1699 | Medium | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 37F | 1700 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 38F | 1701 | Medium | 8-10 rooms (random) | 20 | 15 | no | 50%/6% | 0 | Secondary terrain |
| PurityForest | 39F | 1702 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 40F | 1703 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 41F | 1704 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 42F | 1705 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 43F | 1706 | Medium | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 44F | 1707 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 45F | 1708 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| PurityForest | 46F | 1709 | Medium | 6-8 rooms (random) | 10 | 15 | no | 8%/50% | 0 | none |
| PurityForest | 47F | 1710 | Medium | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 48F | 1711 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 49F | 1712 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 50F | 1713 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 51F | 1714 | Large | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 52F | 1715 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 53F | 1716 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 54F | 1717 | Large | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 55F | 1718 | Large | 6-8 rooms (random) | 20 | 15 | no | 50%/6% | 0 | none |
| PurityForest | 56F | 1719 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 57F | 1720 | Large | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| PurityForest | 58F | 1721 | Large | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 59F | 1722 | Large | 9-11 rooms (random) | 10 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 60F | 1723 | Large | 8-10 rooms (random) | 15 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 61F | 1724 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 62F | 1725 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 63F | 1726 | Large | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 64F | 1727 | Large | 8-10 rooms (random) | 30 | 15 | no | 8%/50% | 0 | Secondary terrain |
| PurityForest | 65F | 1728 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 66F | 1729 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| PurityForest | 67F | 1730 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 68F | 1731 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 69F | 1732 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 70F | 1733 | Large | 6-8 rooms (random) | 10 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 71F | 1734 | Large | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 72F | 1735 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 73F | 1736 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 74F | 1737 | Large | 10-12 rooms (random) | 40 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 75F | 1738 | Large | 10-12 rooms (random) | 30 | 15 | yes | 50%/6% | 0 | none |
| PurityForest | 76F | 1739 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 77F | 1740 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 78F | 1741 | Large | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | Secondary terrain |
| PurityForest | 79F | 1742 | Large | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 80F | 1743 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 81F | 1744 | Large | 12-14 rooms (random) | 10 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 82F | 1745 | Large | 10-12 rooms (random) | 15 | 15 | no | 8%/6% | 0 | Secondary terrain |
| PurityForest | 83F | 1746 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 84F | 1747 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 85F | 1748 | Large | 8-10 rooms (random) | 40 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 86F | 1749 | Large | 8-10 rooms (random) | 30 | 15 | no | 8%/50% | 0 | none |
| PurityForest | 87F | 1750 | Large | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 88F | 1751 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 89F | 1752 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 90F | 1753 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 91F | 1754 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 92F | 1755 | Large | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 93F | 1756 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 94F | 1757 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 95F | 1758 | Large | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 96F | 1759 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| PurityForest | 97F | 1760 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 98F | 1761 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0 | none |
| PurityForest | 99F | 1762 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 0 | none |
| RemainsIsland | 1F | 1051 | Large | 6-8 rooms (random) | 20 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 2F | 1052 | Large | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 3F | 1053 | Large | 6-8 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 4F | 1054 | Large | 10-12 rooms (random) | 20 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 5F | 1055 | Large | 12-14 rooms (random) | 30 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 6F | 1056 | Large | 10-12 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 7F | 1057 | Large | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 8F | 1058 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 9F | 1059 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 10F | 1060 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 11F | 1061 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 12F | 1062 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 13F | 1063 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 14F | 1064 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 0 | Secondary terrain |
| RemainsIsland | 15F | 1065 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 16F | 1066 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 17F | 1067 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 18F | 1068 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 19F | 1069 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RemainsIsland | 20F | 1070 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 10 | Secondary terrain |
| RockPath | 1F | 1121 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| RockPath | 2F | 1122 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| RockPath | 3F | 1123 | Small | 7-9 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| RockPath | 4F | 1124 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SilentChasm | 1F | 30 | Unused 0xD | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| SilentChasm | 2F | 31 | Unused 0xD | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| SilentChasm | 3F | 32 | Unused 0xD | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| SilentChasm | 4F | 33 | Unused 0xD | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | none |
| SilentChasm | 5F | 34 | Unused 0xD | 8-10 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| SilentChasm | 6F | 35 | Large | 7-9 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| SilentChasm | 7F | 36 | Large | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| SilentChasm | 8F | 37 | Large | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| SilentChasm | 9F | 38 | Large | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0 | Secondary terrain |
| SilverTrench | 1F | 227 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 2F | 228 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 3F | 229 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 4F | 230 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 5F | 231 | Small | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 6F | 232 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 7F | 233 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 8F | 234 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 9F | 235 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/10% | 10 | none |
| SilverTrench | 10F | 236 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 11F | 237 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 12F | 238 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 13F | 239 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 14F | 240 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 15F | 241 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 16F | 242 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 17F | 243 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 18F | 244 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 19F | 245 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 20F | 246 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 21F | 247 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 10 | none |
| SilverTrench | 22F | 248 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 23F | 249 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 24F | 250 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 25F | 251 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 26F | 252 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 27F | 253 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/10% | 0 | none |
| SilverTrench | 28F | 254 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 29F | 255 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 30F | 256 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 31F | 257 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 32F | 258 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 33F | 259 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 10 | none |
| SilverTrench | 34F | 260 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 35F | 261 | One-room Monster House | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 36F | 262 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 37F | 263 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 38F | 264 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 39F | 265 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 40F | 266 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 41F | 267 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 42F | 268 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/10% | 0 | none |
| SilverTrench | 43F | 269 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 44F | 270 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 45F | 271 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 46F | 272 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 47F | 273 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 48F | 274 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 49F | 275 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 50F | 276 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 51F | 277 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 52F | 278 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 53F | 279 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 54F | 280 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/10% | 0 | none |
| SilverTrench | 55F | 281 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 56F | 282 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/10% | 0 | none |
| SilverTrench | 57F | 283 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 58F | 284 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 59F | 285 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 60F | 286 | Small | 10-12 rooms (random) | 15 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 61F | 287 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 62F | 288 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 63F | 289 | Beetle | 18-20 rooms (random) | 40 | 15 | yes | 0%/20% | 0 | none |
| SilverTrench | 64F | 290 | Beetle | 20-22 rooms (random) | 30 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 65F | 291 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 66F | 292 | Small | 16-18 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 67F | 293 | Small | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 68F | 294 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 69F | 295 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 70F | 296 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 71F | 297 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 72F | 298 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 73F | 299 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/10% | 0 | none |
| SilverTrench | 74F | 300 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 75F | 301 | One-room Monster House | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 76F | 302 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 77F | 303 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 78F | 304 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 79F | 305 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 80F | 306 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 81F | 307 | Small | 8-10 rooms (random) | 10 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 82F | 308 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 83F | 309 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 84F | 310 | Medium | 16-18 rooms (random) | 20 | 15 | yes | 0%/10% | 0 | Secondary terrain |
| SilverTrench | 85F | 311 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 86F | 312 | Beetle | 20-22 rooms (random) | 30 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 87F | 313 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 10 | none |
| SilverTrench | 88F | 314 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 89F | 315 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 90F | 316 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 91F | 317 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 10 | Secondary terrain |
| SilverTrench | 92F | 318 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 93F | 319 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 94F | 320 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 95F | 321 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 10 | none |
| SilverTrench | 96F | 322 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| SilverTrench | 97F | 323 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| SilverTrench | 98F | 324 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| SilverTrench | 99F | 325 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 10 | none |
| SinisterWoods | 1F | 17 | Medium | 7-9 rooms (random) | 20 | 6 | no | 0%/0% | 0 | none |
| SinisterWoods | 2F | 18 | Medium | 6-8 rooms (random) | 20 | 8 | no | 0%/0% | 0 | none |
| SinisterWoods | 3F | 19 | Medium | 7-9 rooms (random) | 20 | 8 | no | 0%/0% | 0 | none |
| SinisterWoods | 4F | 20 | Medium | 7-9 rooms (random) | 20 | 10 | no | 0%/0% | 0 | none |
| SinisterWoods | 5F | 21 | Large | 8-10 rooms (random) | 25 | 10 | yes | 0%/0% | 0 | none |
| SinisterWoods | 6F | 22 | Large | 9-11 rooms (random) | 20 | 10 | yes | 0%/0% | 0 | none |
| SinisterWoods | 7F | 23 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| SinisterWoods | 8F | 24 | Small | 11-13 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| SinisterWoods | 9F | 25 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| SinisterWoods | 10F | 26 | Large | 12-14 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| SinisterWoods | 11F | 27 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| SinisterWoods | 12F | 28 | Large | 12-14 rooms (random) | 25 | 15 | yes | 0%/0% | 0 | none |
| SinisterWoods | 13F | 29 | Large | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| SkyTower | 1F | 153 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/0% | 0 | Secondary terrain |
| SkyTower | 2F | 154 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/0% | 0 | Secondary terrain |
| SkyTower | 3F | 155 | Large | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 4F | 156 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 5F | 157 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 6F | 158 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 7F | 159 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 8F | 160 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 9F | 161 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 10F | 162 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 11F | 163 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 12F | 164 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 13F | 165 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 14F | 166 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 15F | 167 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 16F | 168 | Large | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 17F | 169 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 18F | 170 | Cross | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 19F | 171 | Cross | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 20F | 172 | Cross | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 21F | 173 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 22F | 174 | Large | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0 | Secondary terrain |
| SkyTower | 23F | 175 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 24F | 176 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTower | 25F | 177 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0 | Secondary terrain |
| SkyTowerSummit | 1F | 178 | Line | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 2F | 179 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 3F | 180 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 4F | 181 | Line | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 5F | 182 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 6F | 183 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 7F | 184 | Line | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 8F | 185 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SkyTowerSummit | 9F | 186 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 0 | Secondary terrain |
| SnowPath | 1F | 1125 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SnowPath | 2F | 1126 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| SnowPath | 3F | 1127 | Small | 7-9 rooms (random) | 20 | 15 | yes | 0%/0% | 1 | none |
| SnowPath | 4F | 1128 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 1F | 900 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 2F | 901 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 3F | 902 | Small | 8-10 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| SolarCave | 4F | 903 | Small | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 5F | 904 | Small | 7-9 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 6F | 905 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| SolarCave | 7F | 906 | Small | 8-10 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 8F | 907 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 9F | 908 | Medium | 15-17 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| SolarCave | 10F | 909 | Large | 11-13 rooms (random) | 10 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 11F | 910 | Medium | 12-14 rooms (random) | 10 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 12F | 911 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 13F | 912 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 14F | 913 | Medium | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 15F | 914 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 16F | 915 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 17F | 916 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 18F | 917 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SolarCave | 19F | 918 | Medium | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | none |
| SolarCave | 20F | 919 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| SouthernCavern | 1F | 760 | Small | 4-6 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 2F | 761 | Small | 4-6 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 3F | 762 | Small | 4-6 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 4F | 763 | Small | 5-7 rooms (random) | 50 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 5F | 764 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 6F | 765 | Large | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 7F | 766 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 8F | 767 | Small | 5-7 rooms (random) | 50 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 9F | 768 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 10F | 769 | Small | 5-7 rooms (random) | 20 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 11F | 770 | Large | 5-7 rooms (random) | 30 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 12F | 771 | Small | 5-7 rooms (random) | 40 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 13F | 772 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 14F | 773 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 15F | 774 | Small | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 16F | 775 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 17F | 776 | Small | 5-7 rooms (random) | 20 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 18F | 777 | Large | 5-7 rooms (random) | 30 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 19F | 778 | Small | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 20F | 779 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 21F | 780 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 22F | 781 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 23F | 782 | Large | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 24F | 783 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 25F | 784 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 26F | 785 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 27F | 786 | Small | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 28F | 787 | Crossroads | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 29F | 788 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 30F | 789 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 31F | 790 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 32F | 791 | Outer ring | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 33F | 792 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 34F | 793 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 35F | 794 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 36F | 795 | Cross | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 37F | 796 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 38F | 797 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 39F | 798 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 40F | 799 | Beetle | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 41F | 800 | Beetle | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 42F | 801 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 43F | 802 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 44F | 803 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 45F | 804 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 46F | 805 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 47F | 806 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 48F | 807 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| SouthernCavern | 49F | 808 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 0 | none |
| SouthernCavern | 50F | 809 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0 | none |
| StormySea | 1F | 187 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | none |
| StormySea | 2F | 188 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| StormySea | 3F | 189 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 4F | 190 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| StormySea | 5F | 191 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | Secondary terrain |
| StormySea | 6F | 192 | Beetle | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| StormySea | 7F | 193 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 8F | 194 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 9F | 195 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | none |
| StormySea | 10F | 196 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| StormySea | 11F | 197 | Beetle | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 12F | 198 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | none |
| StormySea | 13F | 199 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 10 | Secondary terrain |
| StormySea | 14F | 200 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0 | none |
| StormySea | 15F | 201 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 16F | 202 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 17F | 203 | Small | 6-8 rooms (random) | 30 | 15 | no | 0%/0% | 10 | none |
| StormySea | 18F | 204 | Beetle | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0 | none |
| StormySea | 19F | 205 | Small | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0 | Secondary terrain |
| StormySea | 20F | 206 | Small | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0 | none |
| StormySea | 21F | 207 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 10 | Secondary terrain |
| StormySea | 22F | 208 | Small | 9-11 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| StormySea | 23F | 209 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0 | Secondary terrain |
| StormySea | 24F | 210 | Large | 20-22 rooms (random) | 35 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 25F | 211 | Small | 10-12 rooms (random) | 35 | 15 | yes | 0%/0% | 10 | none |
| StormySea | 26F | 212 | Small | 8-10 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| StormySea | 27F | 213 | Small | 9-11 rooms (random) | 35 | 15 | no | 0%/0% | 0 | Secondary terrain |
| StormySea | 28F | 214 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| StormySea | 29F | 215 | Large | 15-17 rooms (random) | 35 | 15 | yes | 0%/0% | 10 | Secondary terrain |
| StormySea | 30F | 216 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| StormySea | 31F | 217 | Small | 9-11 rooms (random) | 35 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 32F | 218 | Small | 8-10 rooms (random) | 35 | 15 | no | 0%/0% | 0 | Secondary terrain |
| StormySea | 33F | 219 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 10 | none |
| StormySea | 34F | 220 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0 | none |
| StormySea | 35F | 221 | Small | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 0 | Secondary terrain |
| StormySea | 36F | 222 | Large | 15-17 rooms (random) | 40 | 15 | no | 0%/0% | 0 | none |
| StormySea | 37F | 223 | Small | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 10 | Secondary terrain |
| StormySea | 38F | 224 | Small | 10-12 rooms (random) | 40 | 15 | yes | 0%/0% | 0 | none |
| StormySea | 39F | 225 | Small | 10-12 rooms (random) | 40 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| StormySea | 40F | 226 | Small | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 0 | Secondary terrain |
| ThunderwaveCave | 1F | 3 | Medium | 7-9 rooms (random) | 15 | 5 | yes | 0%/0% | 0 | none |
| ThunderwaveCave | 2F | 4 | Medium | 7-9 rooms (random) | 15 | 5 | yes | 0%/0% | 0 | none |
| ThunderwaveCave | 3F | 5 | Medium | 8-10 rooms (random) | 15 | 5 | yes | 0%/0% | 0 | none |
| ThunderwaveCave | 4F | 6 | Medium | 8-10 rooms (random) | 15 | 5 | yes | 0%/0% | 0 | none |
| ThunderwaveCave | 5F | 7 | Medium | 8-10 rooms (random) | 15 | 5 | yes | 0%/0% | 0 | none |
| TinyWoods | 1F | 0 | Small | 5-7 rooms (random) | 15 | 5 | no | 0%/0% | 0 | none |
| TinyWoods | 2F | 1 | Small | 6-8 rooms (random) | 15 | 5 | no | 0%/0% | 0 | none |
| TinyWoods | 3F | 2 | Small | 6-8 rooms (random) | 15 | 5 | no | 0%/0% | 0 | none |
| UnownRelic | 1F | 1340 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 2F | 1341 | Large | 9-11 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 3F | 1342 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 4F | 1343 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 5F | 1344 | Large | 15-17 rooms (random) | 30 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 6F | 1345 | Large | 12-14 rooms (random) | 30 | 20 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 7F | 1346 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 8F | 1347 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 9F | 1348 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 10F | 1349 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 0 | Secondary terrain |
| UnownRelic | 11F | 1350 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 0 | Secondary terrain |
| UproarForest | 1F | 1026 | Medium | 6-8 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| UproarForest | 2F | 1027 | Medium | 7-9 rooms (random) | 20 | 20 | no | 0%/6% | 0 | Secondary terrain |
| UproarForest | 3F | 1028 | Medium | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| UproarForest | 4F | 1029 | Medium | 10-12 rooms (random) | 20 | 20 | no | 0%/6% | 0 | Secondary terrain |
| UproarForest | 5F | 1030 | Medium | 11-13 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| UproarForest | 6F | 1031 | Medium | 9-11 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| UproarForest | 7F | 1032 | Medium | 11-13 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| UproarForest | 8F | 1033 | Medium | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| UproarForest | 9F | 1034 | Medium | 7-9 rooms (random) | 20 | 20 | no | 0%/6% | 0 | none |
| UproarForest | 10F | 1035 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0 | none |
| WaterfallPond | 1F | 1321 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 2F | 1322 | Medium | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 3F | 1323 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 4F | 1324 | Medium | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 5F | 1325 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 6F | 1326 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 7F | 1327 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 8F | 1328 | Medium | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 9F | 1329 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 10F | 1330 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 11F | 1331 | Medium | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 12F | 1332 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 13F | 1333 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 14F | 1334 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 15F | 1335 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 16F | 1336 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 17F | 1337 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 18F | 1338 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WaterfallPond | 19F | 1339 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | none |
| WesternCave | 1F | 350 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 2F | 351 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 3F | 352 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| WesternCave | 4F | 353 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 5F | 354 | Large | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0 | Secondary terrain |
| WesternCave | 6F | 355 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 0 | none |
| WesternCave | 7F | 356 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 8F | 357 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 9F | 358 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| WesternCave | 10F | 359 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | Secondary terrain |
| WesternCave | 11F | 360 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 12F | 361 | Medium | 4-6 rooms (random) | 50 | 15 | yes | 0%/7% | 0 | none |
| WesternCave | 13F | 362 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 14F | 363 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 15F | 364 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | Secondary terrain |
| WesternCave | 16F | 365 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 17F | 366 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 18F | 367 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 0 | none |
| WesternCave | 19F | 368 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0 | none |
| WesternCave | 20F | 369 | Large | 18-20 rooms (random) | 40 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 21F | 370 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 22F | 371 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 23F | 372 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 24F | 373 | Large | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 25F | 374 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 26F | 375 | Medium | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 27F | 376 | Medium | 4-6 rooms (random) | 50 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 28F | 377 | Medium | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 29F | 378 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 30F | 379 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | Secondary terrain |
| WesternCave | 31F | 380 | Medium | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 32F | 381 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 33F | 382 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 34F | 383 | Large | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 35F | 384 | Large | 18-20 rooms (random) | 50 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 36F | 385 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 37F | 386 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 38F | 387 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 39F | 388 | Large | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 40F | 389 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 41F | 390 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 42F | 391 | Large | 5-7 rooms (random) | 50 | 15 | yes | 3%/7% | 0 | Secondary terrain |
| WesternCave | 43F | 392 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 44F | 393 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 45F | 394 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | Secondary terrain |
| WesternCave | 46F | 395 | Medium | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 47F | 396 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 48F | 397 | Large | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | Secondary terrain |
| WesternCave | 49F | 398 | Large | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 50F | 399 | Medium | 5-7 rooms (random) | 50 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 51F | 400 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 52F | 401 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 53F | 402 | Large | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 54F | 403 | Large | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 55F | 404 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 56F | 405 | Medium | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 57F | 406 | Medium | 4-6 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 58F | 407 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 59F | 408 | Large | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 60F | 409 | Large | 10-12 rooms (random) | 15 | 15 | yes | 3%/7% | 0 | Secondary terrain |
| WesternCave | 61F | 410 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 62F | 411 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 63F | 412 | Medium | 18-20 rooms (random) | 40 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 64F | 413 | Large | 20-22 rooms (random) | 30 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 65F | 414 | Large | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 66F | 415 | Medium | 16-18 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 67F | 416 | Medium | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 68F | 417 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 69F | 418 | Large | 6-8 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 70F | 419 | Large | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 71F | 420 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 72F | 421 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 73F | 422 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 74F | 423 | Large | 18-20 rooms (random) | 40 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 75F | 424 | Large | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 0 | Secondary terrain |
| WesternCave | 76F | 425 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 77F | 426 | Medium | 16-18 rooms (random) | 50 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 78F | 427 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 79F | 428 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 80F | 429 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 81F | 430 | Large | 8-10 rooms (random) | 10 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 82F | 431 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 83F | 432 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 84F | 433 | Medium | 16-18 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 85F | 434 | Large | 18-20 rooms (random) | 40 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 86F | 435 | Large | 20-22 rooms (random) | 30 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 87F | 436 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 88F | 437 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 89F | 438 | Medium | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 90F | 439 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | Secondary terrain |
| WesternCave | 91F | 440 | Large | 6-8 rooms (random) | 50 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 92F | 441 | Large | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 93F | 442 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 94F | 443 | Large | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 95F | 444 | Large | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0 | Secondary terrain |
| WesternCave | 96F | 445 | Large | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WesternCave | 97F | 446 | Large | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 98F | 447 | Large | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0 | none |
| WesternCave | 99F | 448 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 0 | none |
| WishCave | 1F | 480 | Small | 6-8 rooms (random) | 15 | 0 | yes | 8%/0% | 0 | none |
| WishCave | 2F | 481 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/0% | 0 | none |
| WishCave | 3F | 482 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 10 | Secondary terrain |
| WishCave | 4F | 483 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0 | none |
| WishCave | 5F | 484 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/0% | 0 | Secondary terrain |
| WishCave | 6F | 485 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/0% | 10 | none |
| WishCave | 7F | 486 | Small | 6-8 rooms (random) | 20 | 0 | yes | 50%/0% | 0 | Secondary terrain |
| WishCave | 8F | 487 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 0 | none |
| WishCave | 9F | 488 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 10 | none |
| WishCave | 10F | 489 | Unused 0xF | 10-12 rooms (random) | 40 | 0 | yes | 8%/0% | 0 | none |
| WishCave | 11F | 490 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 0 | Secondary terrain |
| WishCave | 12F | 491 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 10 | Secondary terrain |
| WishCave | 13F | 492 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0 | none |
| WishCave | 14F | 493 | Small | 8-10 rooms (random) | 20 | 0 | no | 8%/0% | 0 | none |
| WishCave | 15F | 494 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | no | 8%/0% | 10 | Secondary terrain |
| WishCave | 16F | 495 | Small | 6-8 rooms (random) | 10 | 0 | yes | 8%/0% | 0 | none |
| WishCave | 17F | 496 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/0% | 0 | none |
| WishCave | 18F | 497 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 10 | none |
| WishCave | 19F | 498 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0 | Secondary terrain |
| WishCave | 20F | 499 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/0% | 0 | none |
| WishCave | 21F | 500 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/0% | 10 | none |
| WishCave | 22F | 501 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0 | Secondary terrain |
| WishCave | 23F | 502 | Small | 6-8 rooms (random) | 20 | 0 | no | 50%/0% | 0 | none |
| WishCave | 24F | 503 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 10 | Secondary terrain |
| WishCave | 25F | 504 | Unused 0xF | 10-12 rooms (random) | 40 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 26F | 505 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 27F | 506 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 28F | 507 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 29F | 508 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/100% | 0 | none |
| WishCave | 30F | 509 | Unused 0xF | 12-14 rooms (random) | 30 | 0 | no | 8%/6% | 10 | Secondary terrain |
| WishCave | 31F | 510 | Small | 6-8 rooms (random) | 15 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 32F | 511 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 0 | none |
| WishCave | 33F | 512 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 34F | 513 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 35F | 514 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/6% | 0 | none |
| WishCave | 36F | 515 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/6% | 10 | none |
| WishCave | 37F | 516 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 38F | 517 | Small | 6-8 rooms (random) | 20 | 0 | no | 50%/6% | 0 | Secondary terrain |
| WishCave | 39F | 518 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 40F | 519 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 41F | 520 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 42F | 521 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 43F | 522 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 44F | 523 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 45F | 524 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | no | 8%/6% | 10 | Secondary terrain |
| WishCave | 46F | 525 | Small | 8-10 rooms (random) | 15 | 0 | yes | 8%/100% | 0 | none |
| WishCave | 47F | 526 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 0 | none |
| WishCave | 48F | 527 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 49F | 528 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 50F | 529 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 51F | 530 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/6% | 10 | none |
| WishCave | 52F | 531 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 53F | 532 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 54F | 533 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 55F | 534 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | yes | 50%/6% | 0 | none |
| WishCave | 56F | 535 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 57F | 536 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | Secondary terrain |
| WishCave | 58F | 537 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 59F | 538 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 0 | none |
| WishCave | 60F | 539 | Unused 0xF | 10-12 rooms (random) | 15 | 0 | no | 8%/6% | 10 | none |
| WishCave | 61F | 540 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 62F | 541 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 63F | 542 | Small | 6-8 rooms (random) | 40 | 0 | no | 8%/6% | 10 | none |
| WishCave | 64F | 543 | Small | 6-8 rooms (random) | 30 | 0 | yes | 8%/100% | 0 | Secondary terrain |
| WishCave | 65F | 544 | Unused 0xF | 12-14 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 66F | 545 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | Secondary terrain |
| WishCave | 67F | 546 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 68F | 547 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 69F | 548 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 70F | 549 | Unused 0xF | 10-12 rooms (random) | 30 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 71F | 550 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 72F | 551 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 73F | 552 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 74F | 553 | Small | 6-8 rooms (random) | 40 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 75F | 554 | Unused 0xF | 20-22 rooms (random) | 30 | 0 | no | 50%/6% | 10 | none |
| WishCave | 76F | 555 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 77F | 556 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 78F | 557 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | Secondary terrain |
| WishCave | 79F | 558 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 80F | 559 | Unused 0xF | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 81F | 560 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 10 | none |
| WishCave | 82F | 561 | Small | 6-8 rooms (random) | 15 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 83F | 562 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 84F | 563 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | Secondary terrain |
| WishCave | 85F | 564 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 86F | 565 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/100% | 0 | Secondary terrain |
| WishCave | 87F | 566 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 88F | 567 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 89F | 568 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 90F | 569 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | no | 8%/6% | 10 | Secondary terrain |
| WishCave | 91F | 570 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 92F | 571 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | none |
| WishCave | 93F | 572 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 94F | 573 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | Secondary terrain |
| WishCave | 95F | 574 | Unused 0xF | 10-12 rooms (random) | 30 | 0 | no | 8%/6% | 0 | none |
| WishCave | 96F | 575 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 10 | none |
| WishCave | 97F | 576 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0 | none |
| WishCave | 98F | 577 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0 | Secondary terrain |
| WishCave | 99F | 578 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 10 | none |
| WondrousSea | 1F | 965 | Large | 10-12 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 2F | 966 | Large | 5-7 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 3F | 967 | Large | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 4F | 968 | Large | 5-7 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 5F | 969 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 6F | 970 | Large | 12-14 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 7F | 971 | Large | 5-7 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 8F | 972 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 9F | 973 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 10F | 974 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 11F | 975 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WondrousSea | 12F | 976 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WyvernHill | 1F | 810 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WyvernHill | 2F | 811 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0 | Secondary terrain |
| WyvernHill | 3F | 812 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 0 | Secondary terrain |
| WyvernHill | 4F | 813 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 5F | 814 | Small | 5-7 rooms (random) | 40 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 6F | 815 | Small | 8-10 rooms (random) | 30 | 15 | yes | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 7F | 816 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 8F | 817 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 9F | 818 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 10F | 819 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 11F | 820 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 12F | 821 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 13F | 822 | Small | 5-7 rooms (random) | 50 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 14F | 823 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 15F | 824 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 16F | 825 | Small | 8-10 rooms (random) | 10 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 17F | 826 | Small | 8-10 rooms (random) | 15 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 18F | 827 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 19F | 828 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | Secondary terrain |
| WyvernHill | 20F | 829 | Medium | 5-7 rooms (random) | 40 | 15 | no | 8%/5% | 0 | none |
| WyvernHill | 21F | 830 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 8%/5% | 0 | none |
| WyvernHill | 22F | 831 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| WyvernHill | 23F | 832 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| WyvernHill | 24F | 833 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | none |
| WyvernHill | 25F | 834 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| WyvernHill | 26F | 835 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/5% | 0 | none |
| WyvernHill | 27F | 836 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | none |
| WyvernHill | 28F | 837 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| WyvernHill | 29F | 838 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0 | none |
| WyvernHill | 30F | 839 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 0 | none |

## Seed catalog (definition -> where used)
| Seed ID | Layout | Rooms | Conn. | Extra halls | Dead ends | Shops/MH% | Maze% | Secondary budget | Room flags | Fixed room | Used by (dungeon: floors) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | Small | 5-7 rooms (random) | 15 | 5 | no | 0%/0% | 0% | 0 | none | 0 | TinyWoods: 1F |
| 1 | Small | 6-8 rooms (random) | 15 | 5 | no | 0%/0% | 0% | 0 | none | 0 | TinyWoods: 2F |
| 2 | Small | 6-8 rooms (random) | 15 | 5 | no | 0%/0% | 0% | 0 | none | 0 | TinyWoods: 3F |
| 3 | Medium | 7-9 rooms (random) | 15 | 5 | yes | 0%/0% | 0% | 0 | none | 0 | ThunderwaveCave: 1F |
| 4 | Medium | 7-9 rooms (random) | 15 | 5 | yes | 0%/0% | 0% | 0 | none | 0 | ThunderwaveCave: 2F |
| 5 | Medium | 8-10 rooms (random) | 15 | 5 | yes | 0%/0% | 0% | 0 | none | 0 | ThunderwaveCave: 3F |
| 6 | Medium | 8-10 rooms (random) | 15 | 5 | yes | 0%/0% | 0% | 0 | none | 0 | ThunderwaveCave: 4F |
| 7 | Medium | 8-10 rooms (random) | 15 | 5 | yes | 0%/0% | 0% | 0 | none | 0 | ThunderwaveCave: 5F |
| 8 | Medium | 9-11 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 1F |
| 9 | Medium | 9-11 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 2F |
| 10 | Medium | 8-10 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 3F |
| 11 | Large (0x8 grid cap) | 8-10 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 4F |
| 12 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 5F |
| 13 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 6F |
| 14 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 7F |
| 15 | Large (0x8 grid cap) | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 0 | MtSteel: 8F |
| 16 | Large | 7-9 rooms (random) | 12 | 0 | no | 0%/0% | 0% | 0 | none | 1 | MtSteel: 9F |
| 17 | Medium | 7-9 rooms (random) | 20 | 6 | no | 0%/0% | 0% | 0 | none | 0 | SinisterWoods: 1F |
| 18 | Medium | 6-8 rooms (random) | 20 | 8 | no | 0%/0% | 0% | 0 | none | 0 | SinisterWoods: 2F |
| 19 | Medium | 7-9 rooms (random) | 20 | 8 | no | 0%/0% | 0% | 0 | none | 0 | SinisterWoods: 3F |
| 20 | Medium | 7-9 rooms (random) | 20 | 10 | no | 0%/0% | 0% | 0 | none | 0 | SinisterWoods: 4F |
| 21 | Large | 8-10 rooms (random) | 25 | 10 | yes | 0%/0% | 1% | 0 | none | 0 | SinisterWoods: 5F |
| 22 | Large | 9-11 rooms (random) | 20 | 10 | yes | 0%/0% | 1% | 0 | none | 0 | SinisterWoods: 6F |
| 23 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | SinisterWoods: 7F |
| 24 | Small | 11-13 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SinisterWoods: 8F |
| 25 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SinisterWoods: 9F |
| 26 | Large | 12-14 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SinisterWoods: 10F |
| 27 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | SinisterWoods: 11F |
| 28 | Large | 12-14 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | SinisterWoods: 12F |
| 29 | Large | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 2 | SinisterWoods: 13F |
| 30 | Unused 0xD | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | SilentChasm: 1F |
| 31 | Unused 0xD | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | SilentChasm: 2F |
| 32 | Unused 0xD | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | SilentChasm: 3F |
| 33 | Unused 0xD | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | SilentChasm: 4F |
| 34 | Unused 0xD | 8-10 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | SilentChasm: 5F |
| 35 | Large | 7-9 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | SilentChasm: 6F |
| 36 | Large | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | SilentChasm: 7F |
| 37 | Large | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | SilentChasm: 8F |
| 38 | Large | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | SilentChasm: 9F |
| 39 | Medium | 11-13 rooms (random) | 25 | 10 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 1F |
| 40 | Medium | 10-12 rooms (random) | 20 | 5 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 2F |
| 41 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 3F |
| 42 | Large | 8-10 rooms (random) | 30 | 10 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 4F |
| 43 | Large | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 5F |
| 44 | Large | 10-12 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 6F |
| 45 | Outer ring | 5-7 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 7F |
| 46 | Crossroads | 9-11 rooms (random) | 20 | 35 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 8F |
| 47 | Outer ring | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 9F |
| 48 | Crossroads | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MtThunder: 10F |
| 49 | Large | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtThunderPeak: 1F |
| 50 | Large | 5-7 rooms (random) | 22 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtThunderPeak: 2F |
| 51 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 3 | MtThunderPeak: 3F |
| 52 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 1F |
| 53 | Medium | 5-7 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 2F |
| 54 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 3F |
| 55 | Medium | 5-7 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 4F |
| 56 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 5F |
| 57 | Large | 12-14 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 6F |
| 58 | Large | 5-7 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 7F |
| 59 | Unused 0xD | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 8F |
| 60 | Unused 0xD | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 9F |
| 61 | Large | 8-10 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 10F |
| 62 | Large | 8-10 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 11F |
| 63 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | GreatCanyon: 12F |
| 64 | Small | 6-8 rooms (random) | 25 | 10 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 1F |
| 65 | Small | 6-8 rooms (random) | 25 | 10 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 2F |
| 66 | Small | 6-8 rooms (random) | 25 | 10 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 3F |
| 67 | Small | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 4F |
| 68 | Small | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | LapisCave: 5F |
| 69 | Small | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | LapisCave: 6F |
| 70 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | LapisCave: 7F |
| 71 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | LapisCave: 8F |
| 72 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 9F |
| 73 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 10F |
| 74 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 11F |
| 75 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 12F |
| 76 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 1% | 0 | none | 0 | LapisCave: 13F |
| 77 | Small | 6-8 rooms (random) | 30 | 20 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | LapisCave: 14F |
| 78 | Medium | 6-8 rooms (random) | 15 | 20 | no | 0%/0% | 0% | 0 | none | 0 | MtBlaze: 1F |
| 79 | Medium | 6-8 rooms (random) | 16 | 20 | no | 0%/0% | 0% | 0 | none | 0 | MtBlaze: 2F |
| 80 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0% | 0 | none | 0 | MtBlaze: 3F |
| 81 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 4F |
| 82 | Medium | 6-8 rooms (random) | 20 | 20 | no | 8%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 5F |
| 83 | Medium | 6-8 rooms (random) | 20 | 20 | no | 8%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 6F |
| 84 | Medium | 6-8 rooms (random) | 20 | 20 | no | 10%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 7F |
| 85 | Medium | 6-8 rooms (random) | 20 | 20 | no | 8%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 8F |
| 86 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 9F |
| 87 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 10F |
| 88 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 11F |
| 89 | Medium | 6-8 rooms (random) | 20 | 20 | no | 6%/0% | 0% | 0 | Secondary terrain | 0 | MtBlaze: 12F |
| 90 | Large | 6-8 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | none | 0 | MtBlazePeak: 1F |
| 91 | Large | 7-9 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | none | 0 | MtBlazePeak: 2F |
| 92 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | none | 4 | MtBlazePeak: 3F |
| 93 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 8%/0% | 0% | 0 | none | 0 | FrostyForest: 1F |
| 94 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 8%/0% | 0% | 0 | none | 0 | FrostyForest: 2F |
| 95 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 8%/0% | 0% | 0 | none | 0 | FrostyForest: 3F |
| 96 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 7%/0% | 0% | 0 | Secondary terrain | 0 | FrostyForest: 4F |
| 97 | Large | 6-8 rooms (random) | 20 | 15 | yes | 7%/0% | 0% | 0 | Secondary terrain | 0 | FrostyForest: 5F |
| 98 | Large | 6-8 rooms (random) | 20 | 15 | yes | 7%/0% | 0% | 0 | Secondary terrain | 0 | FrostyForest: 6F |
| 99 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 6%/0% | 0% | 0 | none | 0 | FrostyForest: 7F |
| 100 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 6%/0% | 1% | 0 | none | 0 | FrostyForest: 8F |
| 101 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 6%/0% | 1% | 0 | none | 0 | FrostyForest: 9F |
| 102 | Medium | 6-8 rooms (random) | 20 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FrostyGrotto: 1F |
| 103 | Medium | 6-8 rooms (random) | 25 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FrostyGrotto: 2F |
| 104 | Large | 8-10 rooms (random) | 30 | 40 | yes | 0%/0% | 1% | 0 | none | 0 | FrostyGrotto: 3F |
| 105 | Large | 8-10 rooms (random) | 20 | 40 | yes | 0%/0% | 1% | 0 | none | 0 | FrostyGrotto: 4F |
| 106 | Large | 8-10 rooms (random) | 10 | 40 | yes | 0%/0% | 1% | 0 | none | 5 | FrostyGrotto: 5F |
| 107 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 1F |
| 108 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 2F |
| 109 | Medium | 6-8 rooms (random) | 25 | 10 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 3F |
| 110 | Medium | 6-8 rooms (random) | 25 | 10 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 4F |
| 111 | Medium | 6-8 rooms (random) | 15 | 10 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 5F |
| 112 | Medium | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 6F |
| 113 | Medium | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 7F |
| 114 | Medium | 6-8 rooms (random) | 25 | 20 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 8F |
| 115 | Large | 6-8 rooms (random) | 15 | 20 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 9F |
| 116 | Large | 6-8 rooms (random) | 25 | 20 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 10F |
| 117 | Medium | 6-8 rooms (random) | 25 | 30 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 11F |
| 118 | Medium | 6-8 rooms (random) | 15 | 30 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 12F |
| 119 | Medium | 6-8 rooms (random) | 15 | 30 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 13F |
| 120 | Medium | 6-8 rooms (random) | 25 | 30 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 14F |
| 121 | Medium | 6-8 rooms (random) | 25 | 30 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | MtFreeze: 15F |
| 122 | Medium | 6-8 rooms (random) | 15 | 5 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeak: 1F |
| 123 | Medium | 6-8 rooms (random) | 20 | 5 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeak: 2F |
| 124 | Large | 12-14 rooms (random) | 25 | 5 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeak: 3F |
| 125 | Large | 10-12 rooms (random) | 15 | 10 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeak: 4F |
| 126 | Large | 11-13 rooms (random) | 20 | 10 | no | 0%/0% | 0% | 0 | Secondary terrain | 6 | MtFreezePeak: 5F |
| 127 | Large | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MagmaCavern: 1F |
| 128 | Large | 7-9 rooms (random) | 20 | 15 | no | 5%/0% | 0% | 0 | none | 0 | MagmaCavern: 2F |
| 129 | Medium | 6-8 rooms (random) | 20 | 15 | no | 10%/0% | 0% | 0 | none | 0 | MagmaCavern: 3F |
| 130 | Medium | 6-8 rooms (random) | 25 | 15 | no | 10%/0% | 0% | 0 | none | 0 | MagmaCavern: 4F |
| 131 | Large | 8-10 rooms (random) | 20 | 15 | no | 5%/0% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 5F |
| 132 | Large | 8-10 rooms (random) | 20 | 15 | no | 5%/0% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 6F |
| 133 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 7F |
| 134 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/8% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 8F |
| 135 | Medium | 6-8 rooms (random) | 15 | 15 | no | 0%/8% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 9F |
| 136 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/8% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 10F |
| 137 | Large | 7-9 rooms (random) | 25 | 15 | no | 0%/8% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 11F |
| 138 | Large | 7-9 rooms (random) | 25 | 15 | no | 0%/8% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 12F |
| 139 | Large | 7-9 rooms (random) | 20 | 15 | no | 0%/8% | 0% | 0 | none | 0 | MagmaCavern: 13F |
| 140 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/8% | 0% | 0 | none | 0 | MagmaCavern: 14F |
| 141 | Medium | 6-8 rooms (random) | 15 | 15 | no | 0%/8% | 0% | 0 | none | 0 | MagmaCavern: 15F |
| 142 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/8% | 0% | 0 | none | 0 | MagmaCavern: 16F |
| 143 | Large | 8-10 rooms (random) | 25 | 15 | no | 0%/8% | 0% | 0 | none | 0 | MagmaCavern: 17F |
| 144 | Medium | 6-8 rooms (random) | 25 | 15 | no | 0%/8% | 0% | 0 | none | 0 | MagmaCavern: 18F |
| 145 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/8% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 19F |
| 146 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/8% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 20F |
| 147 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 21F |
| 148 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 22F |
| 149 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MagmaCavern: 23F |
| 150 | Unused 0xF | 8-10 rooms (random) | 20 | 10 | no | 0%/0% | 0% | 0 | none | 0 | MagmaCavernPit: 1F |
| 151 | Unused 0xF | 8-10 rooms (random) | 20 | 10 | yes | 0%/0% | 1% | 0 | none | 20 | MagmaCavernPit: 2F |
| 152 | Unused 0xF | 7-9 rooms (random) | 20 | 50 | no | 0%/0% | 0% | 0 | none | 7 | MagmaCavernPit: 3F |
| 153 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | SkyTower: 1F |
| 154 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTower: 2F |
| 155 | Large | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 3F |
| 156 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 4F |
| 157 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 5F |
| 158 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 6F |
| 159 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 7F |
| 160 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 8F |
| 161 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 9F |
| 162 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 10F |
| 163 | Medium | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 11F |
| 164 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 12F |
| 165 | Medium | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 13F |
| 166 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 14F |
| 167 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 15F |
| 168 | Large | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 16F |
| 169 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 17F |
| 170 | Cross | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 18F |
| 171 | Cross | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 19F |
| 172 | Cross | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 20F |
| 173 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 21F |
| 174 | Large | 5-7 rooms (random) | 50 | 0 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 22F |
| 175 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 23F |
| 176 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | SkyTower: 24F |
| 177 | Large | 5-7 rooms (random) | 50 | 0 | no | 0%/5% | 1% | 0 | Secondary terrain | 0 | SkyTower: 25F |
| 178 | Line | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 1F |
| 179 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 2F |
| 180 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 3F |
| 181 | Line | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 4F |
| 182 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 5F |
| 183 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 6F |
| 184 | Line | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 7F |
| 185 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | SkyTowerSummit: 8F |
| 186 | Crossroads | 8-10 rooms (random) | 30 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 8 | SkyTowerSummit: 9F |
| 187 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | none | 0 | StormySea: 1F |
| 188 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 2F |
| 189 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 3F |
| 190 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | StormySea: 4F |
| 191 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | Secondary terrain | 0 | StormySea: 5F |
| 192 | Beetle | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 6F |
| 193 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 7F |
| 194 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 8F |
| 195 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | none | 0 | StormySea: 9F |
| 196 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 10F |
| 197 | Beetle | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 11F |
| 198 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | StormySea: 12F |
| 199 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | Secondary terrain | 0 | StormySea: 13F |
| 200 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 14F |
| 201 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 15F |
| 202 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 16F |
| 203 | Small | 6-8 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 10 | none | 0 | StormySea: 17F |
| 204 | Beetle | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 18F |
| 205 | Small | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | StormySea: 19F |
| 206 | Small | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 20F |
| 207 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 10 | Secondary terrain | 0 | StormySea: 21F |
| 208 | Small | 9-11 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 22F |
| 209 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | StormySea: 23F |
| 210 | Large | 20-22 rooms (random) | 35 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 24F |
| 211 | Small | 10-12 rooms (random) | 35 | 15 | yes | 0%/0% | 1% | 10 | none | 0 | StormySea: 25F |
| 212 | Small | 8-10 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 26F |
| 213 | Small | 9-11 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | StormySea: 27F |
| 214 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 28F |
| 215 | Large | 15-17 rooms (random) | 35 | 15 | yes | 0%/0% | 1% | 10 | Secondary terrain | 0 | StormySea: 29F |
| 216 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 30F |
| 217 | Small | 9-11 rooms (random) | 35 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 31F |
| 218 | Small | 8-10 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | StormySea: 32F |
| 219 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 10 | none | 0 | StormySea: 33F |
| 220 | Small | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 34F |
| 221 | Small | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | StormySea: 35F |
| 222 | Large | 15-17 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | StormySea: 36F |
| 223 | Small | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 10 | Secondary terrain | 0 | StormySea: 37F |
| 224 | Small | 10-12 rooms (random) | 40 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | StormySea: 38F |
| 225 | Small | 10-12 rooms (random) | 40 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | StormySea: 39F |
| 226 | Small | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 22 | StormySea: 40F |
| 227 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 1F |
| 228 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 2F |
| 229 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 3F |
| 230 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 4F |
| 231 | Small | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 10 | Secondary terrain | 0 | SilverTrench: 5F |
| 232 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 6F |
| 233 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 7F |
| 234 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 8F |
| 235 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/10% | 1% | 10 | none | 0 | SilverTrench: 9F |
| 236 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 10F |
| 237 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 11F |
| 238 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 12F |
| 239 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 13F |
| 240 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 14F |
| 241 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 15F |
| 242 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 16F |
| 243 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 17F |
| 244 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 18F |
| 245 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 19F |
| 246 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 20F |
| 247 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 10 | none | 0 | SilverTrench: 21F |
| 248 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 22F |
| 249 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 23F |
| 250 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 24F |
| 251 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 25F |
| 252 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 26F |
| 253 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/10% | 1% | 0 | none | 0 | SilverTrench: 27F |
| 254 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 28F |
| 255 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 29F |
| 256 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 30F |
| 257 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 31F |
| 258 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 32F |
| 259 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 10 | none | 0 | SilverTrench: 33F |
| 260 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 34F |
| 261 | One-room Monster House | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 35F |
| 262 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 36F |
| 263 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 37F |
| 264 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 38F |
| 265 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 39F |
| 266 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 40F |
| 267 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | Secondary terrain | 0 | SilverTrench: 41F |
| 268 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/10% | 1% | 0 | none | 0 | SilverTrench: 42F |
| 269 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 43F |
| 270 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 44F |
| 271 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 10 | Secondary terrain | 0 | SilverTrench: 45F |
| 272 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 46F |
| 273 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 47F |
| 274 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 48F |
| 275 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | Secondary terrain | 0 | SilverTrench: 49F |
| 276 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 50F |
| 277 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 51F |
| 278 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 52F |
| 279 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | Secondary terrain | 0 | SilverTrench: 53F |
| 280 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/10% | 1% | 0 | none | 0 | SilverTrench: 54F |
| 281 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 55F |
| 282 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/10% | 0% | 0 | none | 0 | SilverTrench: 56F |
| 283 | Small | 4-6 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 10 | Secondary terrain | 0 | SilverTrench: 57F |
| 284 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 58F |
| 285 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 59F |
| 286 | Small | 10-12 rooms (random) | 15 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 60F |
| 287 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 61F |
| 288 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 62F |
| 289 | Beetle | 18-20 rooms (random) | 40 | 15 | yes | 0%/20% | 1% | 0 | none | 0 | SilverTrench: 63F |
| 290 | Beetle | 20-22 rooms (random) | 30 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 64F |
| 291 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 65F |
| 292 | Small | 16-18 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 66F |
| 293 | Small | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 67F |
| 294 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 68F |
| 295 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 69F |
| 296 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 70F |
| 297 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 10 | Secondary terrain | 0 | SilverTrench: 71F |
| 298 | Small | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 72F |
| 299 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/10% | 0% | 0 | none | 0 | SilverTrench: 73F |
| 300 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 74F |
| 301 | One-room Monster House | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 75F |
| 302 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 76F |
| 303 | Small | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 77F |
| 304 | Small | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 78F |
| 305 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 79F |
| 306 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 80F |
| 307 | Small | 8-10 rooms (random) | 10 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 81F |
| 308 | Small | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 82F |
| 309 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 83F |
| 310 | Medium | 16-18 rooms (random) | 20 | 15 | yes | 0%/10% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 84F |
| 311 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 85F |
| 312 | Beetle | 20-22 rooms (random) | 30 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 86F |
| 313 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 10 | none | 0 | SilverTrench: 87F |
| 314 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 88F |
| 315 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 89F |
| 316 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | SilverTrench: 90F |
| 317 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | Secondary terrain | 0 | SilverTrench: 91F |
| 318 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 92F |
| 319 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 93F |
| 320 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 94F |
| 321 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | SilverTrench: 95F |
| 322 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | SilverTrench: 96F |
| 323 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | SilverTrench: 97F |
| 324 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | SilverTrench: 98F |
| 325 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 10 | none | 21 | SilverTrench: 99F |
| 326 | Large | 8-10 rooms (random) | 40 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 1F |
| 327 | Large | 10-12 rooms (random) | 40 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 2F |
| 328 | Large | 12-14 rooms (random) | 40 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 3F |
| 329 | Large | 15-17 rooms (random) | 40 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 4F |
| 330 | Large | 12-14 rooms (random) | 40 | 20 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 5F |
| 331 | Large | 15-17 rooms (random) | 40 | 20 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 6F |
| 332 | Large | 20-22 rooms (random) | 40 | 20 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 7F |
| 333 | Large | 15-17 rooms (random) | 40 | 30 | yes | 0%/0% | 1% | 0 | none | 0 | MeteorCave: 8F |
| 334 | Large | 12-14 rooms (random) | 40 | 30 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 9F |
| 335 | Large | 10-12 rooms (random) | 40 | 30 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 10F |
| 336 | Large | 8-10 rooms (random) | 40 | 30 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 11F |
| 337 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 12F |
| 338 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 13F |
| 339 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 14F |
| 340 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 15F |
| 341 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 16F |
| 342 | Large | 12-14 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 17F |
| 343 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 18F |
| 344 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MeteorCave: 19F |
| 345 | Large | 15-17 rooms (random) | 40 | 40 | yes | 0%/0% | 1% | 0 | Secondary terrain | 23 | MeteorCave: 20F |
| 346 | Medium | 6-8 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeakAlt: 1F |
| 347 | Medium | 6-8 rooms (random) | 20 | 0 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeakAlt: 2F |
| 348 | Large | 12-14 rooms (random) | 25 | 0 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeakAlt: 3F |
| 349 | Large | 10-12 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MtFreezePeakAlt: 4F |
| 350 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 1F |
| 351 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 2F |
| 352 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | WesternCave: 3F |
| 353 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 4F |
| 354 | Large | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 5F |
| 355 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | WesternCave: 6F |
| 356 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 7F |
| 357 | Medium | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 8F |
| 358 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | WesternCave: 9F |
| 359 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 10F |
| 360 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 11F |
| 361 | Medium | 4-6 rooms (random) | 50 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | WesternCave: 12F |
| 362 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 13F |
| 363 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 14F |
| 364 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 15F |
| 365 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 16F |
| 366 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 17F |
| 367 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | WesternCave: 18F |
| 368 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | WesternCave: 19F |
| 369 | Large | 18-20 rooms (random) | 40 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 20F |
| 370 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 21F |
| 371 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 22F |
| 372 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 23F |
| 373 | Large | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 24F |
| 374 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 25F |
| 375 | Medium | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 26F |
| 376 | Medium | 4-6 rooms (random) | 50 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 27F |
| 377 | Medium | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 28F |
| 378 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 29F |
| 379 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 30F |
| 380 | Medium | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 31F |
| 381 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 32F |
| 382 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 33F |
| 383 | Large | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 34F |
| 384 | Large | 18-20 rooms (random) | 50 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 35F |
| 385 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 36F |
| 386 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 37F |
| 387 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 38F |
| 388 | Large | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 39F |
| 389 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 40F |
| 390 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 41F |
| 391 | Large | 5-7 rooms (random) | 50 | 15 | yes | 3%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 42F |
| 392 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 43F |
| 393 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 44F |
| 394 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 45F |
| 395 | Medium | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 46F |
| 396 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 47F |
| 397 | Large | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 48F |
| 398 | Large | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 49F |
| 399 | Medium | 5-7 rooms (random) | 50 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 50F |
| 400 | Medium | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 51F |
| 401 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 52F |
| 402 | Large | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 53F |
| 403 | Large | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 54F |
| 404 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 55F |
| 405 | Medium | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 56F |
| 406 | Medium | 4-6 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 57F |
| 407 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 58F |
| 408 | Large | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0% | 0 | none | 59 | WesternCave: 59F |
| 409 | Large | 10-12 rooms (random) | 15 | 15 | yes | 3%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 60F |
| 410 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 61F |
| 411 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 62F |
| 412 | Medium | 18-20 rooms (random) | 40 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 63F |
| 413 | Large | 20-22 rooms (random) | 30 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 64F |
| 414 | Large | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 65F |
| 415 | Medium | 16-18 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 66F |
| 416 | Medium | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 67F |
| 417 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 68F |
| 418 | Large | 6-8 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 69F |
| 419 | Large | 8-10 rooms (random) | 10 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 70F |
| 420 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 71F |
| 421 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 72F |
| 422 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 73F |
| 423 | Large | 18-20 rooms (random) | 40 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 74F |
| 424 | Large | 20-22 rooms (random) | 30 | 15 | yes | 3%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 75F |
| 425 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 76F |
| 426 | Medium | 16-18 rooms (random) | 50 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 77F |
| 427 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 78F |
| 428 | Large | 8-10 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 79F |
| 429 | Large | 6-8 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 80F |
| 430 | Large | 8-10 rooms (random) | 10 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 81F |
| 431 | Medium | 10-12 rooms (random) | 15 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 82F |
| 432 | Medium | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 83F |
| 433 | Medium | 16-18 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 84F |
| 434 | Large | 18-20 rooms (random) | 40 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 85F |
| 435 | Large | 20-22 rooms (random) | 30 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 86F |
| 436 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 87F |
| 437 | Medium | 16-18 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 88F |
| 438 | Medium | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 89F |
| 439 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | Secondary terrain | 0 | WesternCave: 90F |
| 440 | Large | 6-8 rooms (random) | 50 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 91F |
| 441 | Large | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 92F |
| 442 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 93F |
| 443 | Large | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 94F |
| 444 | Large | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | Secondary terrain | 0 | WesternCave: 95F |
| 445 | Large | 12-14 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 0 | WesternCave: 96F |
| 446 | Large | 10-12 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 97F |
| 447 | Large | 12-14 rooms (random) | 20 | 15 | no | 3%/7% | 0% | 0 | none | 0 | WesternCave: 98F |
| 448 | Large | 8-10 rooms (random) | 20 | 15 | yes | 3%/7% | 1% | 0 | none | 9 | WesternCave: 99F |
| 449 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 1F |
| 450 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 2F |
| 451 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 3F |
| 452 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 4F |
| 453 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 5F |
| 454 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 6F |
| 455 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 7F |
| 456 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 8F |
| 457 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 9F |
| 458 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 10F |
| 459 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 11F |
| 460 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 12F |
| 461 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 13F |
| 462 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 14F |
| 463 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 15F |
| 464 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 16F |
| 465 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 17F |
| 466 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 18F |
| 467 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 19F |
| 468 | Large | 12-14 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 1 | none | 0 | Boss3: 20F |
| 469 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss4: 1F |
| 470 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss4: 2F |
| 471 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss4: 3F |
| 472 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 4F |
| 473 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 5F |
| 474 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 6F |
| 475 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 7F |
| 476 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 8F |
| 477 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 9F |
| 478 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 10F |
| 479 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss4: 11F |
| 480 | Small | 6-8 rooms (random) | 15 | 0 | yes | 8%/0% | 0% | 0 | none | 0 | WishCave: 1F |
| 481 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/0% | 1% | 0 | none | 0 | WishCave: 2F |
| 482 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 0% | 10 | Secondary terrain | 0 | WishCave: 3F |
| 483 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0% | 0 | none | 0 | WishCave: 4F |
| 484 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/0% | 1% | 0 | Secondary terrain | 0 | WishCave: 5F |
| 485 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/0% | 0% | 10 | none | 0 | WishCave: 6F |
| 486 | Small | 6-8 rooms (random) | 20 | 0 | yes | 50%/0% | 0% | 0 | Secondary terrain | 0 | WishCave: 7F |
| 487 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 1% | 0 | none | 0 | WishCave: 8F |
| 488 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 0% | 10 | none | 0 | WishCave: 9F |
| 489 | Unused 0xF | 10-12 rooms (random) | 40 | 0 | yes | 8%/0% | 0% | 0 | none | 0 | WishCave: 10F |
| 490 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 1% | 0 | Secondary terrain | 0 | WishCave: 11F |
| 491 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 0% | 10 | Secondary terrain | 0 | WishCave: 12F |
| 492 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0% | 0 | none | 0 | WishCave: 13F |
| 493 | Small | 8-10 rooms (random) | 20 | 0 | no | 8%/0% | 1% | 0 | none | 0 | WishCave: 14F |
| 494 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | no | 8%/0% | 0% | 10 | Secondary terrain | 0 | WishCave: 15F |
| 495 | Small | 6-8 rooms (random) | 10 | 0 | yes | 8%/0% | 0% | 0 | none | 0 | WishCave: 16F |
| 496 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/0% | 1% | 0 | none | 0 | WishCave: 17F |
| 497 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 0% | 10 | none | 0 | WishCave: 18F |
| 498 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0% | 0 | Secondary terrain | 0 | WishCave: 19F |
| 499 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/0% | 1% | 0 | none | 26 | WishCave: 20F |
| 500 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/0% | 0% | 10 | none | 0 | WishCave: 21F |
| 501 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/0% | 0% | 0 | Secondary terrain | 0 | WishCave: 22F |
| 502 | Small | 6-8 rooms (random) | 20 | 0 | no | 50%/0% | 1% | 0 | none | 0 | WishCave: 23F |
| 503 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/0% | 0% | 10 | Secondary terrain | 0 | WishCave: 24F |
| 504 | Unused 0xF | 10-12 rooms (random) | 40 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 25F |
| 505 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 26F |
| 506 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 27F |
| 507 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 28F |
| 508 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/100% | 1% | 0 | none | 0 | WishCave: 29F |
| 509 | Unused 0xF | 12-14 rooms (random) | 30 | 0 | no | 8%/6% | 0% | 10 | Secondary terrain | 0 | WishCave: 30F |
| 510 | Small | 6-8 rooms (random) | 15 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 31F |
| 511 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 32F |
| 512 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 33F |
| 513 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 34F |
| 514 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 35F |
| 515 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 36F |
| 516 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 37F |
| 517 | Small | 6-8 rooms (random) | 20 | 0 | no | 50%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 38F |
| 518 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 39F |
| 519 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 40F |
| 520 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 41F |
| 521 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 42F |
| 522 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 43F |
| 523 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 44F |
| 524 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | Secondary terrain | 0 | WishCave: 45F |
| 525 | Small | 8-10 rooms (random) | 15 | 0 | yes | 8%/100% | 0% | 0 | none | 0 | WishCave: 46F |
| 526 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 47F |
| 527 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 48F |
| 528 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 49F |
| 529 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 65 | WishCave: 50F |
| 530 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 51F |
| 531 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 52F |
| 532 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 53F |
| 533 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 54F |
| 534 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | yes | 50%/6% | 0% | 0 | none | 0 | WishCave: 55F |
| 535 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 56F |
| 536 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | Secondary terrain | 0 | WishCave: 57F |
| 537 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 58F |
| 538 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 59F |
| 539 | Unused 0xF | 10-12 rooms (random) | 15 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 60F |
| 540 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 61F |
| 541 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 62F |
| 542 | Small | 6-8 rooms (random) | 40 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 63F |
| 543 | Small | 6-8 rooms (random) | 30 | 0 | yes | 8%/100% | 0% | 0 | Secondary terrain | 0 | WishCave: 64F |
| 544 | Unused 0xF | 12-14 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 65F |
| 545 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | Secondary terrain | 0 | WishCave: 66F |
| 546 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 67F |
| 547 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 68F |
| 548 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 69F |
| 549 | Unused 0xF | 10-12 rooms (random) | 30 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 70F |
| 550 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 71F |
| 551 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 72F |
| 552 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 73F |
| 553 | Small | 6-8 rooms (random) | 40 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 74F |
| 554 | Unused 0xF | 20-22 rooms (random) | 30 | 0 | no | 50%/6% | 0% | 10 | none | 0 | WishCave: 75F |
| 555 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 76F |
| 556 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 77F |
| 557 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | Secondary terrain | 0 | WishCave: 78F |
| 558 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 79F |
| 559 | Unused 0xF | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 80F |
| 560 | Small | 6-8 rooms (random) | 15 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 81F |
| 561 | Small | 6-8 rooms (random) | 15 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 82F |
| 562 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 83F |
| 563 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | Secondary terrain | 0 | WishCave: 84F |
| 564 | Unused 0xF | 18-20 rooms (random) | 40 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 85F |
| 565 | Small | 6-8 rooms (random) | 30 | 0 | no | 8%/100% | 1% | 0 | Secondary terrain | 0 | WishCave: 86F |
| 566 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 87F |
| 567 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 88F |
| 568 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 89F |
| 569 | Unused 0xF | 8-10 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | Secondary terrain | 0 | WishCave: 90F |
| 570 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 91F |
| 571 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 92F |
| 572 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 93F |
| 573 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | Secondary terrain | 0 | WishCave: 94F |
| 574 | Unused 0xF | 10-12 rooms (random) | 30 | 0 | no | 8%/6% | 1% | 0 | none | 0 | WishCave: 95F |
| 575 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 0% | 10 | none | 0 | WishCave: 96F |
| 576 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 0 | none | 0 | WishCave: 97F |
| 577 | Small | 6-8 rooms (random) | 20 | 0 | no | 8%/6% | 1% | 0 | Secondary terrain | 0 | WishCave: 98F |
| 578 | Small | 6-8 rooms (random) | 20 | 0 | yes | 8%/6% | 0% | 10 | none | 10 | WishCave: 99F |
| 579 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | BuriedRelic: 1F |
| 580 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | BuriedRelic: 2F |
| 581 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 3F |
| 582 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | BuriedRelic: 4F |
| 583 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 5F |
| 584 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | BuriedRelic: 6F |
| 585 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 7F |
| 586 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | BuriedRelic: 8F |
| 587 | Small | 5-7 rooms (random) | 50 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | BuriedRelic: 9F |
| 588 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | BuriedRelic: 10F |
| 589 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 11F |
| 590 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 12F |
| 591 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | BuriedRelic: 13F |
| 592 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 14F |
| 593 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 17 | BuriedRelic: 15F |
| 594 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 16F |
| 595 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 17F |
| 596 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 18F |
| 597 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 19F |
| 598 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 20F |
| 599 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 21F |
| 600 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 22F |
| 601 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 23F |
| 602 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 24F |
| 603 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 18 | BuriedRelic: 25F |
| 604 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 26F |
| 605 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 27F |
| 606 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 28F |
| 607 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 29F |
| 608 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 30F |
| 609 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 31F |
| 610 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 32F |
| 611 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 33F |
| 612 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 34F |
| 613 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0% | 0 | none | 19 | BuriedRelic: 35F |
| 614 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 36F |
| 615 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 37F |
| 616 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 38F |
| 617 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 39F |
| 618 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 40F |
| 619 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 41F |
| 620 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 42F |
| 621 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 43F |
| 622 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 44F |
| 623 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 63 | BuriedRelic: 45F |
| 624 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 46F |
| 625 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 47F |
| 626 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 48F |
| 627 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 49F |
| 628 | Small | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 50F |
| 629 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 51F |
| 630 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 52F |
| 631 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 53F |
| 632 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 54F |
| 633 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 55F |
| 634 | Medium | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 56F |
| 635 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 57F |
| 636 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 58F |
| 637 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 59F |
| 638 | Large | 8-10 rooms (random) | 15 | 15 | yes | 0%/6% | 1% | 0 | none | 61 | BuriedRelic: 60F |
| 639 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 61F |
| 640 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 62F |
| 641 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 63F |
| 642 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 64F |
| 643 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 65F |
| 644 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 66F |
| 645 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 67F |
| 646 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 68F |
| 647 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 69F |
| 648 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 62 | BuriedRelic: 70F |
| 649 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 71F |
| 650 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 72F |
| 651 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 73F |
| 652 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 74F |
| 653 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 75F |
| 654 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 76F |
| 655 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 77F |
| 656 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 78F |
| 657 | Small | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 79F |
| 658 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 60 | BuriedRelic: 80F |
| 659 | Medium | 8-10 rooms (random) | 10 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 81F |
| 660 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 82F |
| 661 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 83F |
| 662 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 84F |
| 663 | Small | 5-7 rooms (random) | 50 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 85F |
| 664 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 86F |
| 665 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 87F |
| 666 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 88F |
| 667 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 89F |
| 668 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | BuriedRelic: 90F |
| 669 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 91F |
| 670 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 92F |
| 671 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 93F |
| 672 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 94F |
| 673 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 95F |
| 674 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 96F |
| 675 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | BuriedRelic: 97F |
| 676 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | BuriedRelic: 98F |
| 677 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | BuriedRelic: 99F |
| 678 | Medium | 6-8 rooms (random) | 20 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 1F |
| 679 | Medium | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 2F |
| 680 | Medium | 6-8 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 3F |
| 681 | Medium | 10-12 rooms (random) | 20 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 4F |
| 682 | Medium | 12-14 rooms (random) | 30 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 5F |
| 683 | Medium | 10-12 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 6F |
| 684 | Medium | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 7F |
| 685 | Medium | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 8F |
| 686 | Medium | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 9F |
| 687 | Medium | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 10F |
| 688 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 11F |
| 689 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 12F |
| 690 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 13F |
| 691 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 14F |
| 692 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 15F |
| 693 | Cross | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 16F |
| 694 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 17F |
| 695 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 18F |
| 696 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 19F |
| 697 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 20F |
| 698 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 21F |
| 699 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 22F |
| 700 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 23F |
| 701 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | PitfallValley: 24F |
| 702 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | PitfallValley: 25F |
| 703 | Small | 7-9 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 1F |
| 704 | Small | 8-10 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 2F |
| 705 | Small | 9-11 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 3F |
| 706 | Small | 8-10 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 4F |
| 707 | Small | 10-12 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 5F |
| 708 | Small | 10-12 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 6F |
| 709 | Small | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 7F |
| 710 | Small | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 8F |
| 711 | Small | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 9F |
| 712 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 10F |
| 713 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 11F |
| 714 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 12F |
| 715 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 13F |
| 716 | Cross | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 14F |
| 717 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | none | 0 | NorthernRange: 15F |
| 718 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | none | 0 | NorthernRange: 16F |
| 719 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | none | 0 | NorthernRange: 17F |
| 720 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | NorthernRange: 18F |
| 721 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 19F |
| 722 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | none | 0 | NorthernRange: 20F |
| 723 | Crossroads | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | Secondary terrain | 0 | NorthernRange: 21F |
| 724 | Outer ring | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | none | 0 | NorthernRange: 22F |
| 725 | Large | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | none | 0 | NorthernRange: 23F |
| 726 | Large | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 0 | none | 0 | NorthernRange: 24F |
| 727 | Large | 11-13 rooms (random) | 25 | 20 | yes | 7%/0% | 1% | 1 | none | 16 | NorthernRange: 25F |
| 728 | Unused 0xF | 15-17 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss9: 1F |
| 729 | Unused 0xF | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | Boss9: 2F |
| 730 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 3F |
| 731 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 4F |
| 732 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 5F |
| 733 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 6F |
| 734 | Unused 0xF | 15-17 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 7F |
| 735 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 8F |
| 736 | Unused 0xF | 15-17 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 9F |
| 737 | Unused 0xF | 15-17 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 10F |
| 738 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 11F |
| 739 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | Boss9: 12F |
| 740 | Medium | 6-8 rooms (random) | 40 | 10 | no | 6%/3% | 0% | 0 | none | 0 | DesertRegion: 1F |
| 741 | Medium | 7-9 rooms (random) | 40 | 10 | yes | 6%/3% | 0% | 0 | Secondary terrain | 0 | DesertRegion: 2F |
| 742 | Medium | 8-10 rooms (random) | 40 | 10 | no | 6%/3% | 1% | 0 | none | 0 | DesertRegion: 3F |
| 743 | Medium | 9-11 rooms (random) | 40 | 10 | no | 6%/3% | 0% | 0 | Secondary terrain | 0 | DesertRegion: 4F |
| 744 | Large | 10-12 rooms (random) | 40 | 10 | yes | 6%/3% | 1% | 0 | Secondary terrain | 0 | DesertRegion: 5F |
| 745 | Large | 11-13 rooms (random) | 40 | 10 | no | 6%/3% | 0% | 0 | none | 0 | DesertRegion: 6F |
| 746 | Large | 12-14 rooms (random) | 40 | 10 | no | 6%/3% | 0% | 0 | Secondary terrain | 0 | DesertRegion: 7F |
| 747 | Large | 13-15 rooms (random) | 40 | 10 | yes | 6%/3% | 1% | 0 | none | 0 | DesertRegion: 8F |
| 748 | Cross | 14-16 rooms (random) | 40 | 10 | no | 6%/3% | 0% | 0 | Secondary terrain | 0 | DesertRegion: 9F |
| 749 | Cross | 15-17 rooms (random) | 40 | 15 | no | 6%/3% | 0% | 0 | none | 0 | DesertRegion: 10F |
| 750 | Large | 6-8 rooms (random) | 40 | 15 | yes | 6%/3% | 1% | 0 | Secondary terrain | 0 | DesertRegion: 11F |
| 751 | Cross | 7-9 rooms (random) | 40 | 15 | no | 6%/3% | 0% | 0 | Secondary terrain | 0 | DesertRegion: 12F |
| 752 | Large | 8-10 rooms (random) | 40 | 15 | yes | 6%/3% | 1% | 0 | none | 0 | DesertRegion: 13F |
| 753 | Cross | 9-11 rooms (random) | 40 | 15 | yes | 6%/3% | 1% | 0 | none | 0 | DesertRegion: 14F |
| 754 | Large | 10-12 rooms (random) | 40 | 15 | no | 6%/3% | 0% | 0 | none | 0 | DesertRegion: 15F |
| 755 | Large | 11-13 rooms (random) | 40 | 15 | no | 6%/3% | 0% | 0 | Secondary terrain | 0 | DesertRegion: 16F |
| 756 | Unused 0xF | 12-14 rooms (random) | 40 | 15 | yes | 6%/3% | 1% | 0 | none | 0 | DesertRegion: 17F |
| 757 | Large | 13-15 rooms (random) | 40 | 15 | no | 6%/3% | 0% | 0 | Secondary terrain | 0 | DesertRegion: 18F |
| 758 | Unused 0xF | 14-16 rooms (random) | 40 | 15 | yes | 6%/3% | 1% | 0 | Secondary terrain | 0 | DesertRegion: 19F |
| 759 | Large | 15-17 rooms (random) | 30 | 15 | no | 6%/3% | 0% | 0 | none | 0 | DesertRegion: 20F |
| 760 | Small | 4-6 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 1F |
| 761 | Small | 4-6 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 2F |
| 762 | Small | 4-6 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 3F |
| 763 | Small | 5-7 rooms (random) | 50 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 4F |
| 764 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 5F |
| 765 | Large | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 6F |
| 766 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 7F |
| 767 | Small | 5-7 rooms (random) | 50 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 8F |
| 768 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 9F |
| 769 | Small | 5-7 rooms (random) | 20 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 10F |
| 770 | Large | 5-7 rooms (random) | 30 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 11F |
| 771 | Small | 5-7 rooms (random) | 40 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 12F |
| 772 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 13F |
| 773 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 14F |
| 774 | Small | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 15F |
| 775 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 16F |
| 776 | Small | 5-7 rooms (random) | 20 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 17F |
| 777 | Large | 5-7 rooms (random) | 30 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 18F |
| 778 | Small | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 19F |
| 779 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 20F |
| 780 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 21F |
| 781 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 22F |
| 782 | Large | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 23F |
| 783 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 24F |
| 784 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 25F |
| 785 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 26F |
| 786 | Small | 5-7 rooms (random) | 50 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 27F |
| 787 | Crossroads | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 28F |
| 788 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 29F |
| 789 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 30F |
| 790 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 31F |
| 791 | Outer ring | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 32F |
| 792 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 33F |
| 793 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 34F |
| 794 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 35F |
| 795 | Cross | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 36F |
| 796 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 37F |
| 797 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 38F |
| 798 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 39F |
| 799 | Beetle | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 40F |
| 800 | Beetle | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 41F |
| 801 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 42F |
| 802 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 43F |
| 803 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 44F |
| 804 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 45F |
| 805 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 46F |
| 806 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 47F |
| 807 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 48F |
| 808 | Small | 5-7 rooms (random) | 10 | 5 | yes | 0%/5% | 1% | 0 | none | 0 | SouthernCavern: 49F |
| 809 | Small | 5-7 rooms (random) | 10 | 5 | no | 0%/5% | 0% | 0 | none | 0 | SouthernCavern: 50F |
| 810 | Small | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 1F |
| 811 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 2F |
| 812 | Small | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | WyvernHill: 3F |
| 813 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 4F |
| 814 | Small | 5-7 rooms (random) | 40 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 5F |
| 815 | Small | 8-10 rooms (random) | 30 | 15 | yes | 8%/5% | 1% | 0 | Secondary terrain | 0 | WyvernHill: 6F |
| 816 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 7F |
| 817 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 8F |
| 818 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | Secondary terrain | 0 | WyvernHill: 9F |
| 819 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 10F |
| 820 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 11F |
| 821 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | Secondary terrain | 0 | WyvernHill: 12F |
| 822 | Small | 5-7 rooms (random) | 50 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 13F |
| 823 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 14F |
| 824 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | Secondary terrain | 0 | WyvernHill: 15F |
| 825 | Small | 8-10 rooms (random) | 10 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 16F |
| 826 | Small | 8-10 rooms (random) | 15 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 17F |
| 827 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | Secondary terrain | 0 | WyvernHill: 18F |
| 828 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | WyvernHill: 19F |
| 829 | Medium | 5-7 rooms (random) | 40 | 15 | no | 8%/5% | 0% | 0 | none | 57 | WyvernHill: 20F |
| 830 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 8%/5% | 1% | 0 | none | 0 | WyvernHill: 21F |
| 831 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | WyvernHill: 22F |
| 832 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | WyvernHill: 23F |
| 833 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | none | 0 | WyvernHill: 24F |
| 834 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | WyvernHill: 25F |
| 835 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/5% | 0% | 0 | none | 0 | WyvernHill: 26F |
| 836 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | none | 0 | WyvernHill: 27F |
| 837 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | WyvernHill: 28F |
| 838 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | WyvernHill: 29F |
| 839 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | none | 50 | WyvernHill: 30F |
| 840 | Medium | 4-6 rooms (random) | 15 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 1F |
| 841 | Medium | 8-10 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 2F |
| 842 | Medium | 6-8 rooms (random) | 13 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 3F |
| 843 | Medium | 5-7 rooms (random) | 15 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 4F |
| 844 | Medium | 4-6 rooms (random) | 10 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 5F |
| 845 | Medium | 8-10 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 6F |
| 846 | Medium | 9-11 rooms (random) | 10 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 7F |
| 847 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 8F |
| 848 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 9F |
| 849 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 10F |
| 850 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 11F |
| 851 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 12F |
| 852 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 13F |
| 853 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 14F |
| 854 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 15F |
| 855 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 16F |
| 856 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 17F |
| 857 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 18F |
| 858 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 19F |
| 859 | Medium | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 20F |
| 860 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 21F |
| 861 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 22F |
| 862 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 23F |
| 863 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 24F |
| 864 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 25F |
| 865 | Small | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 26F |
| 866 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 27F |
| 867 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FieryField: 28F |
| 868 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | FieryField: 29F |
| 869 | Large | 10-12 rooms (random) | 20 | 0 | yes | 0%/0% | 1% | 0 | Secondary terrain | 12 | FieryField: 30F |
| 870 | Medium | 5-7 rooms (random) | 8 | 6 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | NorthwindField: 1F |
| 871 | Medium | 6-8 rooms (random) | 10 | 8 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | NorthwindField: 2F |
| 872 | Medium | 10-12 rooms (random) | 12 | 8 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | NorthwindField: 3F |
| 873 | Medium | 3-5 rooms (random) | 10 | 10 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | NorthwindField: 4F |
| 874 | Medium | 4-6 rooms (random) | 6 | 10 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 5F |
| 875 | Medium | 10-12 rooms (random) | 10 | 10 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 6F |
| 876 | Medium | 6-8 rooms (random) | 8 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 7F |
| 877 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 8F |
| 878 | Large | 6-8 rooms (random) | 8 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 9F |
| 879 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 10F |
| 880 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 11F |
| 881 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 12F |
| 882 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 13F |
| 883 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 14F |
| 884 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 15F |
| 885 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 16F |
| 886 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 17F |
| 887 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 18F |
| 888 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 19F |
| 889 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 58 | NorthwindField: 20F |
| 890 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 21F |
| 891 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 22F |
| 892 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 23F |
| 893 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 24F |
| 894 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 25F |
| 895 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 26F |
| 896 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 27F |
| 897 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 28F |
| 898 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | NorthwindField: 29F |
| 899 | Medium | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 14 | NorthwindField: 30F |
| 900 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | SolarCave: 1F |
| 901 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 1% | 0 | none | 0 | SolarCave: 2F |
| 902 | Small | 8-10 rooms (random) | 15 | 15 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | SolarCave: 3F |
| 903 | Small | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 1% | 0 | none | 0 | SolarCave: 4F |
| 904 | Small | 7-9 rooms (random) | 15 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | SolarCave: 5F |
| 905 | Small | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | SolarCave: 6F |
| 906 | Small | 8-10 rooms (random) | 15 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | SolarCave: 7F |
| 907 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 1% | 0 | none | 0 | SolarCave: 8F |
| 908 | Medium | 15-17 rooms (random) | 15 | 15 | yes | 0%/0% | 0% | 0 | Secondary terrain | 0 | SolarCave: 9F |
| 909 | Large | 11-13 rooms (random) | 10 | 15 | no | 0%/0% | 1% | 0 | none | 51 | SolarCave: 10F |
| 910 | Medium | 12-14 rooms (random) | 10 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | SolarCave: 11F |
| 911 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 1% | 0 | none | 0 | SolarCave: 12F |
| 912 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | SolarCave: 13F |
| 913 | Medium | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 1% | 0 | none | 0 | SolarCave: 14F |
| 914 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0% | 0 | none | 52 | SolarCave: 15F |
| 915 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 1% | 0 | none | 0 | SolarCave: 16F |
| 916 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | SolarCave: 17F |
| 917 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 1% | 0 | none | 0 | SolarCave: 18F |
| 918 | Medium | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | SolarCave: 19F |
| 919 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 53 | SolarCave: 20F |
| 920 | Small | 6-8 rooms (random) | 10 | 10 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 1F |
| 921 | Small | 8-10 rooms (random) | 5 | 5 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 2F |
| 922 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 3F |
| 923 | Small | 8-10 rooms (random) | 30 | 10 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 4F |
| 924 | Small | 9-11 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 5F |
| 925 | Small | 10-12 rooms (random) | 6 | 20 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 6F |
| 926 | Medium | 5-7 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 7F |
| 927 | Medium | 9-11 rooms (random) | 16 | 35 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 8F |
| 928 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 9F |
| 929 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 10F |
| 930 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 11F |
| 931 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 12F |
| 932 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 13F |
| 933 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 14F |
| 934 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 15F |
| 935 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 16F |
| 936 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 17F |
| 937 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 18F |
| 938 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 19F |
| 939 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 20F |
| 940 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 21F |
| 941 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 22F |
| 942 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 23F |
| 943 | Small | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 24F |
| 944 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 25F |
| 945 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 26F |
| 946 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 27F |
| 947 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | LightningField: 28F |
| 948 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | LightningField: 29F |
| 949 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 13 | LightningField: 30F |
| 950 | Large | 5-7 rooms (random) | 10 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 1F |
| 951 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 2F |
| 952 | Large | 8-10 rooms (random) | 50 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 3F |
| 953 | Large | 4-6 rooms (random) | 50 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 4F |
| 954 | Large | 12-14 rooms (random) | 10 | 15 | no | 0%/10% | 0% | 0 | Secondary terrain | 0 | DarknightRelic: 5F |
| 955 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/10% | 0% | 0 | none | 0 | DarknightRelic: 6F |
| 956 | Large | 10-12 rooms (random) | 50 | 15 | no | 0%/10% | 0% | 0 | none | 0 | DarknightRelic: 7F |
| 957 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/10% | 0% | 0 | none | 0 | DarknightRelic: 8F |
| 958 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 9F |
| 959 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | DarknightRelic: 10F |
| 960 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 11F |
| 961 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 12F |
| 962 | Large | 5-7 rooms (random) | 50 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 13F |
| 963 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | none | 0 | DarknightRelic: 14F |
| 964 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | DarknightRelic: 15F |
| 965 | Large | 10-12 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 1F |
| 966 | Large | 5-7 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 2F |
| 967 | Large | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 3F |
| 968 | Large | 5-7 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 4F |
| 969 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 5F |
| 970 | Large | 12-14 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 6F |
| 971 | Large | 5-7 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 7F |
| 972 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 8F |
| 973 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 9F |
| 974 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 10F |
| 975 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 11F |
| 976 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | WondrousSea: 12F |
| 977 | Medium | 10-12 rooms (random) | 15 | 10 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 1F |
| 978 | Medium | 10-12 rooms (random) | 15 | 10 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 2F |
| 979 | Medium | 10-12 rooms (random) | 15 | 10 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 3F |
| 980 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 4F |
| 981 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 5F |
| 982 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 6F |
| 983 | Medium | 6-8 rooms (random) | 50 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 7F |
| 984 | Medium | 9-11 rooms (random) | 20 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 8F |
| 985 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 9F |
| 986 | Medium | 11-13 rooms (random) | 20 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 10F |
| 987 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 11F |
| 988 | Medium | 11-13 rooms (random) | 25 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 12F |
| 989 | Medium | 8-10 rooms (random) | 50 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 13F |
| 990 | Medium | 11-13 rooms (random) | 25 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 14F |
| 991 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 15F |
| 992 | Medium | 12-14 rooms (random) | 25 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 16F |
| 993 | Medium | 14-16 rooms (random) | 25 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 17F |
| 994 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 18F |
| 995 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/5% | 1% | 0 | Secondary terrain | 0 | MurkyCave: 19F |
| 996 | Small | 4-6 rooms (random) | 15 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 1F |
| 997 | Small | 5-7 rooms (random) | 15 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 2F |
| 998 | Small | 6-8 rooms (random) | 15 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 3F |
| 999 | Small | 7-9 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 4F |
| 1000 | Small | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 5F |
| 1001 | Small | 9-11 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 6F |
| 1002 | Small | 10-12 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 7F |
| 1003 | Small | 11-13 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 8F |
| 1004 | Small | 12-14 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 9F |
| 1005 | Small | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 10F |
| 1006 | Small | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 11F |
| 1007 | Small | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 12F |
| 1008 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 13F |
| 1009 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 14F |
| 1010 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 64 | GrandSea: 15F |
| 1011 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 16F |
| 1012 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 17F |
| 1013 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 18F |
| 1014 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 19F |
| 1015 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 20F |
| 1016 | Medium | 13-15 rooms (random) | 25 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 21F |
| 1017 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 22F |
| 1018 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 23F |
| 1019 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 24F |
| 1020 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 66 | GrandSea: 25F |
| 1021 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 26F |
| 1022 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 27F |
| 1023 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 28F |
| 1024 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 29F |
| 1025 | Large | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | GrandSea: 30F |
| 1026 | Medium | 6-8 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | UproarForest: 1F |
| 1027 | Medium | 7-9 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | UproarForest: 2F |
| 1028 | Medium | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | UproarForest: 3F |
| 1029 | Medium | 10-12 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | UproarForest: 4F |
| 1030 | Medium | 11-13 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | UproarForest: 5F |
| 1031 | Medium | 9-11 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | UproarForest: 6F |
| 1032 | Medium | 11-13 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | UproarForest: 7F |
| 1033 | Medium | 8-10 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | UproarForest: 8F |
| 1034 | Medium | 7-9 rooms (random) | 20 | 20 | no | 0%/6% | 0% | 0 | none | 0 | UproarForest: 9F |
| 1035 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 11 | UproarForest: 10F |
| 1036 | Medium | 7-9 rooms (random) | 15 | 10 | yes | 0%/0% | 0% | 0 | none | 0 | OddityCave: 1F |
| 1037 | Medium | 7-9 rooms (random) | 15 | 10 | yes | 0%/0% | 0% | 0 | none | 0 | OddityCave: 2F |
| 1038 | Medium | 8-10 rooms (random) | 15 | 10 | yes | 0%/0% | 0% | 0 | none | 0 | OddityCave: 3F |
| 1039 | Medium | 8-10 rooms (random) | 15 | 10 | yes | 0%/0% | 0% | 0 | none | 0 | OddityCave: 4F |
| 1040 | Medium | 8-10 rooms (random) | 15 | 10 | yes | 0%/0% | 0% | 0 | none | 0 | OddityCave: 5F |
| 1041 | Small | 6-8 rooms (random) | 20 | 0 | yes | 7%/0% | 0% | 0 | none | 0 | OddityCave: 6F |
| 1042 | Large | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 0% | 0 | none | 0 | OddityCave: 7F |
| 1043 | Large | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 1% | 0 | none | 0 | OddityCave: 8F |
| 1044 | Small | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 1% | 0 | none | 0 | OddityCave: 9F |
| 1045 | Small | 6-8 rooms (random) | 20 | 0 | yes | 6%/0% | 1% | 0 | Secondary terrain | 0 | OddityCave: 10F |
| 1046 | Large | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | OddityCave: 11F |
| 1047 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | OddityCave: 12F |
| 1048 | Large | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | OddityCave: 13F |
| 1049 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | OddityCave: 14F |
| 1050 | Small | 6-8 rooms (random) | 25 | 15 | no | 0%/0% | 1% | 0 | Secondary terrain | 0 | OddityCave: 15F |
| 1051 | Large | 6-8 rooms (random) | 20 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 1F |
| 1052 | Large | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 2F |
| 1053 | Large | 6-8 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 3F |
| 1054 | Large | 10-12 rooms (random) | 20 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 4F |
| 1055 | Large | 12-14 rooms (random) | 30 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 5F |
| 1056 | Large | 10-12 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 6F |
| 1057 | Large | 8-10 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 7F |
| 1058 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 8F |
| 1059 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 9F |
| 1060 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 10F |
| 1061 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 11F |
| 1062 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 12F |
| 1063 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 13F |
| 1064 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 0 | Secondary terrain | 0 | RemainsIsland: 14F |
| 1065 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 15F |
| 1066 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 16F |
| 1067 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 17F |
| 1068 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 18F |
| 1069 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 19F |
| 1070 | Large | 11-13 rooms (random) | 25 | 10 | yes | 7%/0% | 1% | 10 | Secondary terrain | 0 | RemainsIsland: 20F |
| 1071 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | none | 0 | MarvelousSea: 1F |
| 1072 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MarvelousSea: 2F |
| 1073 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MarvelousSea: 3F |
| 1074 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | MarvelousSea: 4F |
| 1075 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | Secondary terrain | 0 | MarvelousSea: 5F |
| 1076 | Beetle | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MarvelousSea: 6F |
| 1077 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MarvelousSea: 7F |
| 1078 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MarvelousSea: 8F |
| 1079 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | none | 0 | MarvelousSea: 9F |
| 1080 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MarvelousSea: 10F |
| 1081 | Beetle | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MarvelousSea: 11F |
| 1082 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | MarvelousSea: 12F |
| 1083 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 10 | Secondary terrain | 0 | MarvelousSea: 13F |
| 1084 | Large | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MarvelousSea: 14F |
| 1085 | Large | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MarvelousSea: 15F |
| 1086 | Large | 10-12 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | MarvelousSea: 16F |
| 1087 | Large | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 10 | none | 0 | MarvelousSea: 17F |
| 1088 | Beetle | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MarvelousSea: 18F |
| 1089 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | MarvelousSea: 19F |
| 1090 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | MarvelousSea: 20F |
| 1091 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 10 | none | 0 | FantasyStrait: 1F |
| 1092 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 2F |
| 1093 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | FantasyStrait: 3F |
| 1094 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 4F |
| 1095 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 10 | Secondary terrain | 0 | FantasyStrait: 5F |
| 1096 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | FantasyStrait: 6F |
| 1097 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | FantasyStrait: 7F |
| 1098 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 8F |
| 1099 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/10% | 1% | 10 | none | 0 | FantasyStrait: 9F |
| 1100 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 10F |
| 1101 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | FantasyStrait: 11F |
| 1102 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | FantasyStrait: 12F |
| 1103 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | FantasyStrait: 13F |
| 1104 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 14F |
| 1105 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | FantasyStrait: 15F |
| 1106 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 16F |
| 1107 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/7% | 0% | 10 | none | 0 | FantasyStrait: 17F |
| 1108 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | none | 0 | FantasyStrait: 18F |
| 1109 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | FantasyStrait: 19F |
| 1110 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 20F |
| 1111 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/7% | 1% | 10 | none | 0 | FantasyStrait: 21F |
| 1112 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | FantasyStrait: 22F |
| 1113 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 23F |
| 1114 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | FantasyStrait: 24F |
| 1115 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | FantasyStrait: 25F |
| 1116 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | Secondary terrain | 0 | FantasyStrait: 26F |
| 1117 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/10% | 1% | 0 | none | 0 | FantasyStrait: 27F |
| 1118 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 0 | none | 0 | FantasyStrait: 28F |
| 1119 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/7% | 0% | 10 | none | 0 | FantasyStrait: 29F |
| 1120 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/7% | 1% | 0 | Secondary terrain | 0 | FantasyStrait: 30F |
| 1121 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | RockPath: 1F |
| 1122 | Small | 6-8 rooms (random) | 15 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | RockPath: 2F |
| 1123 | Small | 7-9 rooms (random) | 20 | 15 | yes | 0%/0% | 0% | 0 | none | 0 | RockPath: 3F |
| 1124 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | RockPath: 4F |
| 1125 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | SnowPath: 1F |
| 1126 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | SnowPath: 2F |
| 1127 | Small | 7-9 rooms (random) | 20 | 15 | yes | 0%/0% | 0% | 1 | none | 0 | SnowPath: 3F |
| 1128 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | SnowPath: 4F |
| 1129 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 1F |
| 1130 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 2F |
| 1131 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 3F |
| 1132 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 4F |
| 1133 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 5F |
| 1134 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 6F |
| 1135 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 7F |
| 1136 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 8F |
| 1137 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 9F |
| 1138 | Outer rooms | 5-7 rooms (random) | 10 | 0 | no | 0%/0% | 0% | 0 | none | 24 | Autopilot: 10F |
| 1139 | Large | 8-10 rooms (random) | 20 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D50: 1F |
| 1140 | Large | 8-10 rooms (random) | 20 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D50: 2F |
| 1141 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 1F |
| 1142 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 2F |
| 1143 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D51: 3F |
| 1144 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 4F |
| 1145 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 5F |
| 1146 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 6F |
| 1147 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 7F |
| 1148 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 8F |
| 1149 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 9F |
| 1150 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 10F |
| 1151 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 11F |
| 1152 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D51: 12F |
| 1153 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 13F |
| 1154 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 14F |
| 1155 | One-room Monster House | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D51: 15F |
| 1156 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 16F |
| 1157 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 17F |
| 1158 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 18F |
| 1159 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 19F |
| 1160 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 20F |
| 1161 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 21F |
| 1162 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 22F |
| 1163 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 23F |
| 1164 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D51: 24F |
| 1165 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 25F |
| 1166 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 26F |
| 1167 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 27F |
| 1168 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 28F |
| 1169 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 29F |
| 1170 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D51: 30F |
| 1171 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 31F |
| 1172 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 32F |
| 1173 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 33F |
| 1174 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 34F |
| 1175 | One-room Monster House | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 35F |
| 1176 | Beetle | 20-22 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 36F |
| 1177 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 37F |
| 1178 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 38F |
| 1179 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 39F |
| 1180 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 40F |
| 1181 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 41F |
| 1182 | Large | 4-6 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 42F |
| 1183 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 43F |
| 1184 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 44F |
| 1185 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D51: 45F |
| 1186 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 46F |
| 1187 | Large | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D51: 47F |
| 1188 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D51: 48F |
| 1189 | Large | 16-18 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 49F |
| 1190 | Beetle | 18-20 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D51: 50F |
| 1191 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 1F |
| 1192 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 2F |
| 1193 | Medium | 9-11 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 27 | DojoRegistration: 3F |
| 1194 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 4F |
| 1195 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 5F |
| 1196 | Medium | 12-14 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 28 | DojoRegistration: 6F |
| 1197 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 7F |
| 1198 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 8F |
| 1199 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 29 | DojoRegistration: 9F |
| 1200 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 10F |
| 1201 | Medium | 15-17 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 11F |
| 1202 | Medium | 10-12 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 30 | DojoRegistration: 12F |
| 1203 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 13F |
| 1204 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 14F |
| 1205 | Medium | 9-11 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 31 | DojoRegistration: 15F |
| 1206 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 16F |
| 1207 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 17F |
| 1208 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 32 | DojoRegistration: 18F |
| 1209 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 19F |
| 1210 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 20F |
| 1211 | Medium | 8-10 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 33 | DojoRegistration: 21F |
| 1212 | Medium | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 22F |
| 1213 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 23F |
| 1214 | Medium | 20-22 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 34 | DojoRegistration: 24F |
| 1215 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 25F |
| 1216 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 26F |
| 1217 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 35 | DojoRegistration: 27F |
| 1218 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 28F |
| 1219 | Medium | 15-17 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 29F |
| 1220 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 36 | DojoRegistration: 30F |
| 1221 | Medium | 9-11 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 31F |
| 1222 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 32F |
| 1223 | Medium | 10-12 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 37 | DojoRegistration: 33F |
| 1224 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 34F |
| 1225 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 35F |
| 1226 | Medium | 15-17 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 38 | DojoRegistration: 36F |
| 1227 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 37F |
| 1228 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 38F |
| 1229 | Medium | 10-12 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | none | 39 | DojoRegistration: 39F |
| 1230 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 40F |
| 1231 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 41F |
| 1232 | Medium | 12-14 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 40 | DojoRegistration: 42F |
| 1233 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 43F, 49F |
| 1234 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 44F |
| 1235 | Medium | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 41 | DojoRegistration: 45F |
| 1236 | Medium | 7-9 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 46F |
| 1237 | Medium | 15-17 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 47F |
| 1238 | Medium | 10-12 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 42 | DojoRegistration: 48F |
| 1239 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 50F |
| 1240 | Medium | 6-8 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | none | 43 | DojoRegistration: 51F |
| 1241 | Medium | 10-12 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | DojoRegistration: 52F |
| 1242 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | DojoRegistration: 53F |
| 1243 | Medium | 9-11 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 44 | DojoRegistration: 54F |
| 1244 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 55F |
| 1245 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 56F |
| 1246 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 45 | DojoRegistration: 57F |
| 1247 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 58F |
| 1248 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 59F |
| 1249 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 46 | DojoRegistration: 60F |
| 1250 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 61F, 64F |
| 1251 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 62F, 65F |
| 1252 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 47 | DojoRegistration: 63F |
| 1253 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 48 | DojoRegistration: 66F |
| 1254 | Medium | 10-12 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | DojoRegistration: 67F |
| 1255 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | DojoRegistration: 68F |
| 1256 | Medium | 9-11 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 49 | DojoRegistration: 69F |
| 1257 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | none | 0 | HowlingForest: 1F |
| 1258 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | none | 0 | HowlingForest: 2F |
| 1259 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 1% | 0 | none | 0 | HowlingForest: 3F |
| 1260 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/5% | 0% | 0 | none | 0 | HowlingForest: 4F |
| 1261 | Medium | 8-10 rooms (random) | 20 | 20 | no | 0%/5% | 0% | 0 | Secondary terrain | 0 | HowlingForest: 5F |
| 1262 | Medium | 8-10 rooms (random) | 30 | 20 | yes | 0%/5% | 1% | 1 | none | 0 | HowlingForest: 6F |
| 1263 | Medium | 8-10 rooms (random) | 30 | 20 | yes | 0%/5% | 0% | 1 | none | 0 | HowlingForest: 7F |
| 1264 | Medium | 10-12 rooms (random) | 30 | 30 | no | 0%/5% | 1% | 0 | Secondary terrain | 0 | HowlingForest: 8F |
| 1265 | Medium | 10-12 rooms (random) | 30 | 30 | yes | 0%/5% | 0% | 1 | Secondary terrain | 0 | HowlingForest: 9F |
| 1266 | Medium | 10-12 rooms (random) | 30 | 30 | no | 0%/5% | 1% | 1 | Secondary terrain | 0 | HowlingForest: 10F |
| 1267 | Medium | 10-12 rooms (random) | 30 | 30 | yes | 0%/5% | 0% | 0 | Secondary terrain | 0 | HowlingForest: 11F |
| 1268 | Medium | 10-12 rooms (random) | 30 | 40 | yes | 0%/5% | 1% | 1 | none | 0 | HowlingForest: 12F |
| 1269 | Medium | 10-12 rooms (random) | 30 | 40 | no | 0%/5% | 1% | 1 | Secondary terrain | 0 | HowlingForest: 13F |
| 1270 | Medium | 10-12 rooms (random) | 30 | 40 | yes | 0%/5% | 0% | 0 | none | 0 | HowlingForest: 14F |
| 1271 | Medium | 10-12 rooms (random) | 30 | 40 | yes | 0%/5% | 1% | 1 | none | 24 | HowlingForest: 15F |
| 1272 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 1F |
| 1273 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 2F |
| 1274 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 3F |
| 1275 | Large | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 4F |
| 1276 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 5F |
| 1277 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 6F |
| 1278 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 7F |
| 1279 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 8F |
| 1280 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 9F |
| 1281 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 10F |
| 1282 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 11F |
| 1283 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 12F |
| 1284 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 13F |
| 1285 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 14F |
| 1286 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 15F |
| 1287 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 16F |
| 1288 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 17F |
| 1289 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 18F |
| 1290 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 19F |
| 1291 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 20F |
| 1292 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 21F |
| 1293 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 22F |
| 1294 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 23F |
| 1295 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 24F |
| 1296 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 25F |
| 1297 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 26F |
| 1298 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 27F |
| 1299 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 28F |
| 1300 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 29F |
| 1301 | Outer rooms | 5-7 rooms (random) | 15 | 0 | no | 0%/0% | 0% | 0 | none | 0 | D54: 30F |
| 1302 | Unused 0xE | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 1F |
| 1303 | Unused 0xE | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 2F |
| 1304 | Unused 0xE | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 3F |
| 1305 | Unused 0xE | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 4F |
| 1306 | Unused 0xE | 7-9 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 5F |
| 1307 | Unused 0xE | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 6F |
| 1308 | Large | 8-10 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 7F |
| 1309 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 8F |
| 1310 | Large | 15-17 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 9F |
| 1311 | Large | 11-13 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 10F |
| 1312 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 11F |
| 1313 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 12F |
| 1314 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 13F |
| 1315 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 14F |
| 1316 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 15F |
| 1317 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 16F |
| 1318 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 17F |
| 1319 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 18F |
| 1320 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FantasyStraitAlt: 19F |
| 1321 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 1F |
| 1322 | Medium | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 2F |
| 1323 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 3F |
| 1324 | Medium | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 4F |
| 1325 | Medium | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 5F |
| 1326 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 6F |
| 1327 | Medium | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 7F |
| 1328 | Medium | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 8F |
| 1329 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 9F |
| 1330 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 10F |
| 1331 | Medium | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 11F |
| 1332 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 12F |
| 1333 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 13F |
| 1334 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 14F |
| 1335 | Medium | 7-9 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 15F |
| 1336 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 16F |
| 1337 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 17F |
| 1338 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 18F |
| 1339 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | WaterfallPond: 19F |
| 1340 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 1F |
| 1341 | Large | 9-11 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 2F |
| 1342 | Large | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 3F |
| 1343 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 4F |
| 1344 | Large | 15-17 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 5F |
| 1345 | Large | 12-14 rooms (random) | 30 | 20 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 6F |
| 1346 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 7F |
| 1347 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 8F |
| 1348 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 9F |
| 1349 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 10F |
| 1350 | Large | 12-14 rooms (random) | 50 | 20 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | UnownRelic: 11F |
| 1351 | Small | 8-10 rooms (random) | 10 | 15 | no | 8%/0% | 0% | 0 | none | 0 | JoyousTower: 1F |
| 1352 | Small | 9-11 rooms (random) | 15 | 15 | no | 8%/0% | 0% | 0 | none | 0 | JoyousTower: 2F |
| 1353 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/0% | 1% | 0 | none | 0 | JoyousTower: 3F |
| 1354 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/0% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 4F |
| 1355 | Small | 12-14 rooms (random) | 40 | 15 | no | 8%/0% | 0% | 0 | none | 0 | JoyousTower: 5F |
| 1356 | Medium | 11-13 rooms (random) | 30 | 15 | yes | 8%/0% | 1% | 0 | none | 0 | JoyousTower: 6F |
| 1357 | Large | 10-12 rooms (random) | 20 | 15 | no | 40%/0% | 0% | 0 | none | 0 | JoyousTower: 7F |
| 1358 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 8F |
| 1359 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/5% | 1% | 0 | none | 0 | JoyousTower: 9F |
| 1360 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | JoyousTower: 10F |
| 1361 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | JoyousTower: 11F |
| 1362 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 12F |
| 1363 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/5% | 0% | 0 | none | 0 | JoyousTower: 13F |
| 1364 | Large | 12-14 rooms (random) | 20 | 15 | no | 8%/40% | 0% | 0 | none | 0 | JoyousTower: 14F |
| 1365 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 15F |
| 1366 | Small | 10-12 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 16F |
| 1367 | Small | 9-11 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 17F |
| 1368 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 18F |
| 1369 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 19F |
| 1370 | Medium | 6-8 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 20F |
| 1371 | Large | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 21F |
| 1372 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 22F |
| 1373 | Small | 12-14 rooms (random) | 20 | 15 | no | 40%/6% | 0% | 0 | none | 0 | JoyousTower: 23F |
| 1374 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 24F |
| 1375 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 25F |
| 1376 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 26F |
| 1377 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 27F |
| 1378 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 28F |
| 1379 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/40% | 0% | 0 | none | 0 | JoyousTower: 29F |
| 1380 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 30F |
| 1381 | Small | 12-14 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 31F |
| 1382 | Small | 11-13 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 32F |
| 1383 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 33F |
| 1384 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 34F |
| 1385 | Medium | 6-8 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 35F |
| 1386 | Medium | 11-13 rooms (random) | 30 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 36F |
| 1387 | Small | 12-14 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 37F |
| 1388 | Small | 11-13 rooms (random) | 20 | 15 | no | 40%/6% | 0% | 0 | none | 0 | JoyousTower: 38F |
| 1389 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 39F |
| 1390 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 40F |
| 1391 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 41F |
| 1392 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 42F |
| 1393 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 43F |
| 1394 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 44F |
| 1395 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 45F |
| 1396 | Small | 12-14 rooms (random) | 10 | 15 | no | 8%/40% | 0% | 0 | none | 0 | JoyousTower: 46F |
| 1397 | Small | 11-13 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 47F |
| 1398 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 48F |
| 1399 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 49F |
| 1400 | Small | 6-8 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 50F |
| 1401 | Small | 8-10 rooms (random) | 30 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 51F |
| 1402 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 52F |
| 1403 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 53F |
| 1404 | Medium | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 54F |
| 1405 | Small | 12-14 rooms (random) | 20 | 15 | no | 40%/6% | 0% | 0 | none | 0 | JoyousTower: 55F |
| 1406 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 56F |
| 1407 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 57F |
| 1408 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 58F |
| 1409 | Small | 8-10 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 59F |
| 1410 | Small | 9-11 rooms (random) | 15 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 60F |
| 1411 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 61F |
| 1412 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 62F |
| 1413 | Small | 6-8 rooms (random) | 40 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 63F |
| 1414 | Small | 11-13 rooms (random) | 30 | 15 | no | 8%/40% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 64F |
| 1415 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 65F |
| 1416 | Small | 9-11 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 66F |
| 1417 | Small | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 67F |
| 1418 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 68F |
| 1419 | Medium | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 69F |
| 1420 | Medium | 11-13 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 70F |
| 1421 | Small | 12-14 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 71F |
| 1422 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 72F |
| 1423 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 73F |
| 1424 | Small | 9-11 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 74F |
| 1425 | Small | 8-10 rooms (random) | 30 | 15 | yes | 40%/6% | 1% | 0 | none | 0 | JoyousTower: 75F |
| 1426 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 76F |
| 1427 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 77F |
| 1428 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 78F |
| 1429 | Small | 12-14 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 79F |
| 1430 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 80F |
| 1431 | Small | 10-12 rooms (random) | 10 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 81F |
| 1432 | Small | 9-11 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 82F |
| 1433 | Small | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 83F |
| 1434 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 84F |
| 1435 | Small | 9-11 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 85F |
| 1436 | Small | 10-12 rooms (random) | 30 | 15 | no | 8%/40% | 0% | 0 | none | 0 | JoyousTower: 86F |
| 1437 | Small | 11-13 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 87F |
| 1438 | Small | 12-14 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 88F |
| 1439 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 89F |
| 1440 | Small | 10-12 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 90F |
| 1441 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 91F |
| 1442 | Small | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | JoyousTower: 92F |
| 1443 | Small | 9-11 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 93F |
| 1444 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 94F |
| 1445 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 95F |
| 1446 | Small | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | JoyousTower: 96F |
| 1447 | Small | 11-13 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 97F |
| 1448 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | JoyousTower: 98F |
| 1449 | Small | 9-11 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | JoyousTower: 99F |
| 1450 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FaroffSea: 1F |
| 1451 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FaroffSea: 2F |
| 1452 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FaroffSea: 3F |
| 1453 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FaroffSea: 4F |
| 1454 | Small | 6-8 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 1 | Secondary terrain | 0 | FaroffSea: 5F |
| 1455 | Small | 6-8 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | FaroffSea: 6F |
| 1456 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 7F |
| 1457 | Large | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FaroffSea: 8F |
| 1458 | Medium | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | FaroffSea: 9F |
| 1459 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 1 | none | 0 | FaroffSea: 10F |
| 1460 | Medium | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 11F |
| 1461 | Large | 6-8 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | FaroffSea: 12F |
| 1462 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | FaroffSea: 13F |
| 1463 | Small | 6-8 rooms (random) | 20 | 15 | no | 0%/100% | 0% | 0 | none | 0 | FaroffSea: 14F |
| 1464 | Small | 6-8 rooms (random) | 20 | 15 | yes | 0%/6% | 1% | 1 | Secondary terrain | 0 | FaroffSea: 15F |
| 1465 | Large | 6-8 rooms (random) | 10 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 16F |
| 1466 | Small | 6-8 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 17F |
| 1467 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 18F |
| 1468 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 19F |
| 1469 | Small | 8-10 rooms (random) | 40 | 15 | no | 0%/6% | 0% | 1 | none | 0 | FaroffSea: 20F |
| 1470 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 21F |
| 1471 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 22F |
| 1472 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 23F |
| 1473 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | FaroffSea: 24F |
| 1474 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 1 | none | 0 | FaroffSea: 25F |
| 1475 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 26F |
| 1476 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 27F |
| 1477 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 28F |
| 1478 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/100% | 0% | 0 | none | 0 | FaroffSea: 29F |
| 1479 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 1 | Secondary terrain | 0 | FaroffSea: 30F |
| 1480 | Medium | 8-10 rooms (random) | 10 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 31F |
| 1481 | Large | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 32F |
| 1482 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 33F |
| 1483 | Small | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 34F |
| 1484 | Small | 5-7 rooms (random) | 40 | 15 | no | 0%/6% | 0% | 1 | none | 0 | FaroffSea: 35F |
| 1485 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 36F |
| 1486 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 37F |
| 1487 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 38F |
| 1488 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 39F |
| 1489 | Large | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 1 | none | 0 | FaroffSea: 40F |
| 1490 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 41F |
| 1491 | Medium | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 42F |
| 1492 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 43F |
| 1493 | Large | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 44F |
| 1494 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 1 | Secondary terrain | 0 | FaroffSea: 45F |
| 1495 | Medium | 5-7 rooms (random) | 10 | 15 | no | 0%/100% | 0% | 0 | none | 0 | FaroffSea: 46F |
| 1496 | Medium | 8-10 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 47F |
| 1497 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 48F |
| 1498 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 49F |
| 1499 | Small | 5-7 rooms (random) | 40 | 15 | no | 0%/6% | 0% | 1 | Secondary terrain | 54 | FaroffSea: 50F |
| 1500 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 51F |
| 1501 | Small | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 52F |
| 1502 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 53F |
| 1503 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 54F |
| 1504 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 1 | none | 0 | FaroffSea: 55F |
| 1505 | Large | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 56F |
| 1506 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | FaroffSea: 57F |
| 1507 | Medium | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 58F |
| 1508 | Medium | 5-7 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 59F |
| 1509 | Large | 8-10 rooms (random) | 15 | 15 | yes | 0%/6% | 1% | 1 | none | 0 | FaroffSea: 60F |
| 1510 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 61F |
| 1511 | Small | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 62F |
| 1512 | Medium | 8-10 rooms (random) | 40 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 63F |
| 1513 | Large | 8-10 rooms (random) | 30 | 15 | no | 0%/100% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 64F |
| 1514 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 1 | none | 0 | FaroffSea: 65F |
| 1515 | Small | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | Secondary terrain | 0 | FaroffSea: 66F |
| 1516 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 67F |
| 1517 | Large | 5-7 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 68F |
| 1518 | Small | 5-7 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 0 | FaroffSea: 69F |
| 1519 | Small | 5-7 rooms (random) | 20 | 15 | no | 0%/6% | 0% | 1 | Secondary terrain | 0 | FaroffSea: 70F |
| 1520 | Medium | 5-7 rooms (random) | 15 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 71F |
| 1521 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 0 | none | 55 | FaroffSea: 72F |
| 1522 | Medium | 8-10 rooms (random) | 30 | 15 | no | 0%/6% | 0% | 0 | none | 0 | FaroffSea: 73F |
| 1523 | Medium | 8-10 rooms (random) | 40 | 15 | no | 0%/6% | 0% | 0 | Secondary terrain | 0 | FaroffSea: 74F |
| 1524 | Medium | 8-10 rooms (random) | 30 | 15 | yes | 0%/6% | 1% | 1 | none | 0 | FaroffSea: 75F |
| 1525 | Medium | 8-10 rooms (random) | 15 | 15 | no | 3%/0% | 0% | 0 | none | 0 | MtFaraway: 1F |
| 1526 | Medium | 8-10 rooms (random) | 15 | 15 | no | 3%/0% | 0% | 0 | none | 0 | MtFaraway: 2F |
| 1527 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/0% | 1% | 0 | Secondary terrain | 0 | MtFaraway: 3F |
| 1528 | Small | 5-7 rooms (random) | 50 | 15 | no | 3%/0% | 0% | 0 | none | 0 | MtFaraway: 4F |
| 1529 | Small | 5-7 rooms (random) | 40 | 15 | no | 3%/0% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 5F |
| 1530 | Small | 5-7 rooms (random) | 30 | 15 | yes | 3%/0% | 1% | 0 | none | 0 | MtFaraway: 6F |
| 1531 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 7F |
| 1532 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0% | 0 | none | 0 | MtFaraway: 8F |
| 1533 | Small | 8-10 rooms (random) | 20 | 15 | yes | 3%/0% | 1% | 0 | none | 0 | MtFaraway: 9F |
| 1534 | Small | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0% | 0 | none | 0 | MtFaraway: 10F |
| 1535 | Small | 8-10 rooms (random) | 20 | 15 | no | 3%/0% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 11F |
| 1536 | Medium | 5-7 rooms (random) | 50 | 15 | yes | 3%/0% | 1% | 0 | Secondary terrain | 0 | MtFaraway: 12F |
| 1537 | Large | 5-7 rooms (random) | 50 | 15 | no | 3%/0% | 0% | 0 | none | 0 | MtFaraway: 13F |
| 1538 | Medium | 5-7 rooms (random) | 20 | 15 | no | 0%/50% | 0% | 0 | none | 0 | MtFaraway: 14F |
| 1539 | Small | 8-10 rooms (random) | 20 | 15 | yes | 3%/6% | 1% | 0 | Secondary terrain | 0 | MtFaraway: 15F |
| 1540 | Small | 8-10 rooms (random) | 10 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 16F |
| 1541 | Small | 8-10 rooms (random) | 15 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 17F |
| 1542 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/6% | 1% | 0 | none | 0 | MtFaraway: 18F |
| 1543 | Medium | 5-7 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 19F |
| 1544 | Medium | 5-7 rooms (random) | 40 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 20F |
| 1545 | Large | 5-7 rooms (random) | 30 | 15 | yes | 3%/6% | 1% | 0 | none | 0 | MtFaraway: 21F |
| 1546 | Medium | 15-17 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 22F |
| 1547 | Medium | 15-17 rooms (random) | 50 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 23F |
| 1548 | Small | 15-17 rooms (random) | 20 | 15 | yes | 3%/6% | 1% | 0 | Secondary terrain | 0 | MtFaraway: 24F |
| 1549 | Small | 5-7 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 25F |
| 1550 | Small | 5-7 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 26F |
| 1551 | Small | 5-7 rooms (random) | 20 | 15 | yes | 3%/6% | 1% | 0 | none | 0 | MtFaraway: 27F |
| 1552 | Medium | 8-10 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 28F |
| 1553 | Medium | 8-10 rooms (random) | 20 | 15 | no | 0%/50% | 0% | 0 | none | 0 | MtFaraway: 29F |
| 1554 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 3%/6% | 1% | 0 | Secondary terrain | 56 | MtFaraway: 30F |
| 1555 | Large | 5-7 rooms (random) | 50 | 15 | no | 3%/6% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 31F |
| 1556 | Large | 5-7 rooms (random) | 15 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 32F |
| 1557 | Large | 5-7 rooms (random) | 20 | 15 | yes | 3%/6% | 1% | 0 | none | 0 | MtFaraway: 33F |
| 1558 | Crossroads | 8-10 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 34F |
| 1559 | Outer ring | 8-10 rooms (random) | 40 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 35F |
| 1560 | Crossroads | 8-10 rooms (random) | 30 | 15 | yes | 3%/6% | 1% | 0 | none | 0 | MtFaraway: 36F |
| 1561 | Outer ring | 15-17 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | none | 0 | MtFaraway: 37F |
| 1562 | Crossroads | 15-17 rooms (random) | 50 | 15 | no | 3%/6% | 0% | 0 | Secondary terrain | 0 | MtFaraway: 38F |
| 1563 | Outer ring | 15-17 rooms (random) | 20 | 15 | yes | 3%/6% | 1% | 0 | none | 0 | MtFaraway: 39F |
| 1564 | Small | 8-10 rooms (random) | 20 | 15 | no | 3%/6% | 0% | 0 | none | 15 | MtFaraway: 40F |
| 1565 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 1F |
| 1566 | Large | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 2F |
| 1567 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 3F |
| 1568 | Large | 11-13 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 4F |
| 1569 | Large | 12-14 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 5F |
| 1570 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 6F |
| 1571 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 7F |
| 1572 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 8F |
| 1573 | Large | 8-10 rooms (random) | 35 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 9F |
| 1574 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 10F |
| 1575 | Large | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 11F |
| 1576 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 12F |
| 1577 | Large | 11-13 rooms (random) | 25 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 13F |
| 1578 | Large | 12-14 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 14F |
| 1579 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 15F |
| 1580 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 16F |
| 1581 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 17F |
| 1582 | Large | 8-10 rooms (random) | 35 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 18F |
| 1583 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 19F |
| 1584 | Large | 9-11 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 20F |
| 1585 | Large | 10-12 rooms (random) | 10 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 21F |
| 1586 | Large | 11-13 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 22F |
| 1587 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 23F |
| 1588 | Large | 11-13 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 24F |
| 1589 | Large | 10-12 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 25F |
| 1590 | Large | 9-11 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 26F |
| 1591 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 27F |
| 1592 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 28F |
| 1593 | Large | 10-12 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 29F |
| 1594 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 30F |
| 1595 | Large | 12-14 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 31F |
| 1596 | Large | 11-13 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 32F |
| 1597 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 33F |
| 1598 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 34F |
| 1599 | Large | 8-10 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 35F |
| 1600 | Large | 11-13 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 36F |
| 1601 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 37F |
| 1602 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 38F |
| 1603 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 39F |
| 1604 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 40F |
| 1605 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 41F |
| 1606 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 42F |
| 1607 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 43F |
| 1608 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 44F |
| 1609 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 45F |
| 1610 | Large | 12-14 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 46F |
| 1611 | Large | 11-13 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 47F |
| 1612 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 48F |
| 1613 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 49F |
| 1614 | Large | 8-10 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 50F |
| 1615 | Large | 8-10 rooms (random) | 30 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 51F |
| 1616 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 52F |
| 1617 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 53F |
| 1618 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 54F |
| 1619 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 55F |
| 1620 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 56F |
| 1621 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 57F |
| 1622 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 58F |
| 1623 | Large | 8-10 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 59F |
| 1624 | Large | 9-11 rooms (random) | 15 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 60F |
| 1625 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 61F |
| 1626 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 62F |
| 1627 | Large | 12-14 rooms (random) | 40 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 63F |
| 1628 | Large | 11-13 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 64F |
| 1629 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 65F |
| 1630 | Large | 9-11 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 66F |
| 1631 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 67F |
| 1632 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 68F |
| 1633 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 69F |
| 1634 | Large | 11-13 rooms (random) | 10 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 70F |
| 1635 | Large | 12-14 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 71F |
| 1636 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 72F |
| 1637 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 73F |
| 1638 | Large | 9-11 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 74F |
| 1639 | Large | 8-10 rooms (random) | 10 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 75F |
| 1640 | Large | 9-11 rooms (random) | 15 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 76F |
| 1641 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 77F |
| 1642 | Large | 11-13 rooms (random) | 25 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 78F |
| 1643 | Large | 12-14 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 79F |
| 1644 | Large | 11-13 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 80F |
| 1645 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 81F |
| 1646 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 82F |
| 1647 | Large | 8-10 rooms (random) | 35 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 83F |
| 1648 | Large | 8-10 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 84F |
| 1649 | Large | 9-11 rooms (random) | 40 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 85F |
| 1650 | Large | 10-12 rooms (random) | 30 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 86F |
| 1651 | Large | 11-13 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 87F |
| 1652 | Large | 12-14 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 88F |
| 1653 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 89F |
| 1654 | Large | 10-12 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | Secondary terrain | 0 | D61: 90F |
| 1655 | Large | 9-11 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 91F |
| 1656 | Large | 8-10 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 92F |
| 1657 | Large | 9-11 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 93F |
| 1658 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 94F |
| 1659 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 95F |
| 1660 | Large | 12-14 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 96F |
| 1661 | Large | 11-13 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | none | 0 | D61: 97F |
| 1662 | Large | 10-12 rooms (random) | 20 | 15 | no | 0%/0% | 0% | 0 | Secondary terrain | 0 | D61: 98F |
| 1663 | Large | 9-11 rooms (random) | 20 | 15 | yes | 0%/0% | 1% | 0 | none | 0 | D61: 99F |
| 1664 | Medium | 6-8 rooms (random) | 10 | 15 | no | 8%/0% | 0% | 0 | none | 0 | PurityForest: 1F |
| 1665 | Medium | 7-9 rooms (random) | 15 | 15 | no | 8%/0% | 0% | 0 | none | 0 | PurityForest: 2F |
| 1666 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/0% | 1% | 0 | Secondary terrain | 0 | PurityForest: 3F |
| 1667 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/0% | 0% | 0 | none | 0 | PurityForest: 4F |
| 1668 | Medium | 10-12 rooms (random) | 40 | 15 | no | 8%/0% | 0% | 0 | Secondary terrain | 0 | PurityForest: 5F |
| 1669 | Medium | 10-12 rooms (random) | 30 | 15 | yes | 8%/5% | 1% | 0 | none | 0 | PurityForest: 6F |
| 1670 | Medium | 9-11 rooms (random) | 20 | 15 | no | 50%/5% | 0% | 0 | Secondary terrain | 0 | PurityForest: 7F |
| 1671 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | PurityForest: 8F |
| 1672 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | none | 0 | PurityForest: 9F |
| 1673 | Medium | 6-8 rooms (random) | 50 | 15 | no | 8%/5% | 0% | 0 | none | 0 | PurityForest: 10F |
| 1674 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | Secondary terrain | 0 | PurityForest: 11F |
| 1675 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 8%/5% | 1% | 0 | Secondary terrain | 0 | PurityForest: 12F |
| 1676 | Small | 10-12 rooms (random) | 20 | 15 | no | 8%/5% | 0% | 0 | none | 0 | PurityForest: 13F |
| 1677 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/50% | 0% | 0 | none | 0 | PurityForest: 14F |
| 1678 | Small | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | PurityForest: 15F |
| 1679 | Small | 6-8 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 16F |
| 1680 | Small | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 17F |
| 1681 | Small | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 18F |
| 1682 | Small | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 19F |
| 1683 | Small | 10-12 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 20F |
| 1684 | Small | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 21F |
| 1685 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 22F |
| 1686 | Medium | 8-10 rooms (random) | 20 | 15 | no | 50%/6% | 0% | 0 | none | 0 | PurityForest: 23F |
| 1687 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | PurityForest: 24F |
| 1688 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 25F |
| 1689 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 26F |
| 1690 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 27F |
| 1691 | Medium | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 28F |
| 1692 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/50% | 0% | 0 | none | 0 | PurityForest: 29F |
| 1693 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | PurityForest: 30F |
| 1694 | Medium | 6-8 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 31F |
| 1695 | Medium | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 32F |
| 1696 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 33F |
| 1697 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 34F |
| 1698 | Medium | 10-12 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 35F |
| 1699 | Medium | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 36F |
| 1700 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 37F |
| 1701 | Medium | 8-10 rooms (random) | 20 | 15 | no | 50%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 38F |
| 1702 | Medium | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 39F |
| 1703 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 40F |
| 1704 | Medium | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 41F |
| 1705 | Medium | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 42F |
| 1706 | Medium | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 43F |
| 1707 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 44F |
| 1708 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | PurityForest: 45F |
| 1709 | Medium | 6-8 rooms (random) | 10 | 15 | no | 8%/50% | 0% | 0 | none | 0 | PurityForest: 46F |
| 1710 | Medium | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 47F |
| 1711 | Medium | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 48F |
| 1712 | Medium | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 49F |
| 1713 | Medium | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 50F |
| 1714 | Large | 10-12 rooms (random) | 30 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 51F |
| 1715 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 52F |
| 1716 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 53F |
| 1717 | Large | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 54F |
| 1718 | Large | 6-8 rooms (random) | 20 | 15 | no | 50%/6% | 0% | 0 | none | 0 | PurityForest: 55F |
| 1719 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 56F |
| 1720 | Large | 12-14 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | PurityForest: 57F |
| 1721 | Large | 10-12 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 58F |
| 1722 | Large | 9-11 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 59F |
| 1723 | Large | 8-10 rooms (random) | 15 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 60F |
| 1724 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 61F |
| 1725 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 62F |
| 1726 | Large | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 63F |
| 1727 | Large | 8-10 rooms (random) | 30 | 15 | no | 8%/50% | 0% | 0 | Secondary terrain | 0 | PurityForest: 64F |
| 1728 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 65F |
| 1729 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | PurityForest: 66F |
| 1730 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 67F |
| 1731 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 68F |
| 1732 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 69F |
| 1733 | Large | 6-8 rooms (random) | 10 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 70F |
| 1734 | Large | 7-9 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 71F |
| 1735 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 72F |
| 1736 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 73F |
| 1737 | Large | 10-12 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 74F |
| 1738 | Large | 10-12 rooms (random) | 30 | 15 | yes | 50%/6% | 1% | 0 | none | 0 | PurityForest: 75F |
| 1739 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 76F |
| 1740 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 77F |
| 1741 | Large | 7-9 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | Secondary terrain | 0 | PurityForest: 78F |
| 1742 | Large | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 79F |
| 1743 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 80F |
| 1744 | Large | 12-14 rooms (random) | 10 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 81F |
| 1745 | Large | 10-12 rooms (random) | 15 | 15 | no | 8%/6% | 0% | 0 | Secondary terrain | 0 | PurityForest: 82F |
| 1746 | Large | 9-11 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 83F |
| 1747 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 84F |
| 1748 | Large | 8-10 rooms (random) | 40 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 85F |
| 1749 | Large | 8-10 rooms (random) | 30 | 15 | no | 8%/50% | 0% | 0 | none | 0 | PurityForest: 86F |
| 1750 | Large | 5-7 rooms (random) | 50 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 87F |
| 1751 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 88F |
| 1752 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 89F |
| 1753 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 90F |
| 1754 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 91F |
| 1755 | Large | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 92F |
| 1756 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 93F |
| 1757 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 94F |
| 1758 | Large | 5-7 rooms (random) | 50 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 95F |
| 1759 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 0 | PurityForest: 96F |
| 1760 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 97F |
| 1761 | Large | 8-10 rooms (random) | 20 | 15 | no | 8%/6% | 0% | 0 | none | 0 | PurityForest: 98F |
| 1762 | Large | 8-10 rooms (random) | 20 | 15 | yes | 8%/6% | 1% | 0 | none | 25 | PurityForest: 99F |
| 1763 | Large | 6-8 rooms (random) | 40 | 20 | no | 0%/0% | 0% | 1 | none | 0 | D63: 1F |
