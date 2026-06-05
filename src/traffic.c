/* traffic.c – Road network drawing, queue management, SimState init
 * ─────────────────────────────────────────────────────────────────── */
#include "../include/traffic.h"
#include <string.h>
#include <stdlib.h>

/* ════════════════════════════════════════════════════════════════════
   LANE QUEUE
   ════════════════════════════════════════════════════════════════════ */
void queue_push(LaneQueue *q, int vid) {
    QNode *n = (QNode *)malloc(sizeof(QNode));
    if (!n) return;
    n->vehicle_id = vid;
    n->next = NULL;
    if (!q->rear) { q->front = q->rear = n; }
    else          { q->rear->next = n; q->rear = n; }
    q->size++;
}

int queue_pop(LaneQueue *q) {
    if (!q->front) return -1;
    QNode *n = q->front;
    int vid = n->vehicle_id;
    q->front = n->next;
    if (!q->front) q->rear = NULL;
    free(n);
    q->size--;
    return vid;
}

void queue_clear(LaneQueue *q) {
    while (q->front) queue_pop(q);
    q->size = 0;
}

/* ════════════════════════════════════════════════════════════════════
   ROAD NETWORK INIT
   Six horizontal/vertical roads making a 3×2 grid
   ════════════════════════════════════════════════════════════════════ */
static void road_network_init(RoadNetwork *rn) {
    memset(rn, 0, sizeof(*rn));
    rn->road_count = 8;

    /* Horizontal roads */
    rn->roads[0] = (Road){0, 1, INT_Y1, 0,        WINDOW_W, 0};
    rn->roads[1] = (Road){1, 1, INT_Y4, 0,        WINDOW_W, 0};

    /* Vertical roads */
    rn->roads[2] = (Road){2, 0, INT_X1, 0,        WINDOW_H, 0};
    rn->roads[3] = (Road){3, 0, INT_X2, 0,        WINDOW_H, 0};
    rn->roads[4] = (Road){4, 0, INT_X3, 0,        WINDOW_H, 0};

    /* Extra diagonalish connectors (horizontal only) */
    rn->roads[5] = (Road){5, 1, 400,    INT_X1,   INT_X3,   0};
    rn->roads[6] = (Road){6, 1, 160,    0,        WINDOW_W, 0};
    rn->roads[7] = (Road){7, 1, 640,    0,        WINDOW_W, 0};
}

/* ════════════════════════════════════════════════════════════════════
   FULL SIM STATE INIT / RESET
   ════════════════════════════════════════════════════════════════════ */
void sim_init(SimState *s) {
    memset(s, 0, sizeof(*s));
    road_network_init(&s->network);
    vehicle_list_init(&s->vehicles);
    signal_array_init(&s->signals);
    accident_array_init(&s->accidents);

    s->paused         = 0;
    s->night_mode     = 0;
    s->density_factor = 1.0f;
    s->last_spawn     = 0;
    s->sim_start      = get_ticks();
    s->fps            = 0;

    /* Rain drops initial random positions */
    for (int i = 0; i < 300; i++) {
        s->rain_drops[i][0] = (float)(rand() % WINDOW_W);
        s->rain_drops[i][1] = (float)(rand() % WINDOW_H);
        s->rain_drops[i][2] = (float)(RAND_RANGE(-2, 2));
        s->rain_drops[i][3] = (float)(RAND_RANGE(8, 18));
    }

    log_event("Simulation initialised.");
}

void sim_reset(SimState *s) {
    /* Free all queues */
    for (int i = 0; i < MAX_ROADS*2; i++)
        queue_clear(&s->network.lane_queues[i]);

    /* Re-init everything */
    sim_init(s);
    log_event("Simulation reset.");
}

/* ════════════════════════════════════════════════════════════════════
   SPAWN POINTS  (8 entry points around screen edge)
   ════════════════════════════════════════════════════════════════════ */
typedef struct { float x, y; Direction dir; int route; } SpawnPoint;

static const SpawnPoint SPAWNS[] = {
    /* Horizontal entries */
    {   0, INT_Y1, DIR_RIGHT, 0 },
    {WINDOW_W, INT_Y1, DIR_LEFT,  0 },
    {   0, INT_Y4, DIR_RIGHT, 1 },
    {WINDOW_W, INT_Y4, DIR_LEFT,  1 },
    {   0, 160,    DIR_RIGHT, 6 },
    {WINDOW_W, 160,   DIR_LEFT,  6 },
    {   0, 640,    DIR_RIGHT, 7 },
    {WINDOW_W, 640,   DIR_LEFT,  7 },
    /* Vertical entries */
    {INT_X1,  0,        DIR_DOWN, 2 },
    {INT_X1, WINDOW_H,  DIR_UP,   2 },
    {INT_X2,  0,        DIR_DOWN, 3 },
    {INT_X2, WINDOW_H,  DIR_UP,   3 },
    {INT_X3,  0,        DIR_DOWN, 4 },
    {INT_X3, WINDOW_H,  DIR_UP,   4 },
};
#define NUM_SPAWNS (int)(sizeof(SPAWNS)/sizeof(SPAWNS[0]))

