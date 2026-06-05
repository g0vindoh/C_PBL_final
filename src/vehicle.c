/* vehicle.c – Vehicle spawning, movement, rendering
 * ─────────────────────────────────────────────────── */
#include "../include/vehicle.h"
#include <string.h>

/* ── Dimensions per vehicle type ─────────────────────────────────── */
static const int VW[VT_COUNT] = {24, 38, 16, 28, 26};
static const int VH[VT_COUNT] = {14, 20, 10, 14, 14};

/* ── Base colours (packed RGBA) per type ─────────────────────────── */
static SDL_Color veh_color(VehicleType t) {
    switch(t) {
        case VT_CAR:       return (SDL_Color){RAND_RANGE(120,255), RAND_RANGE(60,200), RAND_RANGE(40,180), 255};
        case VT_BUS:       return (SDL_Color){  20, 140, 230, 255};
        case VT_BIKE:      return (SDL_Color){ 220, 180,  40, 255};
        case VT_AMBULANCE: return (SDL_Color){ 240, 240, 240, 255};
        case VT_POLICE:    return (SDL_Color){  30,  80, 200, 255};
        default:           return (SDL_Color){ 200, 200, 200, 255};
    }
}

/* ── Init pool ────────────────────────────────────────────────────── */
void vehicle_list_init(VehicleList *vl) {
    memset(vl, 0, sizeof(*vl));
    for (int i = 0; i < MAX_VEHICLES; i++)
        vl->pool[i].active = 0;
    vl->head = NULL;
    vl->count = 0;
    vl->next_id = 1;
}

/* ── Spawn ────────────────────────────────────────────────────────── */
Vehicle *vehicle_spawn(VehicleList *vl, VehicleType type, Direction dir,
                       int route_id, float x, float y) {
    /* find free slot */
    Vehicle *v = NULL;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (!vl->pool[i].active) { v = &vl->pool[i]; break; }
    }
    if (!v || vl->count >= MAX_VEHICLES) {
        log_event("WARNING: vehicle pool full (%d/%d), spawn dropped.", vl->count, MAX_VEHICLES);
        return NULL;
    }

    memset(v, 0, sizeof(*v));
    v->id       = vl->next_id++;
    v->type     = type;
    v->dir      = dir;
    v->route_id = route_id;
    v->x        = x;
    v->y        = y;
    v->w        = VW[type];
    v->h        = VH[type];
    v->state    = VS_MOVING;
    v->active   = 1;
    v->anim_phase = (float)(rand() % 628) / 100.0f;

    /* speed */
    float spd = BASE_SPEED;
    if (type == VT_BUS)        spd = 1.4f;
    if (type == VT_BIKE)       spd = 2.8f;
    if (type == VT_AMBULANCE)  spd = 3.5f;
    if (type == VT_POLICE)     spd = 3.2f;
    v->speed = spd;

    /* velocity by direction */
    switch(dir) {
        case DIR_RIGHT: v->vx =  v->speed; v->vy = 0; break;
        case DIR_LEFT:  v->vx = -v->speed; v->vy = 0; break;
        case DIR_DOWN:  v->vx = 0; v->vy =  v->speed; break;
        case DIR_UP:    v->vx = 0; v->vy = -v->speed; break;
    }

    SDL_Color c = veh_color(type);
    v->color_body = ((Uint32)c.r << 24) | ((Uint32)c.g << 16) |
                    ((Uint32)c.b << 8)  | c.a;

    /* prepend to list */
    v->next = vl->head;
    vl->head = v;
    vl->count++;
    vl->total_spawned++;
    return v;
}

/* ── Remove from list ─────────────────────────────────────────────── */
void vehicle_remove(VehicleList *vl, Vehicle *v) {
    if (!vl || !v) return;
    Vehicle **pp = &vl->head;
    while (*pp) {
        if (*pp == v) { *pp = v->next; break; }
        pp = &(*pp)->next;
    }
    v->next   = NULL; /* prevent stale traversal from any lingering reference */
    v->active = 0;
    vl->count--;
}

/* ── Stop-line check helpers (used internally) ────────────────────── */
/* Returns 1 if vehicle is too far off screen and should be removed */
static int out_of_bounds(const Vehicle *v) {
    return (v->x < -100 || v->x > WINDOW_W + 100 ||
            v->y < -100 || v->y > WINDOW_H + 100);
}

