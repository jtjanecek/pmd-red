#include "global.h"
#include "globaldata.h"
#include "dungeon_cutscene.h"
#include "constants/weather.h"
#include "constants/dungeon.h"
#include "constants/direction.h"
#include "constants/dungeon.h"
#include "constants/dungeon_exit.h"
#include "structs/str_dungeon.h"
#include "structs/sprite_oam.h"
#include "dungeon_cutscenes.h"
#include "dungeon_music.h"
#include "dungeon_logic.h"
#include "exclusive_pokemon.h"
#include "weather.h"
#include "pokemon.h"
#include "dungeon_util.h"
#include "dungeon_map_access.h"
#include "dungeon_misc.h"
#include "dungeon_vram.h"
#include "code_806CD90.h"
#include "dungeon_range.h"
#include "random.h"
#include "math.h"
#include "code_8004AA0.h"
#include "bg_palette_buffer.h"
#include "items.h"
#include "effect_main.h"
#include "effect_sub_1.h"
#include "sprite.h"
#include "effect_data.h"
#include "pokemon_3.h"
#include "dungeon_boss_dialogue.h"
#include "dungeon_pos_data.h"
#include "dungeon_engine.h"
#include "dungeon_tilemap.h"
#include "dungeon_map.h"
#include "dungeon_mon_spawn.h"
#include "save.h"

struct RgbS16
{
    s16 r;
    s16 g;
    s16 b;
};

extern OpenedFile *gDungeonPaletteFile;
extern RGB gUnknown_202ECA4[];

extern bool8 sub_8004C00(unkStruct_202EE8C *a0, s32 a1, s32 a2, s32 brightness, const RGB *ramp, struct RgbS16 *a5);
extern void sub_803F878(s32, s32);
extern bool8 sub_800E90C(DungeonPos *);
extern void sub_8088EE8(void);
extern void sub_8088848(void);
extern void sub_808A718(void);
extern s32 sub_800E700(s32);
extern void sub_8052FB8(const u8 *);
extern u32 sub_8002A70(u32, s32, u8);
extern s8 sub_8002984(s8, u8);

struct unkData_8107234
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
};

// size: 0x8
typedef struct unkStruct_202F3D0
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
} unkStruct_202F3D0;

static void sub_8084854(const struct unkData_8107234 *);
static void sub_8085764(void);
static void sub_80857B8(void);
static void sub_80861EC(Entity *);
static void EnsureSoloSpacingFromBoss(void);
static void SoloNormalizeBossRoomLayout(void);
static bool8 IsBossBehavior(u8 behavior);

static const struct unkData_8107234 gUnknown_8107234[28] = {
    [0] = {1, 1, 0, 2, 1, 3},
    [1] = {2, 4, 2, 5, 3, 6},
    [2] = {3, 7, 4, 8, 5, 9},
    [3] = {4, 0xA, 6, 0xB, 7, 0xC},
    [4] = {5, 0xD, 8, 0xE, 9, 0xF},
    [5] = {6, 0x10, 0xA, 0x11, 0xA, 0x11},
    [6] = {7, 0x12, 0xB, 0x13, 0xC, 0x14},
    [7] = {0x14, 0x15, 0xD, 0x16, 0xD, 0x16},
    [8] = {8, 0x17, 0xE, 0x18, 0xF, 0x19},
    [9] = {0xB, 0x1A, 0x10, 0x1B, 0x11, 0x1C},
    [10] = {9, 0x1D, 0x12, 0x1E, 0x13, 0x1F},
    [11] = {0xC, 0x20, 0x14, 0x21, 0x15, 0x22},
    [12] = {0xD, 0x23, 0x16, 0x24, 0x17, 0x25},
    [13] = {0xE, 0x26, 0x18, 0x27, 0x19, 0x28},
    [14] = {0xF, 0x29, 0x40, 0x29, 0x1A, 0x2A},
    [15] = {0x10, 0x2B, 0x1B, 0x2C, 0x1C, 0x2D},
    [16] = {0x11, 0x2E, 0x40, 0x2E, 0x40, 0x2E},
    [17] = {0x12, 0x2F, 0x40, 0x2F, 0x40, 0x2F},
    [18] = {0x13, 0x30, 0x40, 0x30, 0x40, 0x30},
    [19] = {0xA, 0x31, 0x40, 0x31, 0x1E, 0x32},
    [20] = {0x15, 0x33, 0x40, 0x33, 0x40, 0x33},
    [21] = {0x16, 0x34, 0x40, 0x34, 0x40, 0x34},
    [22] = {0x17, 0x35, 0x40, 0x35, 0x40, 0x35},
    [23] = {0x19, 0x36, 0x40, 0x36, 0x40, 0x36},
    [24] = {0x1B, 0x37, 0x40, 0x37, 0x40, 0x37},
    [25] = {0x1A, 0x38, 0x20, 0x39, 0x20, 0x39},
    [26] = {0x18, 0x3A, 0x21, 0x3B, 0x21, 0x3B},
    [27] = {0},
};

static const s32 gUnknown_8107314[] = {
    0, -1, 1, -2, 2, -3, 3, -4,
    4, -4, 4, -4, 4, -4, 4, -4,
    4
};

EWRAM_DATA static unkStruct_202F3D0 gUnknown_202F3D0 = {0};

// Return TRUE if this behavior represents a boss we want distance from in Solo
static bool8 IsBossBehavior(u8 behavior)
{
    return (
        behavior == BEHAVIOR_SKARMORY ||
        behavior == BEHAVIOR_GENGAR   ||
        behavior == BEHAVIOR_EKANS    ||
        behavior == BEHAVIOR_MEDICHAM
    );
}

// Ensure the leader is at least minDist tiles away (Chebyshev) from any boss entity.
static void EnsureSoloSpacingFromBoss(void)
{
    Entity *leader;
    Entity *bosses[8];
    s32 bossCount;
    s32 i;
    s32 minDist;
    bool8 closeToAny;
    u8 bossRoom;
    s16 leaderSpecies;
    s32 j;

    leader = GetLeader();
    if (!EntityIsValid(leader)) return;

    // Gather boss entities present on the floor
    bossCount = 0;
    for (i = 0; i < DUNGEON_MAX_POKEMON && bossCount < 8; i++) {
        Entity *ent = gDungeon->activePokemon[i];
        if (EntityIsValid(ent)) {
            EntityInfo *info = GetEntInfo(ent);
            if (info->isNotTeamMember && IsBossBehavior(info->monsterBehavior)) {
                bosses[bossCount++] = ent;
            }
        }
    }
    if (bossCount == 0) return; // Nothing to do

    // Target placement: 3 tiles BELOW the lowest boss (max Y), centered between bosses.
    // This ensures all bosses are above the player.
    minDist = 3;
    closeToAny = TRUE; // always reposition for consistency

    {
        s32 minX, maxX, minY, maxY;
        s32 centerX;
        minX = maxX = bosses[0]->pos.x;
        minY = maxY = bosses[0]->pos.y;
        for (i = 1; i < bossCount; i++) {
            if (bosses[i]->pos.x < minX) minX = bosses[i]->pos.x;
            if (bosses[i]->pos.x > maxX) maxX = bosses[i]->pos.x;
            if (bosses[i]->pos.y < minY) minY = bosses[i]->pos.y;
            if (bosses[i]->pos.y > maxY) maxY = bosses[i]->pos.y;
        }
        centerX = (minX + maxX) / 2;

        bossRoom = GetTile(bosses[0]->pos.x, bosses[0]->pos.y)->room;
        leaderSpecies = GetEntInfo(leader)->id;

        // Try exact (centerX, baseYStart) first; if blocked, expand horizontally,
        // then step further down within the room bounds.
        {
            static const s8 dxPref[] = {0, -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6, -7, 7, -8, 8, -9, 9, -10, 10};
            s32 baseX = centerX;
            s32 baseYStart, baseYEnd;
            s32 roomMinY = DUNGEON_MAX_SIZE_Y;
            s32 roomMaxY = -1;
            s32 rx, ry;
            for (ry = 0; ry < DUNGEON_MAX_SIZE_Y; ry++) {
                for (rx = 0; rx < DUNGEON_MAX_SIZE_X; rx++) {
                    const Tile *rtile = GetTile(rx, ry);
                    if (rtile->room == bossRoom) {
                        if (ry < roomMinY) roomMinY = ry;
                        if (ry > roomMaxY) roomMaxY = ry;
                    }
                }
            }
            baseYStart = maxY + 3;
            if (baseYStart > roomMaxY) baseYStart = roomMaxY;
            if (baseYStart < roomMinY) baseYStart = roomMinY;
            baseYEnd = baseYStart + 5;
            if (baseYEnd > roomMaxY) baseYEnd = roomMaxY;
            for (; baseYStart <= baseYEnd; baseYStart++) {
                for (i = 0; i < (s32)(sizeof(dxPref)/sizeof(dxPref[0])); i++) {
                    s32 nx = baseX + dxPref[i];
                    s32 ny = baseYStart;
                    const Tile *tile = GetTile(nx, ny);
                    if (tile->room != bossRoom) continue;
                    if (sub_807034C(leaderSpecies, tile) != 0) continue;

                    sub_80856C8(leader, nx, ny);
                    sub_8085860(nx, ny);
                    return;
                }
            }
        }
    }

    // Fallback: find any valid tile at least 3 tiles away using the offset pattern.
    bossRoom = GetTile(bosses[0]->pos.x, bosses[0]->pos.y)->room;
    leaderSpecies = GetEntInfo(leader)->id;
    for (j = 0; ; j++) {
        DungeonPos off = gUnknown_80F4598[j];
        s32 dx, dy;
        s32 nx, ny;
        const Tile *tile;
        bool8 farFromAll;
        if (off.x == 99) break;

        dx = off.x; if (dx < 0) dx = -dx;
        dy = off.y; if (dy < 0) dy = -dy;
        if ((dx > dy ? dx : dy) < minDist) continue;

        nx = bosses[0]->pos.x + off.x;
        ny = bosses[0]->pos.y + off.y;
        tile = GetTile(nx, ny);
        if (tile->room != bossRoom) continue;
        if (sub_807034C(leaderSpecies, tile) != 0) continue;

        farFromAll = TRUE;
        for (i = 0; i < bossCount; i++) {
            s32 ddx, ddy;
            ddx = nx - bosses[i]->pos.x; if (ddx < 0) ddx = -ddx;
            ddy = ny - bosses[i]->pos.y; if (ddy < 0) ddy = -ddy;
            if ((ddx > ddy ? ddx : ddy) < minDist) { farFromAll = FALSE; break; }
        }
        if (!farFromAll) continue;

        sub_80856C8(leader, nx, ny);
        sub_8085860(nx, ny);
        return;
    }
}

