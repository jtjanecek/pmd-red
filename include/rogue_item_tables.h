// Defines roguelike item pools generated from rogue_files/items.csv.
#ifndef GUARD_ROGUE_ITEM_TABLES_H
#define GUARD_ROGUE_ITEM_TABLES_H

#include "global.h"

typedef enum {
    ROGUE_ITEM_POOL_PRIMARY_LOOT = 0,
    ROGUE_ITEM_POOL_SECONDARY_LOOT,
    ROGUE_ITEM_POOL_KECLEON_RARE,
    ROGUE_ITEM_POOL_KECLEON_COMMON,
    ROGUE_ITEM_POOL_NORMAL,
    ROGUE_ITEM_POOL_RARE,
    ROGUE_ITEM_POOL_MONSTER_HOUSE,
    ROGUE_ITEM_POOL_COUNT
} RogueItemPoolId;

typedef struct RogueItemPool {
    const u16 *items;
    u16 count;
} RogueItemPool;

extern const RogueItemPool gRogueItemPools[ROGUE_ITEM_POOL_COUNT];
extern const char *const gRogueItemPoolNames[ROGUE_ITEM_POOL_COUNT];

#endif // GUARD_ROGUE_ITEM_TABLES_H
