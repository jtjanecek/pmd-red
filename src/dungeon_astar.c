/* dungeon_astar.c — fast A* tuned for GBA (IWRAM arrays, heap, no full clears) */
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
#define PARENT_NONE 0xFFFF
#define IWRAM_DATA __attribute__((section("iwram_data")))

enum { NODE_UNSEEN = 0, NODE_OPEN = 1, NODE_CLOSED = 2 };

/* neighbor offsets + costs */
static const s8 OFFS[8][2] = {
    { -1,-1 }, { 0,-1 }, { 1,-1 },
    { -1, 0 },          { 1, 0 },
    { -1, 1 }, { 0, 1 }, { 1, 1 }
};
static const u8 MOVE_COST[8] = {14,10,14,10,10,14,10,14};

/* --- hot structures in IWRAM --- */
static IWRAM_DATA u16 g_cost[GRID_N]                 = {0};
static IWRAM_DATA u16 parent_idx[GRID_N]             = {0};
static IWRAM_DATA u32 closed_bits[(GRID_N+31)/32]    = {0};
static IWRAM_DATA u16 heap_size                      = 0;
static IWRAM_DATA u16 cur_gen                        = 1;
static IWRAM_DATA bool8 astar_tables_ready           = FALSE;

/* move hot-but-bulky to IWRAM to free EWRAM and speed OPEN ops */
static IWRAM_DATA u16 heap[GRID_N]                   = {0};
static IWRAM_DATA u8  x_of[GRID_N]                   = {0};
static IWRAM_DATA u8  y_of[GRID_N]                   = {0};

/* --- bulk/cold in EWRAM --- */
static EWRAM_DATA u16 gen[GRID_N];  /* generation stamp */



/* --- utils --- */
static inline bool8 InBounds(s32 x,s32 y){return (u32)x<GRID_W && (u32)y<GRID_H;}
static inline u16 Idx(s32 x,s32 y){return (u16)(y*GRID_W+x);}

static void ClosedClearAll(void){
    u16 i,n=(u16)((GRID_N+31)/32);
    for(i=0;i<n;i++) closed_bits[i]=0;
}
static inline bool8 ClosedTest(u16 i){return (closed_bits[i>>5]>>(i&31))&1;}
static inline void ClosedSet(u16 i){closed_bits[i>>5]|=(1u<<(i&31));}

static bool8 IsT1Tile(s32 x,s32 y){
    const Tile* tile; u16 type;
    if(!InBounds(x,y)) return FALSE;
    tile=GetTile(x,y); if(!tile) return FALSE;
    type=tile->terrainFlags & (TERRAIN_TYPE_NORMAL|TERRAIN_TYPE_SECONDARY);
    if(type!=TERRAIN_TYPE_NORMAL) return FALSE;
    if(tile->terrainFlags & (TERRAIN_TYPE_IMPASSABLE_WALL|
                             TERRAIN_TYPE_UNREACHABLE_FROM_STAIRS|
                             TERRAIN_TYPE_UNBREAKABLE)) return FALSE;
    return TRUE;
}
static inline u16 OctileHeuristic(u16 i,s32 gx,s32 gy){
    s32 x=x_of[i], y=y_of[i];
    s32 dx=(x>gx)?x-gx:gx-x;
    s32 dy=(y>gy)?y-gy:gy-y;
    s32 minv=(dx<dy)?dx:dy, maxv=(dx>dy)?dx:dy;
    return (u16)(14*minv+10*(maxv-minv));
}
static inline bool8 CanMoveDiagonalSafely(s32 cx,s32 cy,s32 dx,s32 dy){
    return (dx==0||dy==0) || (IsT1Tile(cx+dx,cy)&&IsT1Tile(cx,cy+dy));
}

/* --- heap (lazy duplicates) --- */
static inline int F_better(u16 a,u16 b,s32 gx,s32 gy){
    u16 ga=g_cost[a], gb=g_cost[b];
    u16 ha=OctileHeuristic(a,gx,gy), hb=OctileHeuristic(b,gx,gy);
    u32 fa=(u32)ga+ha, fb=(u32)gb+hb;
    if(fa!=fb) return fa<fb;
    if(ha!=hb) return ha<hb;
    return ga>gb;
}
static void Heap_SiftUp(u16 i,s32 gx,s32 gy){
    while(i>0){
        u16 p=(i-1)>>1;
        if(!F_better(heap[i],heap[p],gx,gy)) break;
        {u16 t=heap[i];heap[i]=heap[p];heap[p]=t;} i=p;
    }
}
static void Heap_SiftDown(u16 i,s32 gx,s32 gy){
    for(;;){
        u16 l=(i<<1)+1,r=l+1,b=i;
        if(l<heap_size&&F_better(heap[l],heap[b],gx,gy)) b=l;
        if(r<heap_size&&F_better(heap[r],heap[b],gx,gy)) b=r;
        if(b==i) break;
        {u16 t=heap[i];heap[i]=heap[b];heap[b]=t;} i=b;
    }
}
static void OpenPush(u16 n,s32 gx,s32 gy){heap[heap_size++]=n;Heap_SiftUp(heap_size-1,gx,gy);}
static u16 OpenPop(s32 gx, s32 gy)
{
    u16 best;

    if (heap_size == 0)
        return PARENT_NONE;

    best = heap[0];
    heap_size--;
    if (heap_size) {
        heap[0] = heap[heap_size];
        Heap_SiftDown(0, gx, gy);
    }
    return best;
}