// For PlaySolo: move all enemies in the boss room to one row from the top of that room,
// then place the leader three rows below that. Keeps fights readable and consistent.
static void SoloNormalizeBossRoomLayout(void)
{
    Entity *leader;
    Entity *bosses[8];
    s32 bossCount;
    s32 i;
    u8 bossRoom;
    s16 leaderSpecies;
    s32 roomMinY = DUNGEON_MAX_SIZE_Y;
    s32 roomMaxY = -1;
    s32 roomMinX = DUNGEON_MAX_SIZE_X;
    s32 roomMaxX = -1;

    leader = GetLeader();
    if (!EntityIsValid(leader)) return;

    // Identify boss room via any boss-behavior enemy present
    bossCount = 0;
    for (i = 0; i < DUNGEON_MAX_POKEMON && bossCount < 8; i++) {
        Entity *ent = gDungeon->activePokemon[i];
        if (EntityIsValid(ent)) {
            EntityInfo *info = GetEntInfo(ent);
            if (info->isNotTeamMember && IsBossBehavior(info->monsterBehavior)) {
                bosses[bossCount++] = ent;
            }
        }
    }
    if (bossCount == 0) return; // no boss; nothing to normalize

    bossRoom = GetTile(bosses[0]->pos.x, bosses[0]->pos.y)->room;

    // Compute room bounds
    {
        s32 rx, ry;
        for (ry = 0; ry < DUNGEON_MAX_SIZE_Y; ry++) {
            for (rx = 0; rx < DUNGEON_MAX_SIZE_X; rx++) {
                const Tile *t = GetTile(rx, ry);
                if (t->room == bossRoom) {
                    if (ry < roomMinY) roomMinY = ry;
                    if (ry > roomMaxY) roomMaxY = ry;
                    if (rx < roomMinX) roomMinX = rx;
                    if (rx > roomMaxX) roomMaxX = rx;
                }
            }
        }
        if (roomMaxY < roomMinY) return; // invalid room
    }

    // Desired rows
    {
        s32 enemyRow = roomMinY + 1; // one row from the top
        s32 leaderRow = enemyRow + 3; // three rows below
        s32 centerX;
        if (leaderRow > roomMaxY) leaderRow = roomMaxY; // clamp
        if (leaderRow <= enemyRow && enemyRow < roomMaxY) leaderRow = enemyRow + 1;

        leaderSpecies = GetEntInfo(leader)->id;
        // Center X as midpoint of the room horizontally for leader placement
        centerX = (roomMinX + roomMaxX) / 2;

        // Special case: single-boss fights (e.g., Skarmory).
        // Do not relocate the boss; instead anchor the leader 3 rows below the boss
        // (or 1 below if room is shallow). Avoid the same column as the boss.
        if (bossCount == 1) {
            s32 targetY = bosses[0]->pos.y + 3;
            s32 baseX = bosses[0]->pos.x;
            static const s8 dxPrefSingle[] = {0, -1, 1, -2, 2, -3, 3, -4, 4};
            s32 k;

            if (targetY > roomMaxY) targetY = bosses[0]->pos.y + 1;
            if (targetY > roomMaxY) targetY = roomMaxY;
            if (targetY <= bosses[0]->pos.y && bosses[0]->pos.y > roomMinY) {
                // If we still couldn't get below, place one row above as a last resort
                targetY = bosses[0]->pos.y - 1;
            }

            for (k = 0; k < (s32)(sizeof(dxPrefSingle)/sizeof(dxPrefSingle[0])); k++) {
                s32 nx = baseX + dxPrefSingle[k];
                s32 ny = targetY;
                const Tile *tile = GetTile(nx, ny);
                if (tile->room != bossRoom) continue;
                if (sub_807034C(leaderSpecies, tile) != 0) continue;
                if (tile->monster != NULL) continue;
                if (nx == bosses[0]->pos.x && ny == bosses[0]->pos.y) continue; // not on boss
                sub_80856C8(leader, nx, ny);
                sub_8085860(nx, ny);
                return;
            }
            // If we couldn't place relative to boss cleanly, fall back to center-based logic below
        }

        // If this is the Team Meanies fight (multiple bosses), place them in a tight
        // formation on enemyRow around the room center: Medicham (left), Gengar (center), Ekans (right).
        // Otherwise, gently normalize all enemies to enemyRow.
        {
            Entity *gengar = NULL, *ekans = NULL, *medicham = NULL;
            bool8 hasMeanies = FALSE;
            for (i = 0; i < DUNGEON_MAX_POKEMON; i++) {
                Entity *ent = gDungeon->activePokemon[i];
                if (EntityIsValid(ent)) {
                    EntityInfo *info = GetEntInfo(ent);
                    const Tile *currTile = GetTile(ent->pos.x, ent->pos.y);
                    if (info->isNotTeamMember && currTile->room == bossRoom) {
                        if (info->monsterBehavior == BEHAVIOR_GENGAR) gengar = ent;
                        else if (info->monsterBehavior == BEHAVIOR_EKANS) ekans = ent;
                        else if (info->monsterBehavior == BEHAVIOR_MEDICHAM) medicham = ent;
                    }
                }
            }
            if (gengar != NULL && (ekans != NULL || medicham != NULL)) {
                hasMeanies = TRUE;
            }

            if (hasMeanies) {
                // Desired Xs
                s32 gx = centerX;           // Gengar center
                s32 mx = centerX - 2;       // Medicham left
                s32 ex = centerX + 2;       // Ekans right
                static const s8 wiggle[] = {0, -1, 1, -2, 2, -3, 3};
                s32 k;

                // Place Gengar first at center
                if (gengar != NULL) {
                    for (k = 0; k < (s32)(sizeof(wiggle)/sizeof(wiggle[0])); k++) {
                        s32 nx = gx + wiggle[k];
                        const Tile *tile = GetTile(nx, enemyRow);
                        if (tile->room != bossRoom) continue;
                        if (sub_807034C(GetEntInfo(gengar)->id, tile) != 0) continue;
                        if (tile->monster != NULL) continue;
                        sub_80856C8(gengar, nx, enemyRow);
                        break;
                    }
                }

                // Place Medicham on left side
                if (medicham != NULL) {
                    for (k = 0; k < (s32)(sizeof(wiggle)/sizeof(wiggle[0])); k++) {
                        s32 nx = mx + wiggle[k];
                        const Tile *tile = GetTile(nx, enemyRow);
                        if (tile->room != bossRoom) continue;
                        if (sub_807034C(GetEntInfo(medicham)->id, tile) != 0) continue;
                        if (tile->monster != NULL) continue;
                        sub_80856C8(medicham, nx, enemyRow);
                        break;
                    }
                }

                // Place Ekans on right side
                if (ekans != NULL) {
                    for (k = 0; k < (s32)(sizeof(wiggle)/sizeof(wiggle[0])); k++) {
                        s32 nx = ex + wiggle[k];
                        const Tile *tile = GetTile(nx, enemyRow);
                        if (tile->room != bossRoom) continue;
                        if (sub_807034C(GetEntInfo(ekans)->id, tile) != 0) continue;
                        if (tile->monster != NULL) continue;
                        sub_80856C8(ekans, nx, enemyRow);
                        break;
                    }
                }

                // Any other enemies in the boss room: gently slide to enemyRow
                for (i = 0; i < DUNGEON_MAX_POKEMON; i++) {
                    Entity *ent = gDungeon->activePokemon[i];
                    if (EntityIsValid(ent)) {
                        EntityInfo *info = GetEntInfo(ent);
                        const Tile *currTile = GetTile(ent->pos.x, ent->pos.y);
                        if (info->isNotTeamMember && currTile->room == bossRoom && ent != gengar && ent != ekans && ent != medicham) {
                            s32 baseX = ent->pos.x;
                            s32 k2;
                            for (k2 = 0; k2 < (s32)(sizeof(wiggle)/sizeof(wiggle[0])); k2++) {
                                s32 nx = baseX + wiggle[k2];
                                const Tile *tile = GetTile(nx, enemyRow);
                                if (tile->room != bossRoom) continue;
                                if (sub_807034C(info->id, tile) != 0) continue;
                                if (tile->monster != NULL) continue;
                                sub_80856C8(ent, nx, enemyRow);
                                break;
                            }
                        }
                    }
                }
            }
            else {
                // Generic normalization for non-Meanies rooms
                static const s8 dxPref[] = {0, -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6};
                for (i = 0; i < DUNGEON_MAX_POKEMON; i++) {
                    Entity *ent = gDungeon->activePokemon[i];
                    if (EntityIsValid(ent)) {
                        EntityInfo *info = GetEntInfo(ent);
                        const Tile *currTile = GetTile(ent->pos.x, ent->pos.y);
                        if (info->isNotTeamMember && currTile->room == bossRoom) {
                            s32 baseX = ent->pos.x;
                            s32 k;
                            for (k = 0; k < (s32)(sizeof(dxPref)/sizeof(dxPref[0])); k++) {
                                s32 nx = baseX + dxPref[k];
                                const Tile *tile = GetTile(nx, enemyRow);
                                if (tile->room != bossRoom) continue;
                                if (sub_807034C(info->id, tile) != 0) continue;
                                if (tile->monster != NULL) continue;
                                sub_80856C8(ent, nx, enemyRow);
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Place the leader on leaderRow near center; for Team Meanies, anchor to Gengar's X
        // and allow same column so the formation is perfectly vertical.
        {
            static const s8 dxPref2[] = {0, -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6, -7, 7};
            s32 k;
            s32 anchorX = centerX;
            bool8 anchorIsGengar = FALSE;
            // Try to use Gengar's X as anchor if present in this room
            for (i = 0; i < DUNGEON_MAX_POKEMON; i++) {
                Entity *e = gDungeon->activePokemon[i];
                if (EntityIsValid(e)) {
                    EntityInfo *ei = GetEntInfo(e);
                    if (ei->isNotTeamMember && ei->monsterBehavior == BEHAVIOR_GENGAR && GetTile(e->pos.x, e->pos.y)->room == bossRoom) {
                        anchorX = e->pos.x;
                        anchorIsGengar = TRUE;
                        break;
                    }
                }
            }
            for (k = 0; k < (s32)(sizeof(dxPref2)/sizeof(dxPref2[0])); k++) {
                s32 nx = anchorX + dxPref2[k];
                s32 ny = leaderRow;
                const Tile *tile = GetTile(nx, ny);
                if (tile->room != bossRoom) continue;
                if (sub_807034C(leaderSpecies, tile) != 0) continue;
                if (tile->monster != NULL) continue; // avoid landing on an enemy
                if (!anchorIsGengar) {
                    // Avoid same column as any enemy only when not anchoring to Gengar
                    bool8 sameColumn = FALSE;
                    for (i = 0; i < DUNGEON_MAX_POKEMON; i++) {
                        Entity *e2 = gDungeon->activePokemon[i];
                        if (EntityIsValid(e2)) {
                            EntityInfo *ei2 = GetEntInfo(e2);
                            if (ei2->isNotTeamMember && GetTile(e2->pos.x, e2->pos.y)->room == bossRoom) {
                                if (e2->pos.x == nx) { sameColumn = TRUE; break; }
                            }
                        }
                    }
                    if (sameColumn) continue;
                }
                sub_80856C8(leader, nx, ny);
                sub_8085860(nx, ny);
                return;
            }
        }
    }
    // Fallback to spacing-only logic if we couldn't find a good spot
    EnsureSoloSpacingFromBoss();
}

void sub_80847D4(void)
{
    u32 fixedRoomNumber;
    s32 index;

    gDungeon->unk3A0D = 0;
    gDungeon->unk1356C = 0;
    UpdateMinimap();
    for(index = 0; index < 0x3e7 && gUnknown_8107234[index].unk0 != 0;  index++) {
        fixedRoomNumber = gDungeon->fixedRoomNumber;
        if (fixedRoomNumber - 0x1c < 0x16) {
            fixedRoomNumber = 0x1b;
        }
        if (fixedRoomNumber == gUnknown_8107234[index].unk0)
        {
            sub_8084854(&gUnknown_8107234[index]);
            break;
        }
    }

    // In PlaySolo mode, we skip running the pre-fight setup + dialogues,
    // but we keep gDungeon->unk3A0D intact so post-battle triggers work.
    sub_8097FF8();
}

static void sub_8084854(const struct unkData_8107234 *param_1)
{
    if (gDungeon->unk644.unk34 != 0) {
        gDungeon->unk3A0D = param_1->unk5;
    }
    else if (sub_8098100(param_1->unk4) != 0) {
        gDungeon->unk3A0D = param_1->unk5;
    }
    else if (sub_8098100(param_1->unk2) != 0) {
        gDungeon->unk3A0D = param_1->unk3;
    }
    else {
        gDungeon->unk3A0D = param_1->unk1;
        if (param_1->unk2 != 0x40) {
            sub_8097FA8(param_1->unk2);
        }
    }

    gDungeon->unk644.unk31 = 1;
    sub_807E5E4(WEATHER_CLEAR);
}

bool8 ShouldShowDungeonBanner(void)
{
    return TRUE;
}

void sub_80848F0(void)
{
  if (GetPlaySoloSetting()) {
    // Normalize boss room layout (enemies to top row+1; leader 3 rows below)
    SoloNormalizeBossRoomLayout();
    // Skip pre-fight setup in PlaySolo (keep unk3A0D for victory triggers)
    gDungeon->unk1356C = 0;
    return;
  }
  gDungeon->unk1356C = 1;
  switch(gDungeon->unk3A0D) {
    case 0:
    case 0x3c:
        gDungeon->unk1356C = 0;
        break;
    case 1:
    case 2:
        sub_8086B14();
        break;
    case 3:
        sub_8086B94();
        break;
    case 4:
    case 5:
        sub_8086E9C();
        break;
    case 6:
        sub_8086F00();
        break;
    case 7:
    case 8:
        sub_8087230();
        break;
    case 9:
        sub_808729C();
        break;
    case 10:
    case 0xb:
        sub_80877E8();
        break;
    case 0xc:
        sub_8087848();
        break;
    case 0xd:
        sub_8087F54();
        break;
    case 0xe:
        sub_8087FB4();
        break;
    case 0xf:
        sub_8087FF8();
        break;
    case 0x10:
        sub_80885A0();
        break;
    case 0x11:
        sub_80885C4();
        break;
    case 0x12:
        sub_808862C();
        break;
    case 0x13:
        sub_80886C4();
        break;
    case 0x14:
        sub_808875C();
        break;
    case 0x15:
        sub_8088DC0();
        break;
    case 0x16:
        sub_8088E5C();
        break;
    case 0x17:
    case 0x18:
        sub_80892C8();
        break;
    case 0x19:
        sub_8089328();
        break;
    case 0x1a:
    case 0x1b:
        sub_808970C();
        break;
    case 0x1c:
        sub_808974C();
        break;
    case 0x1d:
    case 0x1e:
        sub_8089914();
        break;
    case 0x1f:
        sub_8089978();
        break;
    case 0x20:
    case 0x21:
        sub_8089C44();
        break;
    case 0x22:
        sub_8089C90();
        break;
    case 0x23:
        sub_8089EFC();
        break;
    case 0x24:
        sub_8089F44();
        break;
    case 0x25:
        sub_8089F8C();
        break;
    case 0x26:
        sub_808A264();
        break;
    case 0x27:
        sub_808A2C0();
        break;
    case 0x28:
        sub_808A308();
        break;
    case 0x29:
        sub_808A608();
        break;
    case 0x2a:
        sub_808A668();
        break;
    case 0x2b:
        sub_808A9E4();
        break;
    case 0x2c:
        sub_808AA3C();
        break;
    case 0x2d:
        sub_808AA94();
        break;
    case 0x2e:
        sub_808ACC0();
        break;
    case 0x2f:
        sub_808AD48();
        break;
    case 0x30:
        sub_808ADCC();
        break;
    case 0x31:
        sub_808B2F4();
        break;
    case 0x32:
        sub_808B35C();
        break;
    case 0x33:
        sub_808BDEC();
        break;
    case 0x34:
        sub_808C10C();
        break;
    case 0x35:
        sub_808C3A0();
        break;
    case 0x36:
        sub_808C5C0();
        break;
    case 0x37:
        sub_808C938();
        break;
    case 0x38:
        sub_808C9C4();
        break;
    case 0x39:
        sub_808CB5C();
        break;
    case 0x3a:
        sub_808CBB0();
        break;
    case 0x3b:
        sub_808CD44();
        break;
  }
  sub_8097FF8();
}

void DisplayPreFightDialogue(void)
{
  if (GetPlaySoloSetting()) {
    // Skip dialogue in PlaySolo; triggers still work based on unk3A0D
    return;
  }
  switch(gDungeon->unk3A0D) {
      case 0:
        break;
      case 1:
        SkarmoryPreFightDialogue();
        break;
      case 2:
        SkarmoryReFightDialogue();
        break;
      case 3:
        sub_8086E40();
        break;
      case 4:
        TeamMeaniesPreFightDialogue();
        break;
      case 5:
        TeamMeaniesReFightDialogue();
        break;
      case 6:
        sub_8087130();
        break;
      case 7:
        ZapdosPreFightDialogue();
        break;
      case 8:
        ZapdosReFightDialogue();
        break;
      case 9:
        ZapdosPostStoryPreFightDialogue();
        break;
      case 10:
        MoltresPreFightDialogue();
        break;
      case 0xb:
        MoltresReFightDialogue();
        break;
      case 0xc:
        MoltresPostStoryPreFightDialogue();
        break;
      case 0xd:
        ArticunoPreFightDialogue();
        break;
      case 0xe:
        ArticunoReFightDialogue();
        break;
      case 0xf:
        ArticunoPostStoryPreFightDialogue();
        break;
      case 0x10:
        sub_8088608();
        break;
      case 0x11:
        sub_8088618();
        break;
      case 0x12:
        GroudonPreFightDialogue();
        break;
      case 0x13:
        GroudonReFightDialogue();
        break;
      case 0x14:
        GroudonPostStoryPreFightDialogue();
        break;
      case 0x15:
        MagmaCavernMidDialogue();
        break;
      case 0x16:
        sub_8089168();
        break;
      case 0x17:
        RayquazaPreFightDialogue();
        break;
      case 0x18:
        RayquazaReFightDialogue();
        break;
      case 0x19:
        RayquazaPostStoryPreFightDialogue();
        break;
      case 0x1a:
        sub_80897F0();
        break;
      case 0x1b:
        sub_8089878();
        break;
      case 0x1c:
        sub_80898E4();
        break;
      case 0x1d:
      case 0x1e:
        MewtwoPreFightDialogue();
        break;
      case 0x1f:
        MewtwoReFightDialogue();
        break;
      case 0x20:
        EnteiPreFightDialogue();
        break;
      case 0x21:
        EnteiReFightDialogue();
        break;
      case 0x22:
        EnteiPostStoryPreFightDialogue();
        break;
      case 0x23:
        RaikouPreFightDialogue();
        break;
      case 0x24:
        RaikouReFightDialogue();
        break;
      case 0x25:
        RaikouPostStoryPreFightDialogue();
        break;
      case 0x26:
        SuicunePreFightDialogue();
        break;
      case 0x27:
        SuicuneReFightDialogue();
        break;
      case 0x28:
        SuicunePostStoryPreFightDialogue();
        break;
      case 0x29:
        HoOhPreFightDialogue();
        break;
      case 0x2a:
        HoOhReFightDialogue();
        break;
      case 0x2b:
        LatiosPreFightDialogue();
        break;
      case 0x2c:
        LatiosReFightDialogue();
        break;
      case 0x2d:
        sub_808AC3C();
        break;
      case 0x2e:
        RegirockPreFightDialogue();
        break;
      case 0x2f:
        RegicePreFightDialogue();
        break;
      case 0x30:
        RegisteelPreFightDialogue();
        break;
      case 0x31:
        JirachiPreFightDialogue();
        break;
      case 0x32:
        JirachiReFightDialogue();
        break;
      case 0x33:
        LugiaPreFightDialogue();
        break;
      case 0x34:
        KyogrePreFightDialogue();
        break;
      case 0x35:
        DeoxysPreFightDialogue();
        break;
      case 0x36:
        CelebiJoinDialogue();
        break;
      case 0x37:
        sub_808C998();
        break;
      case 0x38:
        MedichamRescueDialogue();
        break;
      case 0x39:
        DummyFightDialogue();
        break;
      case 0x3a:
        SmeargleRescueDialogue();
        break;
      case 0x3b:
        sub_808CD9C();
        break;
      case 0x3C:
        break;
  }
  sub_8085764();
  sub_80857B8();
  gDungeon->unk1356C = 0;
  sub_8097FF8();
}

void sub_8084E00(Entity *entity, u8 param_2, bool8 param_3)
{
  if (param_2 != 0) {
    switch(gDungeon->unk3A0D) {
        case 0:
            break;
        case 1:
        case 2:
            sub_8086BDC(param_2,gDungeon->unk3A0D);
            break;
        case 4:
        case 5:
            sub_8086F54(param_2,gDungeon->unk3A0D);
            break;
        case 7:
        case 8:
        case 9:
            sub_8087334(param_2,gDungeon->unk3A0D);
            break;
        case 10:
        case 0xb:
        case 0xc:
            sub_80878F4(param_2,gDungeon->unk3A0D);
            break;
        case 0xd:
        case 0xe:
        case 0xf:
            sub_8088088(param_2,gDungeon->unk3A0D);
            break;
        case 0x12:
        case 0x13:
        case 0x14:
            sub_8088818(param_2,gDungeon->unk3A0D);
            break;
        case 0x17:
        case 0x18:
        case 0x19:
            sub_80893B4(param_2,gDungeon->unk3A0D);
            break;
        case 0x1a:
        case 0x1b:
            sub_8089788(entity,param_2,gDungeon->unk3A0D);
            break;
        case 0x1d:
        case 0x1e:
        case 0x1f:
            sub_8089A00(param_2,gDungeon->unk3A0D);
            break;
        case 0x20:
        case 0x21:
        case 0x22:
            sub_8089CFC(param_2,gDungeon->unk3A0D);
            break;
        case 0x23:
        case 0x24:
        case 0x25:
            sub_8089FF0(param_2,gDungeon->unk3A0D);
            break;
        case 0x26:
        case 0x27:
        case 0x28:
            sub_808A36C(param_2,gDungeon->unk3A0D);
            break;
        case 0x29:
        case 0x2a:
            sub_808A6E8(param_2,gDungeon->unk3A0D);
            break;
        case 0x2b:
        case 0x2c:
        case 0x2d:
            sub_808AAF0(param_2,gDungeon->unk3A0D);
            break;
        case 0x2e:
            sub_808AE54(param_2,gDungeon->unk3A0D,&entity->pos);
            break;
        case 0x2f:
            sub_808AEC8(param_2,gDungeon->unk3A0D,&entity->pos);
            break;
        case 0x30:
            sub_808AF3C(param_2,gDungeon->unk3A0D,&entity->pos);
            break;
        case 0x31:
        case 0x32:
            sub_808B3E4(param_2,gDungeon->unk3A0D,param_3);
            break;
        case 0x33:
            sub_808BE70(param_2,gDungeon->unk3A0D,param_3);
            break;
        case 0x34:
            sub_808C1A4(param_2,gDungeon->unk3A0D,param_3);
            break;
        case 0x35:
            sub_808C414(param_2,gDungeon->unk3A0D,param_3);
            break;
        case 0x36:
            nullsub_100(param_2,gDungeon->unk3A0D,param_3);
            break;
        case 0x37:
            sub_808C948(entity,gDungeon->unk3A0D);
            break;
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
            break;
    }
    if ((param_3 == 0) && (gDungeon->unk2 != 0)) {
      DungeonStartNewBGM(999);
    }
  }
}

void sub_8085140(void)
{
    switch(gDungeon->unk3A0D) {
        // NOTE: shortcut way to generate all cases from 0 - 0x3C properly
        default:
        case 0:
        case 1:
        case 0x3C:
            break;
        case 0x2e:
            sub_808AFB0(gDungeon->unk3A0D);
            break;
        case 0x2f:
            sub_808B030(gDungeon->unk3A0D);
            break;
        case 0x30:
            sub_808B0B0(gDungeon->unk3A0D);
            break;
  }
}

bool8 sub_808529C(s32 speciesId_)
{
    s32 r3;
    s32 speciesId = 0;

    // This line serves no purpose other than generating the right asm. Feel free to remove it.
    speciesId += (s16) speciesId_;

    speciesId = (s16) speciesId_;
    r3 = 0x40;

    switch (speciesId) {
        case MONSTER_SKARMORY:
            r3 = 1;
            break;
        case MONSTER_EKANS:
            r3 = 3;
            break;
        case MONSTER_GENGAR:
            r3 = 3;
            break;
        case MONSTER_MEDICHAM:
            r3 = 3;
            break;
        case MONSTER_ZAPDOS:
            r3 = 5;
            break;
        case MONSTER_MOLTRES:
            r3 = 7;
            break;
        case MONSTER_ARTICUNO:
            r3 = 9;
            break;
        case MONSTER_GROUDON:
            r3 = 0xC;
            break;
        case MONSTER_RAYQUAZA:
            r3 = 0xF;
            break;
        case MONSTER_MEWTWO:
            r3 = 0x13;
            break;
        case MONSTER_ENTEI:
            r3 = 0x15;
            break;
        case MONSTER_RAIKOU:
            r3 = 0x17;
            break;
        case MONSTER_SUICUNE:
            r3 = 0x19;
            break;
        case MONSTER_HO_OH:
            r3 = 0x1A;
            break;
        case MONSTER_MANKEY:
            r3 = 0x11;
            break;
        case MONSTER_REGIROCK:
            r3 = 0x1D;
            break;
        case MONSTER_REGICE:
            r3 = 0x1D;
            break;
        case MONSTER_REGISTEEL:
            r3 = 0x1D;
            break;
    }

    if (r3 != 0x40 && !sub_8098100(r3))
        return FALSE;
    return TRUE;
}

void sub_8085374(void)
{
    const Tile *tile;
    Entity *entity;
    DungeonPos pos;
    s32 i, j;
    s32 temp;
    Entity *leaderEntity = NULL;
    Entity *partnerEntity = NULL;
    Pokemon *partnerMon = NULL;
    s32 species = -1;

    for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
        entity = gDungeon->teamPokemon[i];
        if (EntityIsValid(entity)) {
            if (GetEntInfo(entity)->isTeamLeader)
                leaderEntity = entity;
            if (GetEntInfo(entity)->joinedAt.id == DUNGEON_JOIN_LOCATION_PARTNER)
                partnerEntity = entity;
        }
    }

    if (leaderEntity == NULL || partnerEntity != NULL || gDungeon->unk644.unk18 != 0)
        return;

    // Determine species to spawn for partner ally.
    // Prefer recruited partner species; fall back to leader's species if none.
    partnerMon = sub_808D3F8();
    if (partnerMon != NULL) {
        species = partnerMon->speciesNum;
    } else {
        species = GetEntInfo(leaderEntity)->id;
    }

    j = 0;
    while ((pos = gUnknown_80F4598[j]).x != 99) {
        pos.x += leaderEntity->pos.x;
        pos.y += leaderEntity->pos.y;
        tile = GetTile(pos.x, pos.y);
        temp = sub_807034C(species, tile);
        if (temp == 0) {
            struct MonSpawnInfo local_30;

            local_30.species = species;
            local_30.level = 1;
            local_30.unk2 = 2;
            local_30.pos = pos;
            local_30.unk4 = temp;
            local_30.unk10 = 0;
            SpawnWildMon(&local_30, 1);
            break;
        }
        j++;
    }
}

Entity *CutsceneGetLeader(void)
{
    return GetLeader();
}

Entity *CutsceneGetPartner(void)
{
    s32 counter;
    Entity *entity;
    for (counter = 0; counter < MAX_TEAM_MEMBERS; counter++)
    {
        entity = gDungeon->teamPokemon[counter];
        if (EntityIsValid(entity) && GetEntInfo(entity)->joinedAt.id == DUNGEON_JOIN_LOCATION_PARTNER)
        {
            return entity;
        }
    }

    // Fallback: if no partner in team and no existing ally entity, try spawning
    // a temporary partner for cutscenes (e.g., when PlaySolo is enabled).
    {
        Entity *ally = GetEntityFromMonsterBehavior(BEHAVIOR_ALLY);
        if (ally != NULL)
            return ally;

        // Attempt to spawn a temporary partner near the leader before cutscenes use it.
        // sub_8085374 spawns a BEHAVIOR_ALLY entity with the partner's species
        // if leader exists and no partner is present.
        sub_8085374();

        ally = GetEntityFromMonsterBehavior(BEHAVIOR_ALLY);
        if (ally != NULL)
            return ally;

        // Absolute fallback: return leader to avoid NULL dereferences in cutscene scripts.
        return GetLeader();
    }
}

void sub_80854D4(void)
{
    Entity *stack1[MAX_TEAM_MEMBERS];
    Entity *stack2[MAX_TEAM_MEMBERS];
    s32 counter = 0;
    s32 index;
    Entity *entity;
    Entity *entity2;
    Entity *entity3;
    Entity *entity4;
    Entity *entity5;

    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity = gDungeon->teamPokemon[index];
        if(EntityIsValid(entity))
        {
            stack1[counter] = entity;
            counter++;
        }
    }
    for(; counter < MAX_TEAM_MEMBERS; counter++)
    {
        stack1[counter] = NULL;
    }
    counter = 0;
    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity2 = stack1[index];
        if(entity2 != NULL && GetEntInfo(entity2)->isTeamLeader)
        {
            stack1[index] = NULL;
            stack2[counter] = entity2;
            counter++;
        }
    }
    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity3 = stack1[index];
        if(entity3 != NULL && GetEntInfo(entity3)->joinedAt.id == DUNGEON_JOIN_LOCATION_PARTNER)
        {
            stack1[index] = NULL;
            stack2[counter] = entity3;
            counter++;
        }
    }
    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity4 = stack1[index];
        if(entity4 != NULL)
        {
            stack1[index] = NULL;
            stack2[counter] = entity4;
            counter++;
        }
    }
    for(; counter < MAX_TEAM_MEMBERS; counter++)
    {
        stack2[counter] = NULL;
    }
    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity5 = stack2[index];
        if(entity5 != NULL)
            sub_80856C8(entity5, gDungeon->unkE220[index].x, gDungeon->unkE220[index].y);
    }
}

