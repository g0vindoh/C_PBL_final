/* ui.c – Dashboard, HUD, loading screen, rain, overlays
 * ─────────────────────────────────────────────────────── */
#include "../include/ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ════════════════════════════════════════════════════════════════════
   LOADING SCREEN
   ════════════════════════════════════════════════════════════════════ */
void ui_draw_loading(SDL_Renderer *r, float progress, Uint32 ticks) {
    /* Background */
    SDL_Color bg = {8, 10, 20, 255};
    draw_rect_filled(r, 0, 0, WINDOW_W, WINDOW_H, bg);

    /* Grid lines */
    SDL_Color grid = {18, 22, 40, 255};
    for (int x = 0; x < WINDOW_W; x += 40)
        draw_rect_filled(r, x, 0, 1, WINDOW_H, grid);
    for (int y = 0; y < WINDOW_H; y += 40)
        draw_rect_filled(r, 0, y, WINDOW_W, 1, grid);

    /* Animated circle ring */
    int cx = WINDOW_W/2, cy = WINDOW_H/2 - 60;
    float angle = (float)ticks / 400.0f;
    for (int i = 0; i < 12; i++) {
        float a = angle + i * (3.14159f * 2.0f / 12.0f);
        int px = cx + (int)(55 * cosf(a));
        int py = cy + (int)(55 * sinf(a));
        float brightness = (sinf(angle * 3 + i * 0.5f) + 1.0f) * 0.5f;
        SDL_Color dot = {
            (Uint8)(0   + brightness * 0),
            (Uint8)(150 + brightness * 60),
            (Uint8)(200 + brightness * 55),
            (Uint8)(80  + brightness * 175)
        };
        draw_circle_filled(r, px, py, 5, dot);
    }

    /* Inner glow */
    SDL_Color glow = {0, 180, 255, 30};
    draw_glow_circle(r, cx, cy, 30, glow, 4);

    /* Title */
    SDL_Color title_col = {0, 210, 255, 255};
    draw_text_simple(r, "SMART TRAFFIC CONTROL", cx - 120, cy + 80, 2, title_col);
    SDL_Color sub_col = {100, 120, 160, 255};
    draw_text_simple(r, "SYSTEM v2.0", cx - 52, cy + 100, 2, sub_col);

    /* Progress bar */
    int bx = WINDOW_W/2 - 160, by = cy + 130;
    SDL_Color bar_bg = {20, 25, 45, 255};
    SDL_Color bar_fg = {0, 200, 255, 255};
    SDL_Color bar_glow = {0, 200, 255, 60};
    draw_rounded_rect(r, bx, by, 320, 10, 4, bar_bg);
    if (progress > 0.0f) {
        int filled = (int)(320 * progress);
        draw_rounded_rect(r, bx, by, filled, 10, 4, bar_fg);
        draw_glow_circle(r, bx + filled, by + 5, 6, bar_glow, 2);
    }

    /* Percentage */
    char pct_buf[16];
    snprintf(pct_buf, sizeof(pct_buf), "%d%%", (int)(progress * 100));
    draw_text_simple(r, pct_buf, cx - 12, by + 16, 2, sub_col);

    /* Loading messages */
    const char *msgs[] = {
        "INITIALISING ROAD NETWORK...",
        "LOADING VEHICLE MODELS...",
        "CALIBRATING SIGNALS...",
        "STARTING SIMULATION..."
    };
    int msg_idx = (int)(progress * 4);
    if (msg_idx >= 4) msg_idx = 3;
    SDL_Color msg_col = {60, 80, 120, 255};
    int msg_x = cx - (int)strlen(msgs[msg_idx]) * 3;
    draw_text_simple(r, msgs[msg_idx], msg_x, by + 38, 1, msg_col);
}

/* ════════════════════════════════════════════════════════════════════
   DASHBOARD
   ════════════════════════════════════════════════════════════════════ */