/* ── Update all vehicles ──────────────────────────────────────────── */
void vehicle_update_all(VehicleList *vl, float dt) {
    Vehicle *v = vl->head;
    while (v) {
        Vehicle *next = v->next;
        v->anim_phase += 2.0f * dt;
        if (v->anim_phase > 6.2832f) v->anim_phase -= 6.2832f;

        /* Emergency flash */
        if (v->type == VT_AMBULANCE || v->type == VT_POLICE) {
            if (get_ticks() - v->flash_timer > 200) {
                v->flash_state ^= 1;
                v->flash_timer = get_ticks();
            }
        }

        /* Move if not stopped/waiting */
        if (v->state == VS_MOVING || v->state == VS_EMERGENCY) {
            v->x += v->vx;
            v->y += v->vy;
        }
        if (v->state == VS_WAITING) {
            v->waiting_ticks++;
        }

        /* Remove when off screen */
        if (out_of_bounds(v)) {
            vehicle_remove(vl, v);
        }
        v = next;
    }
}

/* ── Draw helpers ─────────────────────────────────────────────────── */

/* Unpack colour from Uint32 */
static SDL_Color unpack_color(Uint32 c) {
    return (SDL_Color){
        (Uint8)((c>>24)&0xFF),
        (Uint8)((c>>16)&0xFF),
        (Uint8)((c>> 8)&0xFF),
        (Uint8)( c     &0xFF)
    };
}

/* Dim a colour */
static SDL_Color dim_color(SDL_Color c, float factor) {
    return (SDL_Color){
        (Uint8)(c.r * factor),
        (Uint8)(c.g * factor),
        (Uint8)(c.b * factor),
        c.a
    };
}

/* Rotate rect drawing for direction */
static void draw_vehicle_body(SDL_Renderer *r, const Vehicle *v, SDL_Color body, int night) {
    int bx = (int)(v->x - v->w/2);
    int by = (int)(v->y - v->h/2);
    int bw = v->w;
    int bh = v->h;

    /* Swap width/height for vertical direction */
    if (v->dir == DIR_DOWN || v->dir == DIR_UP) {
        int tmp = bw; bw = bh; bh = tmp;
        bx = (int)(v->x - bw/2);
        by = (int)(v->y - bh/2);
    }

    /* Shadow */
    SDL_Color shadow = {0, 0, 0, 80};
    draw_rect_filled(r, bx+3, by+3, bw, bh, shadow);

    /* Body */
    draw_rounded_rect(r, bx, by, bw, bh, 3, body);

    /* Roof / cabin (cars & police have distinct cabin) */
    if (v->type == VT_CAR || v->type == VT_POLICE) {
        SDL_Color roof = dim_color(body, 0.7f);
        int rw = bw * 5/8, rh = bh * 6/8;
        int rx = bx + (bw - rw)/2, ry = by + (bh - rh)/2;
        draw_rounded_rect(r, rx, ry, rw, rh, 2, roof);
    }

    /* Ambulance cross */
    if (v->type == VT_AMBULANCE) {
        SDL_Color cross = {220, 40, 40, 255};
        draw_rect_filled(r, bx + bw/2 - 2, by + 2, 4, bh-4, cross);
        draw_rect_filled(r, bx + 2, by + bh/2 - 2, bw-4, 4, cross);
    }

    /* Bus windows */
    if (v->type == VT_BUS) {
        SDL_Color win = {180, 220, 255, 180};
        int step = bw / 4;
        for (int i = 1; i < 4; i++)
            draw_rect_filled(r, bx + i*step - 2, by + 2, 4, bh-4, win);
    }

    /* Headlights */
    SDL_Color hl = night ? (SDL_Color){255, 255, 180, 255} : (SDL_Color){255, 255, 200, 120};
    if (v->dir == DIR_RIGHT) {
        draw_circle_filled(r, bx+bw-2, by+3, 3, hl);
        draw_circle_filled(r, bx+bw-2, by+bh-3, 3, hl);
        if (night) {
            SDL_Color glow = {255,255,180,60};
            draw_glow_circle(r, bx+bw, by+3, 3, glow, 2);
            draw_glow_circle(r, bx+bw, by+bh-3, 3, glow, 2);
        }
    } else if (v->dir == DIR_LEFT) {
        draw_circle_filled(r, bx+2, by+3, 3, hl);
        draw_circle_filled(r, bx+2, by+bh-3, 3, hl);
    } else if (v->dir == DIR_DOWN) {
        draw_circle_filled(r, bx+3, by+bh-2, 3, hl);
        draw_circle_filled(r, bx+bw-3, by+bh-2, 3, hl);
    } else {
        draw_circle_filled(r, bx+3, by+2, 3, hl);
        draw_circle_filled(r, bx+bw-3, by+2, 3, hl);
    }

    /* Emergency lights */
    if (v->type == VT_AMBULANCE || v->type == VT_POLICE) {
        SDL_Color el1 = v->flash_state ? (SDL_Color){255, 0, 0, 255} : (SDL_Color){0, 0, 255, 255};
        SDL_Color el2 = v->flash_state ? (SDL_Color){0, 0, 255, 255} : (SDL_Color){255, 0, 0, 255};
        if (v->dir == DIR_RIGHT || v->dir == DIR_LEFT) {
            draw_circle_filled(r, bx + bw/4,     by - 3, 3, el1);
            draw_circle_filled(r, bx + 3*bw/4,   by - 3, 3, el2);
            if (v->flash_state) {
                draw_glow_circle(r, bx+bw/4,   by-3, 3, el1, 2);
                draw_glow_circle(r, bx+3*bw/4, by-3, 3, el2, 2);
            }
        } else {
            draw_circle_filled(r, bx - 3, by + bh/4,   3, el1);
            draw_circle_filled(r, bx - 3, by + 3*bh/4, 3, el2);
        }
    }

    /* Taillights */
    SDL_Color tl = {200, 40, 40, 200};
    if (v->dir == DIR_LEFT) {
        draw_circle_filled(r, bx+bw-2, by+3, 2, tl);
        draw_circle_filled(r, bx+bw-2, by+bh-3, 2, tl);
    } else if (v->dir == DIR_RIGHT) {
        draw_circle_filled(r, bx+2, by+3, 2, tl);
        draw_circle_filled(r, bx+2, by+bh-3, 2, tl);
    }
}