void sub_80855E4(DungeonCallback func)
{
    bool8 flag;
    Entity * entity;
    Entity * partnerEntity;
    s32 index;

    flag = FALSE;
    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity = gDungeon->teamPokemon[index];
        if (EntityIsValid(entity)) {
            if (GetEntInfo(entity)->joinedAt.id == DUNGEON_JOIN_LOCATION_PARTNER) {
                flag = TRUE;
            }
            func(entity);
        }
    }
    if ((!flag) && (partnerEntity = CutsceneGetPartner(), partnerEntity != NULL)) {
        func(partnerEntity);
    }
}

void sub_808563C(DungeonCallback func)
{
    Entity * entity;
    s32 index;

    for(index = 0; index < DUNGEON_MAX_WILD_POKEMON; index++)
    {
        entity = gDungeon->wildPokemon[index];
        if ((EntityIsValid(entity)) && (GetEntInfo(entity)->joinedAt.id != 0x41)) {
            func(entity);
        }
    }
}

Entity *GetEntityFromMonsterBehavior(u8 entityType)
{
    s32 index;

    for (index = 0; index < DUNGEON_MAX_POKEMON; index++) {
        Entity *entity = gDungeon->activePokemon[index];
        if (EntityIsValid(entity) && GetEntInfo(entity)->monsterBehavior == entityType)
            return entity;
    }
    return NULL;
}