/* Draw a stat card */
static void draw_stat_card(SDL_Renderer *r, int x, int y, int w, int h,
                            const char *label, const char *value,
                            SDL_Color accent) {
    /* Card BG */
    SDL_Color card_bg = {12, 16, 30, 210};
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    draw_rounded_rect(r, x, y, w, h, 5, card_bg);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    /* Left accent bar */
    draw_rect_filled(r, x, y + 4, 3, h - 8, accent);

    /* Label */
    SDL_Color lbl_col = {90, 110, 150, 255};
    draw_text_simple(r, label, x + 8, y + 6, 1, lbl_col);

    /* Value */
    SDL_Color val_col = {220, 230, 255, 255};
    draw_text_simple(r, value, x + 8, y + 19, 2, val_col);
}

/* Draw a bar graph */
static void draw_bar(SDL_Renderer *r, int x, int y, int w, int h,
                     float fraction, SDL_Color fill) {
    SDL_Color bg = {15, 20, 35, 200};
    draw_rect_filled(r, x, y, w, h, bg);
    if (fraction > 0.0f) {
        int fw = (int)(w * CLAMP(fraction, 0, 1));
        draw_rect_filled(r, x, y, fw, h, fill);
    }
}

void ui_draw_dashboard(SDL_Renderer *r, const SimState *s) {
    /* Dashboard panel on the right side */
    int px = WINDOW_W - 240;
    int py = 10;
    int pw = 230;
    int ph = WINDOW_H - 20;

    /* Panel background */
    SDL_Color panel_bg = {8, 10, 22, 200};
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    draw_rounded_rect(r, px, py, pw, ph, 8, panel_bg);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    /* Panel border */
    SDL_Color border = {0, 120, 180, 100};
    draw_rect_outline(r, px, py, pw, ph, border, 1);

    /* Header */
    SDL_Color header_col = {0, 200, 255, 255};
    draw_text_simple(r, "TRAFFIC CONTROL", px + 14, py + 12, 2, header_col);
    SDL_Color subheader = {0, 120, 160, 255};
    draw_text_simple(r, "LIVE DASHBOARD", px + 22, py + 28, 1, subheader);
    draw_rect_filled(r, px + 10, py + 40, pw - 20, 1, (SDL_Color){0, 80, 120, 200});

    int cy = py + 50;
    int cw = pw - 20;
    int ch = 38;
    int gap = 44;

    /* Build stat strings */
    char buf[32];

    /* Total vehicles */
    snprintf(buf, sizeof(buf), "%d", s->vehicles.count);
    draw_stat_card(r, px+10, cy, cw, ch, "ACTIVE VEHICLES", buf,
                   (SDL_Color){0, 200, 255, 255});
    cy += gap;

    /* Waiting */
    snprintf(buf, sizeof(buf), "%d", vehicle_count_waiting(&s->vehicles));
    SDL_Color wait_col = vehicle_count_waiting(&s->vehicles) > 20
        ? (SDL_Color){255, 80, 50, 255}
        : (SDL_Color){255, 180, 0, 255};
    draw_stat_card(r, px+10, cy, cw, ch, "WAITING", buf, wait_col);
    cy += gap;

    /* Avg wait time */
    snprintf(buf, sizeof(buf), "%.1fs", s->avg_wait_time);
    draw_stat_card(r, px+10, cy, cw, ch, "AVG WAIT", buf,
                   (SDL_Color){150, 100, 255, 255});
    cy += gap;

    /* Emergency vehicles */
    snprintf(buf, sizeof(buf), "%d", vehicle_count_by_type(&s->vehicles, VT_AMBULANCE)
                                   + vehicle_count_by_type(&s->vehicles, VT_POLICE));
    SDL_Color emerg_col = {255, 50, 50, 255};
    draw_stat_card(r, px+10, cy, cw, ch, "EMERGENCY", buf, emerg_col);
    cy += gap;

    /* Accidents */
    snprintf(buf, sizeof(buf), "%d active", s->accidents.count);
    draw_stat_card(r, px+10, cy, cw, ch, "ACCIDENTS", buf,
                   (SDL_Color){255, 120, 0, 255});
    cy += gap;

    /* FPS */
    snprintf(buf, sizeof(buf), "%d fps", s->fps);
    SDL_Color fps_col = s->fps >= 50 ? (SDL_Color){50, 220, 100, 255}
                                      : (SDL_Color){255, 100, 60, 255};
    draw_stat_card(r, px+10, cy, cw, ch, "FPS", buf, fps_col);
    cy += gap;

    /* Density bar */
    draw_rect_filled(r, px + 10, cy, cw, 1, (SDL_Color){0, 60, 100, 180});
    cy += 6;
    draw_text_simple(r, "TRAFFIC DENSITY", px + 14, cy, 1, (SDL_Color){80, 110, 160, 255});
    cy += 12;
    SDL_Color density_col = {0, 190, 255, 255};
    float dens = CLAMP((float)s->vehicles.count / MAX_VEHICLES, 0, 1);
    draw_bar(r, px + 10, cy, cw, 10, dens, density_col);
    cy += 16;

    /* Density percentage */
    snprintf(buf, sizeof(buf), "%.0f%%", dens * 100.0f);
    draw_text_simple(r, buf, px + 10, cy, 1, (SDL_Color){150, 180, 220, 255});
    cy += 18;

    /* Vehicle type breakdown */
    draw_rect_filled(r, px + 10, cy, cw, 1, (SDL_Color){0, 60, 100, 180});
    cy += 6;
    draw_text_simple(r, "VEHICLE TYPES", px + 14, cy, 1, (SDL_Color){80, 110, 160, 255});
    cy += 12;

    struct { const char *lbl; VehicleType t; SDL_Color col; } types[] = {
        {"Cars", VT_CAR, {0, 190, 255, 255}},
        {"Buses", VT_BUS, {20, 140, 230, 255}},
        {"Bikes", VT_BIKE, {220, 180, 40, 255}},
        {"Ambul", VT_AMBULANCE, {240, 60, 60, 255}},
        {"Police", VT_POLICE, {60, 80, 220, 255}},
    };
    for (int i = 0; i < 5; i++) {
        int cnt = vehicle_count_by_type(&s->vehicles, types[i].t);
        float frac = (s->vehicles.count > 0)
            ? (float)cnt / s->vehicles.count : 0.0f;
        draw_text_simple(r, types[i].lbl, px + 10, cy, 1, (SDL_Color){140, 160, 200, 255});
        snprintf(buf, sizeof(buf), "%d", cnt);
        draw_text_simple(r, buf, px + cw - 18, cy, 1, types[i].col);
        draw_bar(r, px + 10, cy + 9, cw, 4, frac, types[i].col);
        cy += 20;
    }

    cy += 8;

    /* Mode indicators */
    draw_rect_filled(r, px + 10, cy, cw, 1, (SDL_Color){0, 60, 100, 180});
    cy += 6;

    if (s->night_mode) {
        SDL_Color night_ind = {80, 100, 200, 255};
        draw_text_simple(r, "[ NIGHT MODE ]", px + 20, cy, 1, night_ind);
        cy += 14;
    }
    if (s->rain_enabled) {
        SDL_Color rain_ind = {60, 160, 220, 255};
        draw_text_simple(r, "[ RAIN ON ]", px + 20, cy, 1, rain_ind);
        cy += 14;
    }
    if (s->paused) {
        SDL_Color pause_ind = {255, 180, 0, 255};
        draw_text_simple(r, "[ PAUSED ]", px + 22, cy, 1, pause_ind);
        cy += 14;
    }

    /* Density factor */
    snprintf(buf, sizeof(buf), "DENSITY x%.1f", s->density_factor);
    draw_text_simple(r, buf, px + 10, cy, 1, (SDL_Color){100, 140, 180, 255});
    cy += 16;

    /* Uptime */
    Uint32 uptime_sec = (get_ticks() - s->sim_start) / 1000;
    snprintf(buf, sizeof(buf), "UP %02u:%02u", uptime_sec/60, uptime_sec%60);
    draw_text_simple(r, buf, px + 10, cy, 1, (SDL_Color){70, 90, 130, 255});
}

