/* accident.c – Accident simulation, warning indicators
 * ──────────────────────────────────────────────────── */
#include "../include/accident.h"
#include <string.h>

#define ACCIDENT_DURATION  (15 * 60) /* ~15 seconds at 60fps */

void accident_array_init(AccidentArray *aa) {
    memset(aa, 0, sizeof(*aa));
    aa->count = 0;
    aa->total_occurred = 0;
}

void accident_trigger(AccidentArray *aa, float x, float y) {
    /* Find free slot */
    for (int i = 0; i < MAX_ACCIDENTS; i++) {
        if (!aa->accidents[i].active) {
            Accident *a = &aa->accidents[i];
            a->x = x;
            a->y = y;
            a->active = 1;
            a->start_time = get_ticks();
            a->duration_ticks = ACCIDENT_DURATION;
            a->flash = 0.0f;
            a->responder_sent = 0;
            aa->count++;
            aa->total_occurred++;
            log_event("ACCIDENT at (%.0f, %.0f)", x, y);
            return;
        }
    }
    log_event("WARNING: all %d accident slots occupied, trigger at (%.0f, %.0f) dropped.",
              MAX_ACCIDENTS, x, y);
}

void accident_update_all(AccidentArray *aa) {
    for (int i = 0; i < MAX_ACCIDENTS; i++) {
        Accident *a = &aa->accidents[i];
        if (!a->active) continue;

        a->duration_ticks--;
        if (a->duration_ticks <= 0) {
            a->active = 0;
            aa->count--;
            log_event("Accident cleared.");
        }

        /* Flash warning */
        a->flash = 0.5f + 0.5f * sinf((float)get_ticks() / 150.0f);
    }
}

/* Returns 1 if position (x,y) is within radius of an active accident */
int accident_blocks_pos(const AccidentArray *aa, float x, float y, float radius) {
    for (int i = 0; i < MAX_ACCIDENTS; i++) {
        const Accident *a = &aa->accidents[i];
        if (!a->active) continue;
        float dx = x - a->x, dy = y - a->y;
        if (dx*dx + dy*dy < radius*radius) return 1;
    }
    return 0;
}

/* ── Drawing ─────────────────────────────────────────────────────── */
static void draw_accident(SDL_Renderer *r, const Accident *a) {
    int x = (int)a->x, y = (int)a->y;
    float f = a->flash;

    /* Hazard zone */
    SDL_Color zone = {255, 80, 0, (Uint8)(60 * f)};
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    draw_circle_filled(r, x, y, 30, zone);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    /* Wreckage shape */
    SDL_Color wreck = {60, 55, 55, 255};
    draw_rect_filled(r, x-12, y-8, 24, 16, wreck);
    /* Debris */
    SDL_Color debris = {100, 90, 80, 255};
    draw_rect_filled(r, x-16, y-4, 8,  8, debris);
    draw_rect_filled(r, x+ 8, y-6, 10, 6, debris);
    draw_rect_filled(r, x- 4, y+6, 6,  6, debris);

    /* Animated warning diamond */
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    /* Draw diamond as four triangles */
    SDL_SetRenderDrawColor(r, 255, 200, 0, (Uint8)(200 * f));
    for (int i = -1; i <= 1; i++) {
        SDL_RenderDrawLine(r, x + 20, y - i, x, y - 20 + ABS(i));
        SDL_RenderDrawLine(r, x - 20, y - i, x, y - 20 + ABS(i));
        SDL_RenderDrawLine(r, x + 20, y - i, x, y + 20 - ABS(i));
        SDL_RenderDrawLine(r, x - 20, y - i, x, y + 20 - ABS(i));
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    /* "!" text */
    SDL_Color exclam = {255, 220, 0, 255};
    draw_text_simple(r, "!", x - 2, y - 27, 2, exclam);

    /* Remaining time bar */
    int max_dur = ACCIDENT_DURATION;
    float progress = (float)a->duration_ticks / max_dur;
    SDL_Color bar_bg = {40, 40, 40, 200};
    SDL_Color bar_fg = {255, 120, 0, 255};
    draw_rect_filled(r, x - 20, y + 22, 40, 4, bar_bg);
    draw_rect_filled(r, x - 20, y + 22, (int)(40 * progress), 4, bar_fg);
}

void accident_draw_all(SDL_Renderer *r, const AccidentArray *aa) {
    for (int i = 0; i < MAX_ACCIDENTS; i++) {
        if (aa->accidents[i].active)
            draw_accident(r, &aa->accidents[i]);
    }
}