void sub_80856C8(Entity * pokemon, s32 x, s32 y)
{
    sub_80694C0(pokemon, x, y, 1);
    UpdateEntityPixelPos(pokemon,NULL);
}

void sub_80856E0(Entity * pokemon, s32 direction)
{
    EntityInfo *entityInfo;
    s32 counter;

    entityInfo = GetEntInfo(pokemon);
    sub_80861D4(pokemon, 6, direction);

    for(counter = 0; counter < 5; counter++)
    {
        entityInfo->unk174 = F248_Add(entityInfo->unk174, IntToF248(2));
        DungeonRunFrameActions(0x46);
    }
    entityInfo->action.direction = direction & DIRECTION_MASK;
    sub_806CE68(pokemon, direction);

    for(counter = 0; counter < 5; counter++)
    {
        entityInfo->unk174 = F248_Sub(entityInfo->unk174, IntToF248(2));
        DungeonRunFrameActions(0x46);
    }
    entityInfo->unk174.raw = 0; // weird one that doesn't match with struct assignment
    DungeonRunFrameActions(0x46);
}

static void sub_8085764(void)
{
    Entity *entity;
    s32 index;
    Entity stackEntity;

    sub_80457DC(&stackEntity);
    for(index = 0; index < DUNGEON_MAX_WILD_POKEMON; index++)
    {
        entity = gDungeon->wildPokemon[index];
        if ((EntityIsValid(entity)) && (GetEntInfo(entity)->monsterBehavior == BEHAVIOR_ALLY)) {
            HandleFaint(entity,DUNGEON_EXIT_DEBUG_DAMAGE,&stackEntity);
        }
    }
}