/* ════════════════════════════════════════════════════════════════════
   PAUSE OVERLAY
   ════════════════════════════════════════════════════════════════════ */
void ui_draw_paused_overlay(SDL_Renderer *r) {
    SDL_Color overlay = {0, 0, 0, 120};
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, overlay.r, overlay.g, overlay.b, overlay.a);
    SDL_Rect full = {0, 0, WINDOW_W, WINDOW_H};
    SDL_RenderFillRect(r, &full);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_Color txt = {255, 200, 0, 255};
    draw_text_simple(r, "SIMULATION PAUSED", WINDOW_W/2 - 90, WINDOW_H/2 - 14, 2, txt);
    SDL_Color sub = {150, 150, 180, 255};
    draw_text_simple(r, "PRESS SPACE TO RESUME", WINDOW_W/2 - 90, WINDOW_H/2 + 10, 1, sub);
}

/* ════════════════════════════════════════════════════════════════════
   KEYBIND HELP (bottom of screen)
   ════════════════════════════════════════════════════════════════════ */
void ui_draw_keybind_help(SDL_Renderer *r) {
    const char *binds[] = {
        "SPACE:PAUSE  UP/DN:DENSITY  A:ACCIDENT  E:EMERGENCY  N:NIGHT  R:RAIN  F5:RESET  ESC:QUIT",
        NULL
    };
    SDL_Color bg = {5, 8, 18, 200};
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    draw_rect_filled(r, 0, WINDOW_H - 18, WINDOW_W - 240, 18, bg);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_Color txt = {60, 80, 120, 255};
    draw_text_simple(r, binds[0], 6, WINDOW_H - 13, 1, txt);
}