/* Choose vehicle type weighted by density */
static VehicleType pick_type(float density) {
    (void)density;
    int r = rand() % 100;
    if (r < 5)  return VT_BUS;
    if (r < 12) return VT_BIKE;
    return VT_CAR;
}

/* ── Trigger an accident at a random road position ───────────────── */
void sim_trigger_accident(SimState *s) {
    /* Pick random position on a road */
    int r = rand() % 4;
    float x, y;
    switch(r) {
        case 0: x = RAND_RANGE(100, WINDOW_W-100); y = INT_Y1; break;
        case 1: x = RAND_RANGE(100, WINDOW_W-100); y = INT_Y4; break;
        case 2: x = INT_X1; y = RAND_RANGE(100, WINDOW_H-100); break;
        default:x = INT_X2; y = RAND_RANGE(100, WINDOW_H-100); break;
    }
    accident_trigger(&s->accidents, x, y);
}

/* ── Spawn emergency vehicle ─────────────────────────────────────── */
void sim_spawn_emergency(SimState *s) {
    int idx = rand() % NUM_SPAWNS;
    SpawnPoint sp = SPAWNS[idx];
    VehicleType t = (rand() % 2 == 0) ? VT_AMBULANCE : VT_POLICE;
    Vehicle *v = vehicle_spawn(&s->vehicles, t, sp.dir, sp.route, sp.x, sp.y);
    if (v) {
        v->state = VS_EMERGENCY;
        s->total_emergency++;
        log_event("Emergency vehicle spawned at (%.0f,%.0f)", sp.x, sp.y);
    }
}

/* ════════════════════════════════════════════════════════════════════
   UPDATE  (called every tick while not paused)
   ════════════════════════════════════════════════════════════════════ */