static void sub_80857B8(void)
{
    u8 *direction;
    int index;

    for (index = 0; index < DUNGEON_MAX_POKEMON; index++) {
        Entity *entity = gDungeon->activePokemon[index];
        if (EntityIsValid(entity)) {
            EntityInfo *entityInfo = GetEntInfo(entity);
            if (gDungeon->unk4 == 0 && gDungeon->unk2 == 0) {
                UpdateEntityPixelPos(entity, 0);
                entityInfo->unk15C = 0;
                entityInfo->unkFE = 99;
                direction = &entityInfo->action.direction;
                entityInfo->targetPos.x = gAdjacentTileOffsets[*direction].x + entity->pos.x;
                entityInfo->targetPos.y = gAdjacentTileOffsets[*direction].y + entity->pos.y;
                sub_806CE68(entity,*direction);
            }
        }
    }
}

void sub_8085860(s32 x, s32 y)
{
  sub_803F4A0(0);
  sub_803F878(X_POS_TO_PIXELPOS(x), Y_POS_TO_PIXELPOS(y));
}

UNUSED static void sub_8085890(s32 x, s32 y)
{
    sub_803F4A0(0);
    sub_803F878(x, y);
}

void ShiftCameraToPosition(PixelPos *posStruct, s32 cameraSteps)
{
  s32 XPos;
  s32 YPos;
  s32 XIncrement;
  s32 YIncrement;

  XPos = GetCameraXPos();
  YPos = GetCameraYPos();
  XIncrement = (posStruct->x - XPos) / cameraSteps;
  YIncrement = (posStruct->y - YPos) / cameraSteps;
  sub_803F4A0(0);
  if (0 < cameraSteps) {
    do {
      XPos += XIncrement;
      YPos += YIncrement;
      sub_803F878(XPos,YPos);
      DungeonRunFrameActions(0x46);
      cameraSteps--;
    } while (cameraSteps != 0);
  }
  sub_803F878(posStruct->x,posStruct->y);
  DungeonRunFrameActions(0x46);
}

void SetFacingDirection(Entity *pokemon, s32 direction)
{
    GetEntInfo(pokemon)->action.direction = direction & DIRECTION_MASK;
    sub_806CE68(pokemon, direction);
}

