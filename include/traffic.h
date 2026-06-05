#ifndef TRAFFIC_H
#define TRAFFIC_H

#include "utils.h"
#include "vehicle.h"
#include "signal.h"
#include "accident.h"

/* ── Road descriptor ─────────────────────────────────────────────── */
typedef struct {
    int   id;
    int   is_horizontal;   /* 1=H, 0=V                         */
    int   y_or_x;          /* fixed axis coordinate            */
    int   start, end;      /* range on moving axis             */
    float density;         /* 0–1 congestion level             */
} Road;

/* ── Queue node (per-lane waiting queue) ─────────────────────────── */
typedef struct QNode {
    int          vehicle_id;
    struct QNode *next;
} QNode;

typedef struct {
    QNode *front, *rear;
    int    size;
} LaneQueue;

/* ── Top-level road network ──────────────────────────────────────── */
#define MAX_ROADS 10
typedef struct {
    Road      roads[MAX_ROADS];
    int       road_count;
    LaneQueue lane_queues[MAX_ROADS * 2]; /* each road has 2 lanes  */
} RoadNetwork;

/* ── Full simulation state ───────────────────────────────────────── */
typedef struct {
    RoadNetwork  network;
    VehicleList  vehicles;
    SignalArray  signals;
    AccidentArray accidents;

    int    paused;
    int    night_mode;
    float  density_factor;   /* 1.0 = normal, 2.0 = double       */
    Uint32 last_spawn;
    Uint32 sim_start;
    Uint64 total_ticks;
    int    fps;
    float  fps_acc;
    int    fps_frames;
    Uint32 fps_timer;

    /* rain */
    int    rain_enabled;
    float  rain_drops[300][4]; /* x,y,vx,vy                       */

    /* stats */
    int    total_vehicles;
    int    total_accidents;
    int    total_emergency;
    float  avg_wait_time;
} SimState;

/* ── API ─────────────────────────────────────────────────────────── */
void sim_init(SimState *s);
void sim_update(SimState *s);
void sim_reset(SimState *s);
void sim_trigger_accident(SimState *s);
void sim_spawn_emergency(SimState *s);
void road_network_draw(SDL_Renderer *r, const RoadNetwork *rn, int night_mode, const AccidentArray *aa);
void road_draw_heatmap(SDL_Renderer *r, const Road *road, float density, int night_mode);
int  queue_push(LaneQueue *q, int vid);  /* returns 1 on success, 0 on malloc fail */
int  queue_pop(LaneQueue *q);
void queue_clear(LaneQueue *q);

#endif /* TRAFFIC_H */