void sim_update(SimState *s) {
    if (!s || s->paused) return;
    s->total_ticks++;

    float dt = 1.0f; /* fixed-step at 60fps */

    /* ── Spawn new vehicles ───────────────────────────────────────── */
    Uint32 now = get_ticks();
    Uint32 interval = (Uint32)(SPAWN_INTERVAL_MS / s->density_factor);
    if (now - s->last_spawn > interval && s->vehicles.count < MAX_VEHICLES - 5) {
        int idx = rand() % NUM_SPAWNS;
        SpawnPoint sp = SPAWNS[idx];
        VehicleType t = pick_type(s->density_factor);
        vehicle_spawn(&s->vehicles, t, sp.dir, sp.route, sp.x, sp.y);
        s->last_spawn = now;
    }

    /* ── Compute per-intersection queue lengths ──────────────────── */
    int qlens[MAX_SIGNALS] = {0};
    for (const Vehicle *v = s->vehicles.head; v; v = v->next) {
        if (v->state != VS_WAITING) continue;
        /* Find nearest intersection */
        float best_d2 = 9999999.f;
        int   best_i  = -1;
        for (int i = 0; i < s->signals.count; i++) {
            float dx = v->x - s->signals.signals[i].ix;
            float dy = v->y - s->signals.signals[i].iy;
            float d2 = dx*dx + dy*dy;
            if (d2 < best_d2) { best_d2 = d2; best_i = i; }
        }
        if (best_i >= 0 && best_d2 < 200.0f*200.0f) qlens[best_i]++;
    }

    /* ── Update signals ──────────────────────────────────────────── */
    signal_update_all(&s->signals, qlens);

    /* ── Emergency vehicle signal override ───────────────────────── */
    for (Vehicle *v = s->vehicles.head; v; v = v->next) {
        if (v->type == VT_AMBULANCE || v->type == VT_POLICE) {
            signal_force_green(&s->signals, v->x, v->y, 200.0f);
        }
    }

    /* ── Vehicle AI ──────────────────────────────────────────────── */
    for (Vehicle *v = s->vehicles.head; v; v = v->next) {
        if (!v->active) continue;

        /* Skip crash victims */
        if (v->state == VS_CRASHED) continue;

        /* Emergency vehicles ignore signals */
        if (v->type == VT_AMBULANCE || v->type == VT_POLICE) {
            v->state = VS_EMERGENCY;
            v->vx = (v->dir == DIR_RIGHT) ?  v->speed :
                    (v->dir == DIR_LEFT)  ? -v->speed : 0;
            v->vy = (v->dir == DIR_DOWN)  ?  v->speed :
                    (v->dir == DIR_UP)    ? -v->speed : 0;
            continue;
        }

        /* Check accident blocking */
        if (accident_blocks_pos(&s->accidents, v->x + v->vx * 40,
                                               v->y + v->vy * 40, 50.0f)) {
            v->state = VS_WAITING;
            v->vx = v->vy = 0;
            continue;
        }

        /* Check red signal */
        int red = signal_is_red_at(&s->signals, v->x, v->y, v->dir);

        /* Check leading vehicle (simple follow logic) */
        int blocked_by_vehicle = 0;
        for (const Vehicle *other = s->vehicles.head; other; other = other->next) {
            if (other == v || !other->active) continue;
            if (other->dir != v->dir) continue;
            float dx = other->x - v->x, dy = other->y - v->y;
            float ahead_dist;
            int same_lane = 0;
            switch(v->dir) {
                case DIR_RIGHT:
                    if (ABS((int)dy) < 20 && dx > 0 && dx < SAFE_DIST + v->w) {
                        same_lane = 1; ahead_dist = dx;
                    }
                    break;
                case DIR_LEFT:
                    if (ABS((int)dy) < 20 && dx < 0 && -dx < SAFE_DIST + v->w) {
                        same_lane = 1; ahead_dist = -dx;
                    }
                    break;
                case DIR_DOWN:
                    if (ABS((int)dx) < 20 && dy > 0 && dy < SAFE_DIST + v->h) {
                        same_lane = 1; ahead_dist = dy;
                    }
                    break;
                case DIR_UP:
                    if (ABS((int)dx) < 20 && dy < 0 && -dy < SAFE_DIST + v->h) {
                        same_lane = 1; ahead_dist = -dy;
                    }
                    break;
            }
            if (same_lane) { blocked_by_vehicle = 1; break; }
            (void)ahead_dist;
        }

        /* Apply state */
        if (red || blocked_by_vehicle) {
            v->state = VS_WAITING;
            v->vx = v->vy = 0;
        } else {
            v->state = VS_MOVING;
            switch(v->dir) {
                case DIR_RIGHT: v->vx =  v->speed; v->vy = 0; break;
                case DIR_LEFT:  v->vx = -v->speed; v->vy = 0; break;
                case DIR_DOWN:  v->vx = 0; v->vy =  v->speed; break;
                case DIR_UP:    v->vx = 0; v->vy = -v->speed; break;
            }
        }
    }

    /* ── Move vehicles ───────────────────────────────────────────── */
    vehicle_update_all(&s->vehicles, dt);

    /* ── Accidents ───────────────────────────────────────────────── */
    accident_update_all(&s->accidents);

    /* ── Random accident (rare) ──────────────────────────────────── */
    if (s->total_ticks % (60 * 40) == 0 && s->accidents.count < MAX_ACCIDENTS)
        sim_trigger_accident(s);

    /* ── Update per-road density ─────────────────────────────────── */
    for (int i = 0; i < s->network.road_count; i++) {
        Road *road = &s->network.roads[i];
        int cnt = 0;
        for (const Vehicle *v = s->vehicles.head; v; v = v->next) {
            int on_road = 0;
            if (road->is_horizontal) {
                on_road = (ABS((int)v->y - road->y_or_x) < ROAD_W/2 + 5);
            } else {
                on_road = (ABS((int)v->x - road->y_or_x) < ROAD_W/2 + 5);
            }
            if (on_road) cnt++;
        }
        road->density = CLAMP(cnt / 20.0f, 0.0f, 1.0f);
    }

    /* ── Stats ───────────────────────────────────────────────────── */
    s->total_vehicles  = s->vehicles.total_spawned;
    s->total_accidents = s->accidents.total_occurred;
    s->avg_wait_time   = vehicle_avg_wait(&s->vehicles);
}

/* ════════════════════════════════════════════════════════════════════
   ROAD RENDERING
   ════════════════════════════════════════════════════════════════════ */

/* Density colour: green → yellow → red */
static SDL_Color density_color(float d, int night) {
    Uint8 r, g, b;
    if (d < 0.5f) {
        float t = d * 2.0f;
        r = (Uint8)(20 + t * 220);
        g = (Uint8)(200 - t * 50);
        b = (Uint8)(60);
    } else {
        float t = (d - 0.5f) * 2.0f;
        r = (Uint8)(240);
        g = (Uint8)(150 - t * 130);
        b = (Uint8)(40);
    }
    Uint8 alpha = night ? 140 : 100;
    return (SDL_Color){r, g, b, alpha};
}