void sub_8085930(s32 direction)
{
    s32 index;
    Entity *entity;
    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity = gDungeon->teamPokemon[index];
        if(EntityIsValid(entity))
        {
            if(direction >= NUM_DIRECTIONS)
            {
                sub_806CE68(entity, RandInt(NUM_DIRECTIONS));
            }
            else
            {
                GetEntInfo(entity)->action.direction = direction & DIRECTION_MASK;
                sub_806CE68(entity, direction);
            }
        }
    }
    for(index = 0; index < DUNGEON_MAX_WILD_POKEMON; index++)
    {
        entity = gDungeon->wildPokemon[index];
        if(EntityIsValid(entity))
        {
            if(GetEntInfo(entity)->monsterBehavior == BEHAVIOR_ALLY)
            {
                if(direction >= NUM_DIRECTIONS)
                {
                    sub_806CE68(entity, RandInt(NUM_DIRECTIONS));
                }
                else
                {
                    GetEntInfo(entity)->action.direction = direction & DIRECTION_MASK;
                    sub_806CE68(entity, direction);
                }
            }
        }
    }
}

static void sub_80859F0(s32 direction)
{
    s32 index;
    Entity *entity;
    for(index = 0; index < DUNGEON_MAX_WILD_POKEMON; index++)
    {
        entity = gDungeon->wildPokemon[index];
        if(EntityIsValid(entity))
        {
            if(direction >= NUM_DIRECTIONS)
            {
                sub_806CE68(entity, RandInt(NUM_DIRECTIONS));
            }
            else
            {
                GetEntInfo(entity)->action.direction = direction & DIRECTION_MASK;
                sub_806CE68(entity, direction);
            }
        }
    }
}

bool8 ShouldRunMonsterAI(Entity *pokemon)
{
    EntityInfo *pokemonInfo = GetEntInfo(pokemon);
    switch (pokemonInfo->monsterBehavior)
    {
        case BEHAVIOR_RESCUE_TARGET:
        case BEHAVIOR_SKARMORY:
        case BEHAVIOR_GENGAR:
        case BEHAVIOR_EKANS:
        case BEHAVIOR_MEDICHAM:
        case BEHAVIOR_ZAPDOS:
        case BEHAVIOR_MOLTRES:
        case BEHAVIOR_ARTICUNO:
        case BEHAVIOR_GROUDON_2:
        case BEHAVIOR_RAYQUAZA:
        case BEHAVIOR_MANKEY:
        case BEHAVIOR_MEWTWO:
        case BEHAVIOR_ENTEI:
        case BEHAVIOR_RAIKOU:
        case BEHAVIOR_SUICUNE:
        case BEHAVIOR_HO_OH:
        case BEHAVIOR_LATIOS:
        case BEHAVIOR_REGIROCK:
        case BEHAVIOR_REGICE:
        case BEHAVIOR_REGISTEEL:
        case BEHAVIOR_JIRACHI:
        case BEHAVIOR_LUGIA:
        case BEHAVIOR_KYOGRE:
        case BEHAVIOR_29:
        case BEHAVIOR_CELEBI:
        case BEHAVIOR_SMEARGLE:
        case BEHAVIOR_32:
        case BEHAVIOR_33:
        case BEHAVIOR_34:
        case BEHAVIOR_35:
        case BEHAVIOR_MAZE_BOSS:
            return TRUE;
        case BEHAVIOR_FIXED_ENEMY:
        case BEHAVIOR_ALLY:
        case BEHAVIOR_DIGLETT:
        case BEHAVIOR_ALAKAZAM_1:
        case BEHAVIOR_GROUDON_1:
        case BEHAVIOR_ALAKAZAM_2:
        default:
            return FALSE;
    }
}

UNUSED static void sub_8085B0C(Entity *pokemon)
{
  s32 index = 0;
  u8 local_28[] = {4, 3, 2, 3, 4, 5, 6, 5, 4, 3, 2, 3, 4, 5, 6, 5, 4, 4, 4, 4, 4, 4, 5, 6, 0};

  while (local_28[index] != 0) {
    SetFacingDirection(pokemon, local_28[index]);
    sub_803E708(6,0x46);
    index++;
  }
}

static EWRAM_DATA s32 gUnknown_202F3D8 = 0;

void sub_8085B4C(struct_8085B80 a0[], s16 *a1[], Entity *a2[], s32 a3)
{
    s32 i;

    gUnknown_202F3D8 = a3;

    for (i = 0; i < a3; i++) {
        a0[i].unk0 = 1;
        a0[i].unk4 = a2[i];
        a0[i].unkC = a1[i];
        a0[i].unk8 = a1[i][2];
    }
}

bool8 sub_8085B80(struct_8085B80 *a0)
{
    s16 *unkCPtr;
    int i;
    bool8 ret = FALSE;

    for (i = 0; i < gUnknown_202F3D8; i++) {
        Entity *entity = a0[i].unk4;
        EntityInfo *enInfo = GetEntInfo(entity);
        unkCPtr = a0[i].unkC;
        if (a0[i].unk0 != 0) {
            ret = TRUE;
            if (a0[i].unk8 == 0) {
                unkCPtr = unkCPtr + 4;
                if (unkCPtr[2] == 0) {
                    a0[i].unk0 = 0;
                    enInfo->action.direction = unkCPtr[0] & DIRECTION_MASK;
                    sub_806CDD4(entity, 7, unkCPtr[0]);
                    sub_80856C8(entity, unkCPtr[5], unkCPtr[6]);
                    continue;
                }
                a0[i].unk8 = unkCPtr[2];
                a0[i].unkC = unkCPtr;
              enInfo->action.direction = unkCPtr[0] & DIRECTION_MASK;
                sub_806CDD4(entity, 0, (int)*unkCPtr);
            }

            a0[i].unk8--;
            IncreaseEntityPixelPos(entity,
                gAdjacentTileOffsets[*unkCPtr].x * unkCPtr[1] * 0x100,
                gAdjacentTileOffsets[*unkCPtr].y * unkCPtr[1] * 0x100);
        }
    }
    return ret;
}

void SetDungeonBGColorRGB(s32 r, s32 g, s32 b, bool8 a3, bool8 a4)
{
    s32 i, palIndex, n;
    const RGB *colorPtr;
    RGB color;
    struct RgbS16 colorS16;

    n = 160;
    sub_8085E98();
    palIndex = 0;
    colorPtr = gDungeonPaletteFile->data;
    if (a4) {
        if (r >= 0) { r /= 2; }
        if (g >= 0) { g /= 2; }
        if (b >= 0) { b /= 2; }
    }

    color.r = 0;
    color.g = 0;
    color.b = 0;
    SetBGPaletteBufferColorRGB(palIndex, &color, gDungeonBrightness, gDungeon->colorRamp);
    colorPtr++;
    palIndex++;

    for (i = 1; i < n; i++) {
        s32 newR = colorPtr->r + r;
        s32 newG = colorPtr->g + g;
        s32 newB = colorPtr->b + b;

        if (newR > 0xFF) { newR = 0xFF; }
        if (newG > 0xFF) { newG = 0xFF; }
        if (newB > 0xFF) { newB = 0xFF; }

        if (newR < 0)       { newR = 0; }
        if (newG < 0)       { newG = 0; }
        if (newB < 0)       { newB = 0; }

        color.r = newR;
        color.g = newG;
        color.b = newB;
        SetBGPaletteBufferColorRGB(palIndex, &color, gDungeonBrightness, gDungeon->colorRamp);
        colorPtr++;
        palIndex++;
    }

    colorS16.r = r;
    colorS16.g = g;
    colorS16.b = b;
    sub_8004C00(gUnknown_202EE8C, 160, 32, gDungeonBrightness, gDungeon->colorRamp, &colorS16);
    palIndex += 32;

    colorPtr = gUnknown_202ECA4;
    n = 32;
    for (i = 0; i < n; i++) {
        s32 newR = colorPtr->r + r;
        s32 newG = colorPtr->g + g;
        s32 newB = colorPtr->b + b;

        if (newR > 0xFF) { newR = 0xFF; }
        if (newG > 0xFF) { newG = 0xFF; }
        if (newB > 0xFF) { newB = 0xFF; }

        if (newR < 0)       { newR = 0; }
        if (newG < 0)       { newG = 0; }
        if (newB < 0)       { newB = 0; }

        color.r = newR;
        color.g = newG;
        color.b = newB;
        SetBGPaletteBufferColorRGB(palIndex, &color, gDungeonBrightness, gDungeon->colorRamp);
        colorPtr++;
        palIndex++;
    }

    if (a3) {
        colorPtr = gDungeon->paletFile->data;
        palIndex = 256;
        n = 208;
        for (i = 0; i < n; i++) {
            s32 newR = colorPtr->r + r;
            s32 newG = colorPtr->g + g;
            s32 newB = colorPtr->b + b;

            if (newR > 0xFF) { newR = 0xFF; }
            if (newG > 0xFF) { newG = 0xFF; }
            if (newB > 0xFF) { newB = 0xFF; }

            if (newR < 0)       { newR = 0; }
            if (newG < 0)       { newG = 0; }
            if (newB < 0)       { newB = 0; }

            color.r = newR;
            color.g = newG;
            color.b = newB;
            SetBGPaletteBufferColorRGB(palIndex, &color, gDungeonBrightness, NULL);
            colorPtr++;
            palIndex++;
        }
    }
}