/* ════════════════════════════════════════════════════════════════════
   RAIN
   ════════════════════════════════════════════════════════════════════ */
void ui_update_rain(SimState *s, float dt) {
    (void)dt;
    for (int i = 0; i < 300; i++) {
        s->rain_drops[i][0] += s->rain_drops[i][2];
        s->rain_drops[i][1] += s->rain_drops[i][3];
        if (s->rain_drops[i][1] > WINDOW_H) {
            s->rain_drops[i][1] = (float)(rand() % 20 - 20);
            s->rain_drops[i][0] = (float)(rand() % WINDOW_W);
        }
        if (s->rain_drops[i][0] < 0)        s->rain_drops[i][0] = WINDOW_W;
        if (s->rain_drops[i][0] > WINDOW_W) s->rain_drops[i][0] = 0;
    }
}

void ui_draw_rain(SDL_Renderer *r, const SimState *s) {
    if (!s->rain_enabled) return;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 150, 190, 255, 60);
    for (int i = 0; i < 300; i++) {
        int x = (int)s->rain_drops[i][0];
        int y = (int)s->rain_drops[i][1];
        SDL_RenderDrawLine(r, x, y, x + (int)s->rain_drops[i][2] * 2,
                                     y + (int)s->rain_drops[i][3] * 2);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

/* ════════════════════════════════════════════════════════════════════
   DENSITY INDICATOR (left panel mini-bar)
   ════════════════════════════════════════════════════════════════════ */
void ui_draw_density_indicator(SDL_Renderer *r, float density_factor) {
    int x = 10, y = 10;
    draw_text_simple(r, "DENSITY", x, y, 1, (SDL_Color){80, 110, 160, 255});
    float frac = (density_factor - 0.5f) / 2.5f;
    SDL_Color col = frac < 0.4f ? (SDL_Color){50, 220, 100, 255}
                 : frac < 0.7f ? (SDL_Color){255, 180,  0, 255}
                                : (SDL_Color){255,  50, 60, 255};
    draw_bar(r, x, y + 12, 80, 8, frac, col);
    char buf[8];
    snprintf(buf, sizeof(buf), "x%.1f", density_factor);
    draw_text_simple(r, buf, x, y + 24, 1, (SDL_Color){150, 180, 220, 255});
}
