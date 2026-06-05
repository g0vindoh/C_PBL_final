#ifndef UI_H
#define UI_H

#include "utils.h"
#include "traffic.h"

/* ── Startup / loading screen ────────────────────────────────────── */
void ui_draw_loading(SDL_Renderer *r, float progress, Uint32 ticks);

/* ── HUD dashboard ───────────────────────────────────────────────── */
void ui_draw_dashboard(SDL_Renderer *r, const SimState *s);

/* ── Overlay messages ────────────────────────────────────────────── */
void ui_draw_paused_overlay(SDL_Renderer *r);
void ui_draw_keybind_help(SDL_Renderer *r);

/* ── Rain particles ──────────────────────────────────────────────── */
void ui_update_rain(SimState *s, float dt);
void ui_draw_rain(SDL_Renderer *r, const SimState *s);

/* ── Minimap density indicator ───────────────────────────────────── */
void ui_draw_density_indicator(SDL_Renderer *r, float density_factor);

#endif /* UI_H */