void sub_8085E98(void)
{
    gDungeon->unk181e8.unk18215 = 0;
}

void sub_8085EB0(void)
{
    gDungeon->unk181e8.unk18215 = 1;
}

static const unkStruct_2039DB0 sDefaultSpriteMasks = DEFAULT_UNK_2039DB0_MASKS;

u32 sub_8085EC8(s16 param_1,u32 param_2,u32 param_3,DungeonPos *param_4, bool32 param_5)
{
    u32 uVar1;
    unkStruct_80416E0 local_40;

    bool8 param_5_bool8;
    s32 param_1_s32 = param_1;


    param_5_bool8 = param_5;

    sub_800EE5C(param_1_s32);
    sub_800EF64();
    DungeonRunFrameActions(0x46);
    local_40.unk0 = param_1;
    local_40.unk4 = param_2;
    local_40.dir = param_3;
    local_40.pos1.x = param_4->x;
    local_40.pos1.y = param_4->y;
    local_40.pos2.x = 0;
    local_40.pos2.y = 0;
    local_40.unk18 = 0xffff;
    local_40.spriteMasks = sDefaultSpriteMasks;

    uVar1 = sub_800E890(&local_40);
    if (param_5_bool8) {
        sub_8085F44(uVar1);
        uVar1 = -1;
    }
    return uVar1;
}

void sub_8085F44(s32 param_1)
{
    while (sub_800E9A8(param_1)) {
        sub_800E90C(&gDungeon->unk181e8.cameraPixelPos);
        DungeonRunFrameActions(0x46);
    }
}

void sub_8085F78(void)
{
    switch(gDungeon->unk3A0D) {
        case 0x12:
        case 0x13:
        case 0x14:
            sub_8088848();
            break;
        case 0x15:
            sub_8088EE8();
            break;
        case 0x29:
        case 0x2A:
            sub_808A718();
            break;
        case 0:
        case 0x3C:
        default:
            break;
    }
}

bool8 sub_80860A8(u8 id)
{
    Item *item;
    EntityInfo *info;
    Entity *entity;
    int index;


    for(index = 0; index < INVENTORY_SIZE; index++)
    {
        item = &gTeamInventoryRef->teamItems[index];
        if((item->flags & 1) != 0)
        {
            if((item->id) == id) return TRUE;
        }
    }

    for(index = 0; index < MAX_TEAM_MEMBERS; index++)
    {
        entity = gDungeon->teamPokemon[index];
        if (((EntityIsValid(entity)) && (info = GetEntInfo(entity), ((info->heldItem).flags & 1))) && ((info->heldItem).id == id)) return TRUE;
    }
    return FALSE;
}

UNUSED static void sub_8086124(Entity *entity, u8 param_2)
{
    GetEntInfo(entity)->unk160 = param_2;
}

void sub_8086130(void)
{
    DungeonFadeOutBGM(0x3c);
    sub_803E708(0x3c,0x46);
    sub_8052FB8(gUnknown_810665C);
    sub_803E708(0x1e,0x46);
    sub_8052FB8(gUnknown_810668C);
    sub_803E708(0x1e,0x46);
    sub_8052FB8(gUnknown_81066D4);
    sub_803E708(0x1e,0x46);
    sub_8052FB8(gUnknown_81066F0);
    sub_803E708(0x1e,0x46);
    sub_8052FB8(gUnknown_810671C);
    sub_803E708(0x1e,0x46);
}

void sub_80861A8(Entity *a0)
{
    sub_806CDD4(a0, 0, NUM_DIRECTIONS);
}

void sub_80861B8(Entity *a0, u8 r1, s32 direction)
{
    EntityInfo *info;

    info = GetEntInfo(a0);

    sub_806CDD4(a0, r1, direction);
    info->unkFE = r1;
}

void sub_80861D4(Entity *a0, u8 r1, s32 direction)
{
    sub_806CDD4(a0, r1, direction);
    sub_80861EC(a0);
}

static void sub_80861EC(Entity *a0)
{
    GetEntInfo(a0)->unkFE = 0x63;
}

s32 sub_80861F8(s32 param_1,Entity *param_2,bool32 param_3)
{
    EntityInfo *info;
    s32 uVar2;
    s32 uStack_38;
    DungeonPos pos;
    unkStruct_80416E0 stack;

    // Needed to match.
    s32 param_1Copy2 = (s16) param_1;
    s32 param_1Copy = param_1Copy2;
    u8 param_3_bool32 = param_3;

    sub_800EE5C(param_1Copy2);
    sub_800EF64();
    DungeonRunFrameActions(0x46);
    info = GetEntInfo(param_2);
    pos.x = 0;
    pos.y = 0;
    uStack_38 = sub_800E700(param_1Copy);
    if (uStack_38 != -1) {
        sub_800569C(&pos, &param_2->axObj.axdata, uStack_38);
    }
    stack.unk0 = param_1Copy;
    stack.unk4 = 0;
    stack.dir = info->action.direction;
    stack.pos1.x = (param_2->pixelPos).x / 256;
    stack.pos1.y = (param_2->pixelPos).y / 256;

    stack.pos2.x = pos.x;
    stack.pos2.y = pos.y;
    stack.unk14 = uStack_38;
    stack.unk18 = 0xffff;
    stack.spriteMasks = DEFAULT_UNK_2039DB0_MASKS;

    uVar2 = sub_800E890(&stack);
    if (param_3_bool32) {
        sub_8085F44(uVar2);
        uVar2 = -1;
    }
    return uVar2;
}

void SpriteShockEffect(Entity *entity)
{
    PlaySoundEffect(464);
    sub_80861F8(323, entity, 0);
}

static void sub_80862DC(Entity *entity)
{
    PixelPos pos;
    pos.x = entity->pixelPos.x;
    pos.y = entity->pixelPos.y + 0x3800;

    UpdateEntityPixelPos(entity, &pos);
    sub_806CDD4(entity, 0, DIRECTION_NORTH);
    sub_8086A54(entity);
}

static void sub_8086310(Entity *entity)
{
    PixelPos pos;
    pos.x = entity->pixelPos.x;
    pos.y = entity->pixelPos.y + 0x9000;

    UpdateEntityPixelPos(entity, &pos);
    sub_806CE68(entity, DIRECTION_SOUTH);
    sub_8086A3C(entity);
    entity->isVisible = 0;
}

static void sub_8086348(Entity *entity)
{
    PixelPos pos;
    pos.x = entity->pixelPos.x + 0x7800;
    pos.y = entity->pixelPos.y - 0x2000;

    UpdateEntityPixelPos(entity, &pos);
    sub_806CDD4(entity, 0, DIRECTION_WEST);
    sub_8086A54(entity);
}

static void sub_8086384(Entity *entity)
{
    PixelPos pos;
    pos.x = entity->pixelPos.x + 0x7800;
    pos.y = entity->pixelPos.y;

    UpdateEntityPixelPos(entity, &pos);
    sub_806CDD4(entity, 0, DIRECTION_WEST);
    sub_8086A54(entity);
}

static void sub_80863B8(Entity *entity)
{
    sub_806CDD4(entity, 0, DIRECTION_NORTH);
}

static void sub_80863C8(Entity *entity)
{
    sub_806CDD4(entity, 0, DIRECTION_SOUTH);
}

static void sub_80863D8(Entity *entity)
{
    sub_806CDD4(entity, 0, DIRECTION_WEST);
}

static void sub_80863E8(Entity *entity)
{
    sub_806CDD4(entity, 6, DIRECTION_NORTH);
}

static void sub_80863F8(Entity *entity)
{
    sub_806CE68(entity, DIRECTION_NORTH);
}

static void sub_8086404(Entity *entity)
{
    sub_806CE68(entity, DIRECTION_WEST);
}

static void sub_8086410(Entity *entity)
{
    IncreaseEntityPixelPos(entity, 0, -0x100);
}

static void sub_8086424(Entity *entity)
{
    IncreaseEntityPixelPos(entity, 0, 0x100);
}

static void sub_8086434(Entity *entity)
{
    IncreaseEntityPixelPos(entity, -0x100, 0);
}

void sub_8086448(void)
{
    s32 i;
    u32 XPos;
    u32 YPos;

    sub_80855E4(sub_80862DC);

    for (i = 0; i < 56; i++) {
        XPos = GetCameraXPos();
        YPos = GetCameraYPos();
        YPos += 0x100;
        sub_803F878(XPos, YPos);
        sub_80855E4(sub_8086410);
        DungeonRunFrameActions(0x46);
    }

    sub_80855E4(sub_80863F8);
}

static void sub_8086494(void)
{
    s32 i;

    sub_8085930(DIRECTION_NORTHEAST);
    sub_803E708(4, 70);

    sub_8085930(DIRECTION_EAST);
    sub_803E708(4, 70);

    sub_8085930(DIRECTION_SOUTHEAST);
    sub_803E708(4, 70);

    sub_8085930(DIRECTION_SOUTH);
    sub_803E708(4, 70);

    sub_80855E4(sub_80863C8);

    for (i = 0; i < 144; i++) {
        sub_80855E4(sub_8086424);
        DungeonRunFrameActions(70);
    }

    sub_80855E4(sub_8086310);
}

