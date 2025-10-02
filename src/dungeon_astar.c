/* dungeon_astar.c — compact C89-safe A* for GBA (IWRAM + no h_cost[]) */
#include "global.h"
#include "globaldata.h"
#include "constants/dungeon.h"
#include "constants/direction.h"
#include "structs/map.h"
#include "structs/str_dungeon.h"
#include "dungeon_map_access.h"
#include "dungeon_astar.h"

#define GRID_W DUNGEON_MAX_SIZE_X
#define GRID_H DUNGEON_MAX_SIZE_Y
#define GRID_N ((GRID_W) * (GRID_H))

enum { NODE_UNSEEN = 0, NODE_OPEN = 1, NODE_CLOSED = 2 };
#define PARENT_NONE 0xFFFF

/* Section attribute for IWRAM */
#ifndef IWRAM_DATA
#define IWRAM_DATA __attribute__((section(".iwram")))
#endif

static const s8 OFFS[8][2] = {
    { -1, -1 }, {  0, -1 }, {  1, -1 },
    { -1,  0 },             {  1,  0 },
    { -1,  1 }, {  0,  1 }, {  1,  1 }
};

/* Move node arrays to IWRAM and remove h_cost[] */
static IWRAM_DATA u16 g_cost[GRID_N];
static IWRAM_DATA u16 parent_idx[GRID_N];
static IWRAM_DATA u8  state[GRID_N];
static bool8 astar_initialized = FALSE;

/* --- utils --- */
static bool8 InBounds(s32 x, s32 y)
{
    if (x < 0 || y < 0) return FALSE;
    if (x >= GRID_W || y >= GRID_H) return FALSE;
    return TRUE;
}
static u16 Idx(s32 x, s32 y) { return (u16)(y * GRID_W + x); }
static void XYFromIdx(u16 idx, s32 *x, s32 *y) { *y = idx / GRID_W; *x = idx % GRID_W; }

static bool8 IsT1Tile(s32 x, s32 y)
{
    const Tile *tile;
    u16 type;
    if (!InBounds(x, y)) return FALSE;
    tile = GetTile(x, y);
    if (tile == NULL) return FALSE;
    type = tile->terrainFlags & (TERRAIN_TYPE_NORMAL | TERRAIN_TYPE_SECONDARY);
    if (type != TERRAIN_TYPE_NORMAL) return FALSE;
    if (tile->terrainFlags & (TERRAIN_TYPE_IMPASSABLE_WALL |
                              TERRAIN_TYPE_UNREACHABLE_FROM_STAIRS |
                              TERRAIN_TYPE_UNBREAKABLE))
        return FALSE;
    /* if (TileHasBlockingEntity(x, y)) return FALSE; */
    return TRUE;
}

/* Octile heuristic computed on the fly (saves 2 bytes/tile) */
static u16 OctileHeuristic(s32 x1, s32 y1, s32 x2, s32 y2)
{
    s32 dx, dy, minv, maxv;
    dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);
    dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);
    minv = (dx < dy) ? dx : dy;
    maxv = (dx > dy) ? dx : dy;
    return (u16)(14 * minv + 10 * (maxv - minv));
}

static bool8 CanMoveDiagonalSafely(s32 cx, s32 cy, s32 dx, s32 dy)
{
    if (dx == 0 || dy == 0) return TRUE;
    if (!IsT1Tile(cx + dx, cy)) return FALSE;
    if (!IsT1Tile(cx, cy + dy)) return FALSE;
    return TRUE;
}

static void ResetAStarBuffers(void)
{
    u16 i;
    for (i = 0; i < GRID_N; i++) {
        g_cost[i] = 0;
        parent_idx[i] = PARENT_NONE;
        state[i] = NODE_UNSEEN;
    }
    astar_initialized = TRUE;
}