/* ── Draw one vehicle ─────────────────────────────────────────────── */
void vehicle_draw_one(SDL_Renderer *r, const Vehicle *v, int night) {
    SDL_Color body = unpack_color(v->color_body);

    /* Dim stopped vehicles */
    if (v->state == VS_WAITING || v->state == VS_STOPPED) {
        body = dim_color(body, 0.75f);
    }

    draw_vehicle_body(r, v, body, night);

    /* Waiting bar above vehicle */
    if (v->state == VS_WAITING && v->waiting_ticks > 30) {
        int wt = MIN(v->waiting_ticks, 300);
        float pct = wt / 300.0f;
        SDL_Color bar_bg  = {40, 40, 40, 200};
        SDL_Color bar_fg  = pct < 0.5f ? (SDL_Color){80, 200, 80, 255}
                                       : (SDL_Color){220, 80, 40, 255};
        int bx = (int)v->x - 14, by2 = (int)v->y - (v->h/2) - 7;
        draw_rect_filled(r, bx, by2, 28, 4, bar_bg);
        draw_rect_filled(r, bx, by2, (int)(28*pct), 4, bar_fg);
    }
}

/* ── Draw all vehicles ────────────────────────────────────────────── */
void vehicle_draw_all(SDL_Renderer *r, const VehicleList *vl, int night) {
    for (const Vehicle *v = vl->head; v; v = v->next)
        vehicle_draw_one(r, v, night);
}

/* ── Stats helpers ────────────────────────────────────────────────── */
int vehicle_count_by_type(const VehicleList *vl, VehicleType t) {
    int n = 0;
    for (const Vehicle *v = vl->head; v; v = v->next)
        if (v->type == t) n++;
    return n;
}

int vehicle_count_waiting(const VehicleList *vl) {
    int n = 0;
    for (const Vehicle *v = vl->head; v; v = v->next)
        if (v->state == VS_WAITING) n++;
    return n;
}

float vehicle_avg_wait(const VehicleList *vl) {
    float total = 0;
    int n = 0;
    for (const Vehicle *v = vl->head; v; v = v->next) {
        if (v->waiting_ticks > 0) { total += v->waiting_ticks; n++; }
    }
    return n > 0 ? total / n / 60.0f : 0.0f; /* seconds at 60fps */
}
