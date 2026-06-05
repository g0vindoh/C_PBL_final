/* simulation.c – Input handler + full render pipeline
 * ──────────────────────────────────────────────────── */
#include "../include/simulation.h"
#include "../include/ui.h"
#include <math.h>

/* ── Background city grid ────────────────────────────────────────── */
static void draw_city_bg(SDL_Renderer *r, int night, Uint32 ticks) {
    SDL_Color bg = night ? (SDL_Color)COL_BG_NIGHT : (SDL_Color)COL_BG_DAY;
    SDL_SetRenderDrawColor(r, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(r);

    /* City grid dots */
    SDL_Color grid_col = night ? (SDL_Color){14, 18, 32, 255}
                               : (SDL_Color){22, 26, 42, 255};
    for (int x = 0; x < WINDOW_W; x += 40) {
        for (int y = 0; y < WINDOW_H; y += 40) {
            draw_rect_filled(r, x, y, 1, 1, grid_col);
        }
    }

    /* City block buildings (static decorative rects) */
    static const struct { int x, y, w, h; } blocks[] = {
        { 10,  10, 100, 100}, {150,  10,  80,  60}, {260,  10, 50,  80},
        {380,  10,  80,  60}, {500,  10, 100, 100}, {680,  10, 60,  80},
        {760,  10,  80,  50}, {880,  10,  60, 100}, { 10, 340, 80,  80},
        {150, 330,  60,  60}, { 10, 460, 100,  80}, {150, 460, 80,  60},
        {380, 310,  60,  60}, {500, 310,  80,  60},
        { 10, 580, 100,  80}, {150, 590,  80,  60}, {380, 600, 100,  60},
        {500, 600,  60,  60}, {680, 600,  80,  60}, {760, 590,  60,  80},
    };
    int num_blocks = sizeof(blocks)/sizeof(blocks[0]);
    float pulse = 0.5f + 0.5f * sinf((float)ticks / 3000.0f);

    for (int i = 0; i < num_blocks; i++) {
        /* Building colour cycles slightly between day/night */
        Uint8 base = night ? 18 : 28;
        Uint8 r_val = base + (Uint8)(4 * (i % 5));
        Uint8 g_val = base + (Uint8)(3 * (i % 4));
        Uint8 b_val = base + (Uint8)(8 * (i % 3));
        SDL_Color bld = {r_val, g_val, b_val, 255};
        draw_rect_filled(r, blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h, bld);

        /* Windows */
        SDL_Color win_col;
        if (night) {
            Uint8 lit = (Uint8)(180 + pulse * 40);
            win_col = (i % 3 == 0) ? (SDL_Color){lit, lit-20, 40, 200}  /* warm */
                    : (i % 3 == 1) ? (SDL_Color){40, 80, lit, 160}       /* cool */
                    : (SDL_Color){0,0,0,0};                               /* off  */
        } else {
            win_col = (SDL_Color){140, 160, 200, 80};
        }
        if (win_col.a > 0) {
            for (int wy = blocks[i].y + 8; wy < blocks[i].y + blocks[i].h - 8; wy += 14) {
                for (int wx = blocks[i].x + 6; wx < blocks[i].x + blocks[i].w - 6; wx += 12) {
                    draw_rect_filled(r, wx, wy, 6, 8, win_col);
                }
            }
        }

        /* Building outline */
        SDL_Color outline = night ? (SDL_Color){30, 35, 55, 200} : (SDL_Color){38, 42, 62, 200};
        draw_rect_outline(r, blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h, outline, 1);
    }

    /* Stars (night only) */
    if (night) {
        srand(42); /* deterministic stars */
        SDL_Color star = {200, 210, 255, (Uint8)(100 + pulse * 80)};
        for (int i = 0; i < 60; i++) {
            int sx = rand() % WINDOW_W;
            int sy = rand() % (WINDOW_H / 3);
            draw_rect_filled(r, sx, sy, 1 + (i % 2), 1 + (i % 2), star);
        }
        srand((unsigned)time(NULL)); /* restore random */
    }
}

/* ── FPS tracking ─────────────────────────────────────────────────── */
static void update_fps(SimState *s) {
    s->fps_frames++;
    Uint32 now = get_ticks();
    if (now - s->fps_timer >= 1000) {
        s->fps = s->fps_frames;
        s->fps_frames = 0;
        s->fps_timer = now;
    }
}

/* ════════════════════════════════════════════════════════════════════
   INPUT HANDLER
   ════════════════════════════════════════════════════════════════════ */
void simulation_handle_input(SimState *s, SDL_Event *e) {
    if (e->type != SDL_KEYDOWN) return;
    SDL_Keycode k = e->key.keysym.sym;

    switch (k) {
        case SDLK_SPACE:
            s->paused ^= 1;
            log_event("Simulation %s.", s->paused ? "paused" : "resumed");
            break;

        case SDLK_UP:
            s->density_factor = CLAMP(s->density_factor + 0.25f, 0.5f, 3.0f);
            log_event("Density factor -> %.2f", s->density_factor);
            break;

        case SDLK_DOWN:
            s->density_factor = CLAMP(s->density_factor - 0.25f, 0.5f, 3.0f);
            log_event("Density factor -> %.2f", s->density_factor);
            break;

        case SDLK_a:
            if (s->accidents.count < MAX_ACCIDENTS) {
                sim_trigger_accident(s);
            }
            break;

        case SDLK_e:
            sim_spawn_emergency(s);
            break;

        case SDLK_n:
            s->night_mode ^= 1;
            log_event("Night mode %s.", s->night_mode ? "on" : "off");
            break;

        case SDLK_r:
            s->rain_enabled ^= 1;
            log_event("Rain %s.", s->rain_enabled ? "on" : "off");
            break;

        case SDLK_F5:
            sim_reset(s);
            break;

        default: break;
    }
}

/* ════════════════════════════════════════════════════════════════════
   STEP  (thin wrapper – real logic in sim_update)
   ════════════════════════════════════════════════════════════════════ */
void simulation_step(SimState *s, float dt) {
    if (s->rain_enabled) ui_update_rain(s, dt);
    sim_update(s);
    update_fps(s);
}

/* ════════════════════════════════════════════════════════════════════
   DRAW
   ════════════════════════════════════════════════════════════════════ */
void simulation_draw(SDL_Renderer *r, SimState *s) {
    Uint32 ticks = get_ticks();

    /* 1. City background */
    draw_city_bg(r, s->night_mode, ticks);

    /* 2. Roads + intersections + heatmap */
    road_network_draw(r, &s->network, s->night_mode, &s->accidents);

    /* 3. Signals */
    signal_draw_all(r, &s->signals);

    /* 4. Accidents */
    accident_draw_all(r, &s->accidents);

    /* 5. Vehicles */
    vehicle_draw_all(r, &s->vehicles, s->night_mode);

    /* 6. Rain (above vehicles for atmosphere) */
    ui_draw_rain(r, s);

    /* 7. Dashboard */
    ui_draw_dashboard(r, s);

    /* 8. Density indicator */
    ui_draw_density_indicator(r, s->density_factor);

    /* 9. Paused overlay */
    if (s->paused) ui_draw_paused_overlay(r);

    /* 10. Key help bar */
    ui_draw_keybind_help(r);
}