/* Linear OPEN scan; f = g + h computed inline */
static u16 PopBestOpenIndex(s32 goal_x, s32 goal_y)
{
    u16 best = PARENT_NONE;
    u32 best_f = 0xFFFFFFFFu;
    u16 best_h_local = 0xFFFF;
    u16 best_g_local = 0;
    u16 i;

    for (i = 0; i < GRID_N; i++) {
        if (state[i] == NODE_OPEN) {
            s32 x, y;
            u16 g, h;
            u32 f;

            XYFromIdx(i, &x, &y);
            g = g_cost[i];
            h = OctileHeuristic(x, y, goal_x, goal_y);
            f = (u32)g + (u32)h;

            if (f < best_f ||
                (f == best_f && (h < best_h_local ||
                 (h == best_h_local && g > best_g_local)))) {
                best = i;
                best_f = f;
                best_h_local = h;
                best_g_local = g;
            }
        }
    }
    return best;
}

static DungeonPos FirstStepFromParents(u16 goal_i, DungeonPos start)
{
    DungeonPos step;
    u16 cur;

    step.x = -1; step.y = -1;
    cur = goal_i;

    while (cur != PARENT_NONE) {
        u16 p;
        s32 px, py, cx, cy;

        p = parent_idx[cur];
        if (p == PARENT_NONE) break;

        XYFromIdx(p, &px, &py);
        XYFromIdx(cur, &cx, &cy);

        if (px == start.x && py == start.y) {
            step.x = cx; step.y = cy;
            return step;
        }
        cur = p;
    }
    return step;
}

/* --- public --- */
DungeonPos AStarPathfind(DungeonPos start, DungeonPos goal)
{
    DungeonPos next_step;
    u16 s_idx, g_idx;
    u16 cur;
    s32 cx, cy;
    s32 k;

    next_step.x = -1; next_step.y = -1;

    if (!InBounds(start.x, start.y) || !InBounds(goal.x, goal.y)) return next_step;
    if (!IsT1Tile(start.x, start.y) || !IsT1Tile(goal.x, goal.y)) return next_step;
    if (start.x == goal.x && start.y == goal.y) return start;

    ResetAStarBuffers();

    s_idx = Idx(start.x, start.y);
    g_idx = Idx(goal.x,  goal.y);

    g_cost[s_idx] = 0;
    state[s_idx]  = NODE_OPEN;

    for (;;) {
        cur = PopBestOpenIndex(goal.x, goal.y);
        if (cur == PARENT_NONE) break;          /* no path */
        if (cur == g_idx) {                      /* reached goal */
            next_step = FirstStepFromParents(cur, start);
            break;
        }

        state[cur] = NODE_CLOSED;
        XYFromIdx(cur, &cx, &cy);

        for (k = 0; k < 8; k++) {
            s32 dx, dy, nx, ny;
            u16 ni;
            u16 move;
            u16 tentative;

            dx = OFFS[k][0]; dy = OFFS[k][1];
            nx = cx + dx;    ny = cy + dy;

            if (!InBounds(nx, ny)) continue;
            if (!IsT1Tile(nx, ny)) continue;
            if (!CanMoveDiagonalSafely(cx, cy, dx, dy)) continue;

            ni = Idx(nx, ny);
            if (state[ni] == NODE_CLOSED) continue;

            move = (dx != 0 && dy != 0) ? 14 : 10;
            tentative = (u16)(g_cost[cur] + move);

            if (state[ni] != NODE_OPEN) {
                state[ni] = NODE_OPEN;
                parent_idx[ni] = cur;
                g_cost[ni] = tentative;
            } else if (tentative < g_cost[ni]) {
                parent_idx[ni] = cur;
                g_cost[ni] = tentative;
            }
        }
    }
    return next_step;
}

s32 AStarGetDirection(DungeonPos current, DungeonPos next_step)
{
    s32 dx = next_step.x - current.x;
    s32 dy = next_step.y - current.y;

    if (dx == -1 && dy == -1) return DIRECTION_NORTHWEST;
    if (dx ==  0 && dy == -1) return DIRECTION_NORTH;
    if (dx ==  1 && dy == -1) return DIRECTION_NORTHEAST;
    if (dx == -1 && dy ==  0) return DIRECTION_WEST;
    if (dx ==  1 && dy ==  0) return DIRECTION_EAST;
    if (dx == -1 && dy ==  1) return DIRECTION_SOUTHWEST;
    if (dx ==  0 && dy ==  1) return DIRECTION_SOUTH;
    if (dx ==  1 && dy ==  1) return DIRECTION_SOUTHEAST;
    return -1;
}
