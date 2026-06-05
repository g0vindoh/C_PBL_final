#ifndef VEHICLE_H
#define VEHICLE_H

#include "utils.h"

/* ── Vehicle types ───────────────────────────────────────────────── */
typedef enum {
    VT_CAR=0, VT_BUS, VT_BIKE, VT_AMBULANCE, VT_POLICE,
    VT_COUNT
} VehicleType;

/* ── Vehicle state ───────────────────────────────────────────────── */
typedef enum {
    VS_MOVING=0, VS_WAITING, VS_STOPPED, VS_EMERGENCY, VS_CRASHED
} VehicleState;

/* ── Single vehicle ──────────────────────────────────────────────── */
typedef struct Vehicle {
    int          id;
    VehicleType  type;
    VehicleState state;
    Direction    dir;

    float  x, y;           /* centre position                  */
    float  vx, vy;         /* velocity                         */
    float  speed;          /* base speed                       */
    int    w, h;           /* sprite bounding box              */

    int    waiting_ticks;  /* frames spent at red              */
    int    active;         /* 1 = in simulation                */
    int    route_id;       /* which horizontal/vertical road   */

    /* visual */
    float  anim_phase;     /* for oscillation / lights         */
    Uint32 color_body;     /* packed RGBA body colour          */
    int    flash_state;    /* emergency flash toggle           */
    Uint32 flash_timer;

    struct Vehicle *next;  /* linked-list ptr                  */
} Vehicle;

/* ── Vehicle pool / list ─────────────────────────────────────────── */
typedef struct {
    Vehicle  pool[MAX_VEHICLES];
    Vehicle *head;          /* linked list of active vehicles   */
    int      count;
    int      total_spawned;
    int      next_id;
} VehicleList;

/* ── API ─────────────────────────────────────────────────────────── */
void     vehicle_list_init(VehicleList *vl);
Vehicle *vehicle_spawn(VehicleList *vl, VehicleType type, Direction dir, int route_id, float x, float y);
void     vehicle_remove(VehicleList *vl, Vehicle *v);
void     vehicle_update_all(VehicleList *vl, float dt);
void     vehicle_draw_all(SDL_Renderer *r, const VehicleList *vl, int night_mode);
void     vehicle_draw_one(SDL_Renderer *r, const Vehicle *v, int night_mode);
int      vehicle_count_by_type(const VehicleList *vl, VehicleType t);
int      vehicle_count_waiting(const VehicleList *vl);
float    vehicle_avg_wait(const VehicleList *vl);

#endif /* VEHICLE_H */