/* --- init --- */
static void AStarInitTables(void){
    if(!astar_tables_ready){
        u16 i=0; s32 y,x;
        for(y=0;y<GRID_H;y++)for(x=0;x<GRID_W;x++,i++){x_of[i]=x;y_of[i]=y;gen[i]=0;}
        cur_gen=1; astar_tables_ready=TRUE;
    }
}
static void ResetForSearch(void){
    cur_gen++; if(cur_gen==0){cur_gen=1; memset(gen,0,sizeof(gen));}
    ClosedClearAll(); heap_size=0;
}
static inline bool8 IsSeen(u16 i){return gen[i]==cur_gen;}
static inline void MarkSeen(u16 i){gen[i]=cur_gen;}

/* reconstruct first step */
static DungeonPos FirstStepFromParents(u16 goal_i,DungeonPos start){
    DungeonPos step={-1,-1}; u16 cur=goal_i;
    while(cur!=PARENT_NONE){
        u16 p=parent_idx[cur]; if(p==PARENT_NONE) break;
        if(x_of[p]==start.x && y_of[p]==start.y){step.x=x_of[cur];step.y=y_of[cur];return step;}
        cur=p;
    }
    return step;
}

/* --- public --- */
DungeonPos AStarPathfind(DungeonPos start,DungeonPos goal){
    DungeonPos next={-1,-1};
    u16 s_idx,g_idx,cur; s32 cx,cy; int k;

    AStarInitTables();
    if(!InBounds(start.x,start.y)||!InBounds(goal.x,goal.y)) return next;
    if(!IsT1Tile(start.x,start.y)||!IsT1Tile(goal.x,goal.y)) return next;
    if(start.x==goal.x&&start.y==goal.y) return start;

    ResetForSearch();
    s_idx=Idx(start.x,start.y); g_idx=Idx(goal.x,goal.y);
    g_cost[s_idx]=0; parent_idx[s_idx]=PARENT_NONE; MarkSeen(s_idx); OpenPush(s_idx,goal.x,goal.y);

    for(;;){
        do{cur=OpenPop(goal.x,goal.y); if(cur==PARENT_NONE) return next;}while(ClosedTest(cur));
        if(cur==g_idx){next=FirstStepFromParents(cur,start);break;}
        ClosedSet(cur); cx=x_of[cur]; cy=y_of[cur];
        for(k=0;k<8;k++){
            s32 dx=OFFS[k][0],dy=OFFS[k][1]; s32 nx=cx+dx,ny=cy+dy; u16 ni,tent;
            if(!InBounds(nx,ny)||!IsT1Tile(nx,ny)||!CanMoveDiagonalSafely(cx,cy,dx,dy)) continue;
            ni=Idx(nx,ny); if(ClosedTest(ni)) continue;
            tent=(u16)(g_cost[cur]+MOVE_COST[k]);
            if(!IsSeen(ni)){MarkSeen(ni);parent_idx[ni]=cur;g_cost[ni]=tent;OpenPush(ni,goal.x,goal.y);}
            else if(tent<g_cost[ni]){parent_idx[ni]=cur;g_cost[ni]=tent;OpenPush(ni,goal.x,goal.y);}
        }
    }
    return next;
}

s32 AStarGetDirection(DungeonPos cur,DungeonPos nxt){
    s32 dx=nxt.x-cur.x,dy=nxt.y-cur.y;
    if(dx==-1&&dy==-1)return DIRECTION_NORTHWEST;
    if(dx== 0&&dy==-1)return DIRECTION_NORTH;
    if(dx== 1&&dy==-1)return DIRECTION_NORTHEAST;
    if(dx==-1&&dy== 0)return DIRECTION_WEST;
    if(dx== 1&&dy== 0)return DIRECTION_EAST;
    if(dx==-1&&dy== 1)return DIRECTION_SOUTHWEST;
    if(dx== 0&&dy== 1)return DIRECTION_SOUTH;
    if(dx== 1&&dy== 1)return DIRECTION_SOUTHEAST;
    return -1;
}
