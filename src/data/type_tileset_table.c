#include "global.h"
#include "type_selection.h"

#include "constants/type.h"

// Generated from rogue_files/tileset_types.csv.
const TypeTilesetPool gTypeTilesetTable[NUM_TYPES] = {
    [TYPE_NONE] = {{0}, 0},
    [TYPE_NORMAL] = {{2, 23, 25, 60}, 4},
    [TYPE_FIRE] = {{22, 46, 48, 59}, 4},
    [TYPE_WATER] = {{32, 49, 54}, 3},
    [TYPE_GRASS] = {{1, 11, 20, 41, 51, 52, 53, 63}, 8},
    [TYPE_ELECTRIC] = {{29, 42, 43}, 3},
    [TYPE_ICE] = {{9, 15, 16, 18, 36, 40, 47}, 7},
    [TYPE_FIGHTING] = {{5, 8}, 2},
    [TYPE_POISON] = {{33, 57}, 2},
    [TYPE_GROUND] = {{13, 21, 31, 37, 44}, 5},
    [TYPE_FLYING] = {{6, 27, 35, 56}, 4},
    [TYPE_PSYCHIC] = {{3, 28, 58}, 3},
    [TYPE_BUG] = {{14, 26, 62}, 3},
    [TYPE_ROCK] = {{17, 38, 39, 50}, 4},
    [TYPE_GHOST] = {{7, 12}, 2},
    [TYPE_DRAGON] = {{10, 34, 45, 55}, 4},
    [TYPE_DARK] = {{4, 61}, 2},
    [TYPE_STEEL] = {{19, 24, 30}, 3},
};