void road_draw_heatmap(SDL_Renderer *r, const Road *road, float density, int night) {
    if (density < 0.05f) return; /* nothing to show */
    SDL_Color dc = density_color(density, night);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    if (road->is_horizontal) {
        SDL_Rect rect = { road->start, road->y_or_x - ROAD_W/2,
                          road->end - road->start, ROAD_W };
        SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a);
        SDL_RenderFillRect(r, &rect);
    } else {
        SDL_Rect rect = { road->y_or_x - ROAD_W/2, road->start,
                          ROAD_W, road->end - road->start };
        SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a);
        SDL_RenderFillRect(r, &rect);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

/* Draw road surface + lane markings */
static void draw_road(SDL_Renderer *r, const Road *road, int night) {
    SDL_Color road_col = night ? (SDL_Color)COL_ROAD_NIGHT : (SDL_Color)COL_ROAD_DAY;
    SDL_Color mark_col = (SDL_Color)COL_LANE_MARK;

    if (road->is_horizontal) {
        /* Road surface */
        draw_rect_filled(r, road->start, road->y_or_x - ROAD_W/2,
                         road->end - road->start, ROAD_W, road_col);
        /* Centre dashed line */
        int y_centre = road->y_or_x;
        for (int x = road->start; x < road->end; x += 24) {
            draw_rect_filled(r, x, y_centre - 1, 14, 2, mark_col);
        }
        /* Edge lines */
        SDL_Color edge = {70, 75, 90, 255};
        draw_rect_filled(r, road->start, road->y_or_x - ROAD_W/2,
                         road->end - road->start, 2, edge);
        draw_rect_filled(r, road->start, road->y_or_x + ROAD_W/2 - 2,
                         road->end - road->start, 2, edge);
    } else {
        /* Road surface */
        draw_rect_filled(r, road->y_or_x - ROAD_W/2, road->start,
                         ROAD_W, road->end - road->start, road_col);
        /* Centre dashed line */
        int x_centre = road->y_or_x;
        for (int y = road->start; y < road->end; y += 24) {
            draw_rect_filled(r, x_centre - 1, y, 2, 14, mark_col);
        }
        /* Edge lines */
        SDL_Color edge = {70, 75, 90, 255};
        draw_rect_filled(r, road->y_or_x - ROAD_W/2, road->start, 2,
                         road->end - road->start, edge);
        draw_rect_filled(r, road->y_or_x + ROAD_W/2 - 2, road->start, 2,
                         road->end - road->start, edge);
    }
}

/* Draw intersection box */
static void draw_intersection(SDL_Renderer *r, int ix, int iy, int night) {
    SDL_Color col = night ? (SDL_Color){35, 38, 55, 255} : (SDL_Color){50, 55, 75, 255};
    int half = ROAD_W / 2;
    draw_rect_filled(r, ix - half, iy - half, ROAD_W, ROAD_W, col);

    /* Zebra crossing marks */
    SDL_Color zebra = {80, 85, 100, 180};
    for (int i = 0; i < 4; i++) {
        draw_rect_filled(r, ix - half + i*9, iy - half,     6, 10, zebra);
        draw_rect_filled(r, ix - half + i*9, iy + half - 10, 6, 10, zebra);
        draw_rect_filled(r, ix - half,      iy - half + i*9, 10, 6, zebra);
        draw_rect_filled(r, ix + half - 10, iy - half + i*9, 10, 6, zebra);
    }
}

void road_network_draw(SDL_Renderer *r, const RoadNetwork *rn,
                       int night, const AccidentArray *aa) {
    (void)aa;
    /* Roads */
    for (int i = 0; i < rn->road_count; i++) {
        draw_road(r, &rn->roads[i], night);
    }

    /* Intersections */
    int ix[MAX_SIGNALS] = {INT_X1,INT_X2,INT_X3,INT_X4,INT_X5,INT_X6};
    int iy[MAX_SIGNALS] = {INT_Y1,INT_Y2,INT_Y3,INT_Y4,INT_Y5,INT_Y6};
    for (int i = 0; i < MAX_SIGNALS; i++)
        draw_intersection(r, ix[i], iy[i], night);

    /* Heatmap overlays */
    for (int i = 0; i < rn->road_count; i++) {
        road_draw_heatmap(r, &rn->roads[i], rn->roads[i].density, night);
    }
}