void sub_8086500(void)
{
    s32 i;
    u32 XPos;
    u32 YPos;

    sub_80855E4(sub_8086348);

    for (i = 0; i < 72; i++) {
        XPos = GetCameraXPos();
        XPos += 0x100;
        YPos = GetCameraYPos();
        sub_803F878(XPos, YPos);
        sub_80855E4(sub_8086434);
        DungeonRunFrameActions(70);
    }

    sub_80855E4(sub_8086404);
}

void sub_808654C(void)
{
    s32 i;
    u32 XPos;
    u32 YPos;

    sub_80855E4(sub_8086384);

    for (i = 0; i < 120; i++) {
        XPos = GetCameraXPos();
        XPos += 0x100;
        YPos = GetCameraYPos();
        sub_803F878(XPos, YPos);
        sub_80855E4(sub_8086434);
        DungeonRunFrameActions(70);
    }

    sub_80855E4(sub_8086404);
}

void sub_8086598(void)
{
    s32 i;
    u32 XPos;
    u32 YPos;

    sub_80855E4(sub_80863B8);

    for (i = 0; i < 24; i++) {
        XPos = GetCameraXPos();
        YPos = GetCameraYPos();
        sub_803F878(XPos, YPos - 0x100);
        sub_80855E4(sub_8086410);
        DungeonRunFrameActions(70);
    }

    sub_80855E4(sub_80863F8);
}

void sub_80865E8(void)
{
    s32 i;
    u32 XPos;
    u32 YPos;

    sub_80855E4(sub_80863E8);

    for (i = 0; i < 12; i++) {
        XPos = GetCameraXPos();
        YPos = GetCameraYPos();
        sub_803F878(XPos, YPos + 0x200);
        sub_80855E4(sub_8086424);
        sub_80855E4(sub_8086424);
        DungeonRunFrameActions(70);
    }

    sub_80855E4(sub_80863F8);
}

void sub_808663C(void)
{
    s32 i;
    u32 XPos;
    u32 YPos;

    sub_80855E4(sub_80863E8);

    for (i = 0; i < 16; i++) {
        XPos = GetCameraXPos();
        YPos = GetCameraYPos();
        sub_803F878(XPos, YPos + 0x200);
        sub_80855E4(sub_8086424);
        sub_80855E4(sub_8086424);
        DungeonRunFrameActions(70);
    }

    sub_80855E4(sub_80863F8);
}

void sub_8086690(void)
{
    s32 i;

    sub_80855E4(sub_80863D8);

    for (i = 0; i < 48; i++) {
        sub_80855E4(sub_8086434);
        DungeonRunFrameActions(70);
    }

    sub_80855E4(sub_8086404);
}

void sub_80866C4(const struct DungeonDialogueStruct *dialogue)
{
    SpriteLookAroundEffect(CutsceneGetLeader());
    sub_803E708(10, 70);
    DisplayDungeonDialogue(dialogue);
    sub_803E708(10, 70);
    sub_8086494();
    gDungeon->unk2 = 1;
}

UNUSED static void sub_80866FC(void)
{
    s32 i;

    for (i = 0; i >= -250; i -= 10) {
        SetDungeonBGColorRGB(i, i, i, 1, 0);
        DungeonRunFrameActions(70);
    }
}

UNUSED static void sub_808627C(void)
{
    sub_803E748();
}

void sub_8086738(void)
{
    s32 i;

    for (i = 0; i < 250; i += 10) {
        SetDungeonBGColorRGB(i, i, i, 1, 0);
        DungeonRunFrameActions(70);
    }
}

void sub_8086764(void)
{
    s32 i;

    for (i = 250; i >= 0; i -= 5) {
        SetDungeonBGColorRGB(i, i, i, 1, 0);
        DungeonRunFrameActions(70);
    }

    sub_8085EB0();
}

void sub_8086794(void)
{
    s32 i;

    gDungeonBrightness = 0;

    for (i = 0; i < 200; i++) {
        SetDungeonBGColorRGB(0, 0, 0, 1, 0);
        BgColorCallNullsub4();
        DungeonRunFrameActions(70);

        if ((i & 3) == 0) {
            gDungeonBrightness++;
            if (gDungeonBrightness == 31)
                break;
        }
    }

    DungeonRunFrameActions(70);
    gDungeon->unk7 = 0;
}

void sub_80867F4(void)
{
    gUnknown_202F3D0.unk0 = 0;
    gUnknown_202F3D0.unk1 = 0;
    gUnknown_202F3D0.unk2 = 0;
    gUnknown_202F3D0.unk3 = 0;
    gUnknown_202F3D0.unk4 = 0;
    gUnknown_202F3D0.unk5 = 0;
}

void sub_808680C(void)
{
    gUnknown_202F3D0.unk0 = 1;
    if (gUnknown_202F3D0.unk3 != 0)
        PlaySoundEffect(505);
}

void sub_808682C(void)
{
    gUnknown_202F3D0.unk0 = 0;
}

void sub_8086838(u8 entity, u8 r1, u8 r2)
{
    gUnknown_202F3D0.unk1 = entity;
    gUnknown_202F3D0.unk2 = r1;
    gUnknown_202F3D0.unk3 = r2;
}

void sub_8086848(u8 entity, u8 r1)
{
    gUnknown_202F3D0.unk4 = entity;
    gUnknown_202F3D0.unk5 = r1;
}

void sub_8086854(void)
{
    if (gUnknown_202F3D0.unk0) {
        if (gUnknown_202F3D0.unk4 == 0) {
            if (gUnknown_202F3D0.unk1) {
                gUnknown_202F3D0.unk4 = 80;
                gUnknown_202F3D0.unk5 = 16;
            }
            else {
                gUnknown_202F3D0.unk4 = RandInt(6) + 2;
                gUnknown_202F3D0.unk5 = RandInt(6) + 2;
            }

            if (gUnknown_202F3D0.unk3 != 0)
                PlaySoundEffect(505);
        }
        else
            gUnknown_202F3D0.unk4--;

        if (gUnknown_202F3D0.unk5 != 0)
            gUnknown_202F3D0.unk5--;

        gDungeon->unk181e8.unk181FC = gUnknown_8107314[gUnknown_202F3D0.unk5];
    }
    else
        gDungeon->unk181e8.unk181FC = 0;
}

void sub_80868F4(Entity *entity)
{
    GetEntInfo(entity)->unk15C = 1;
    GetEntInfo(entity)->unk15D = 1;
}

void sub_8086910(Entity *entity)
{
    GetEntInfo(entity)->unk15C = 0;
    GetEntInfo(entity)->unk15D = 0;
}

void sub_808692C(void)
{
    sub_80859F0(DIRECTION_NORTHWEST);
    sub_803E708(4, 70);

    sub_80859F0(DIRECTION_WEST);
    sub_803E708(4, 70);

    sub_80859F0(DIRECTION_SOUTHWEST);
    sub_803E708(4, 70);

    sub_80859F0(DIRECTION_SOUTH);
    sub_803E708(4, 70);
}

void SpriteLookAroundEffect(Entity *entity)
{
    s8 r4;
    s8 r3;

    r4 = sub_8002984(GetEntInfo(entity)->action.direction, 4);

    sub_80869E4(entity, 4, 2, r4);

    sub_803E708(15, 70);

    r4 = sub_8002984(r4, 5);

    sub_80869E4(entity, 4, 1, r4);

    sub_803E708(15, 70);

    r3 = sub_8002984(r4, 4);

    sub_80869E4(entity, 4, 2, r3);

    sub_803E708(15, 70);
}

void sub_80869E4(struct Entity *entity, s32 a1, u8 a2, s32 _someDirection)
{
    s32 someDirection = (s8) _someDirection;
    struct EntityInfo *info = GetEntInfo(entity);
    s32 direction = (s8) info->action.direction;

    while (direction != someDirection) {
        direction = (s8) sub_8002A70(direction, someDirection, a2);
        info->action.direction = direction;
        info->action.direction &= DIRECTION_MASK;
        sub_806CE68(entity, info->action.direction);
        sub_803E708(a1, 70);
    }
}

void sub_8086A3C(Entity *pokemon)
{
    GetEntInfo(pokemon)->unk15C = 1;
    GetEntInfo(pokemon)->unk15E = 1;
}

void sub_8086A54(Entity *pokemon)
{
    GetEntInfo(pokemon)->unk15C = 1;
    GetEntInfo(pokemon)->unk15E = 0;
}

void SetupBossFightHP(Entity *pokemon, s32 newHP, u16 songIndex)
{
  EntityInfo *entityInfo = GetEntInfo(pokemon);

  entityInfo->bossFlag = TRUE;

  // BUG: Source of the Reviver Seed Boss Glitch
  //
  // Video to demonstration:
  // https://www.youtube.com/watch?v=rHu7EehrZ68
  entityInfo->originalHP = entityInfo->maxHPStat;
  if (newHP != 0) {
    entityInfo->maxHPStat = newHP;
    entityInfo->HP = newHP;
  }

  gDungeon->unk644.bossSongIndex = songIndex;
  SetDefaultIQSkills(&entityInfo->IQSkillMenuFlags, entityInfo->bossFlag);
  LoadIQSkills(pokemon);
}

void sub_8086AC0(void)
{
    if(!IsFloorOver())
        if(gDungeon->unk2 == 0)
            sub_8097FF8();
}

u8 sub_8086AE4(s16 _index)
{
    s32 pokeIndex = _index;

    if(gDungeon->unk644.unk18 == 0)
        return 1;
    else
        return HasRecruitedMon(pokeIndex);
}
