/* signal.c – Adaptive traffic signal management
 * ──────────────────────────────────────────────── */
#include "../include/signal.h"
#include <string.h>

/* ── Intersection positions matching utils.h ─────────────────────── */
static const int INT_X[MAX_SIGNALS] = { INT_X1, INT_X2, INT_X3, INT_X4, INT_X5, INT_X6 };
static const int INT_Y[MAX_SIGNALS] = { INT_Y1, INT_Y2, INT_Y3, INT_Y4, INT_Y5, INT_Y6 };

/* Tick constants (at ~60fps) */
#define TICKS_PER_SEC   60
#define BASE_GREEN     (5  * TICKS_PER_SEC)
#define BASE_YELLOW    (2  * TICKS_PER_SEC)
#define BASE_RED       (6  * TICKS_PER_SEC)
#define MIN_GREEN      (2  * TICKS_PER_SEC)
#define MAX_GREEN      (12 * TICKS_PER_SEC)

/* ── Initialise all signals ──────────────────────────────────────── */
void signal_array_init(SignalArray *sa) {
    memset(sa, 0, sizeof(*sa));
    sa->count = MAX_SIGNALS;
    for (int i = 0; i < MAX_SIGNALS; i++) {
        Signal *sig = &sa->signals[i];
        sig->id         = i;
        sig->ix         = INT_X[i];
        sig->iy         = INT_Y[i];
        /* Stagger phases so not all green at once */
        sig->phase      = (i % 3 == 0) ? SIG_GREEN : SIG_RED;
        sig->green_dur  = BASE_GREEN;
        sig->yellow_dur = BASE_YELLOW;
        sig->red_dur    = BASE_RED;
        sig->phase_ticks = 0;
        sig->glow       = 0.0f;
    }
}

/* ── Adaptive timing update ──────────────────────────────────────── */
void signal_update_all(SignalArray *sa, int *queue_lens) {
    Uint32 now = get_ticks();

    for (int i = 0; i < sa->count; i++) {
        Signal *sig = &sa->signals[i];

        /* Emergency override – keep green until expired.
           Use SDL_TICKS_PASSED to handle Uint32 wraparound safely. */
        if (sig->forced_green) {
            if (SDL_TICKS_PASSED(now, sig->forced_until)) {
                sig->forced_green = 0;
            } else {
                sig->phase = SIG_GREEN;
                sig->glow = 0.8f + 0.2f * sinf((float)now / 200.0f);
                continue;
            }
        }

        /* Adaptive green timing from queue length */
        if (queue_lens) {
            int ql = queue_lens[i];
            /* Scale: 0 queue = MIN_GREEN, 10+ = MAX_GREEN */
            float scale = CLAMP((float)ql / 10.0f, 0.0f, 1.0f);
            sig->green_dur = (int)LERP(MIN_GREEN, MAX_GREEN, scale);
        }

        sig->phase_ticks++;

        int phase_limit;
        switch (sig->phase) {
            case SIG_GREEN:  phase_limit = sig->green_dur;  break;
            case SIG_YELLOW: phase_limit = sig->yellow_dur; break;
            case SIG_RED:    phase_limit = sig->red_dur;    break;
            default:         phase_limit = BASE_RED;
        }

        if (sig->phase_ticks >= phase_limit) {
            sig->phase_ticks = 0;
            /* Cycle: GREEN -> YELLOW -> RED -> GREEN */
            switch (sig->phase) {
                case SIG_GREEN:  sig->phase = SIG_YELLOW; break;
                case SIG_YELLOW: sig->phase = SIG_RED;    break;
                case SIG_RED:    sig->phase = SIG_GREEN;  break;
            }
        }

        /* Glow pulse for green */
        sig->glow = (sig->phase == SIG_GREEN)
            ? 0.5f + 0.5f * sinf((float)now / 400.0f)
            : (sig->phase == SIG_YELLOW ? 0.6f : 0.0f);
    }
}

/* ── Force green for emergency vehicles ─────────────────────────── */
void signal_force_green(SignalArray *sa, float ex, float ey, float radius) {
    float r2 = radius * radius;
    for (int i = 0; i < sa->count; i++) {
        Signal *sig = &sa->signals[i];
        float dx = ex - sig->ix, dy = ey - sig->iy;
        if (dx*dx + dy*dy < r2) {
            sig->forced_green = 1;
            sig->forced_until = get_ticks() + EMERGENCY_OVERRIDE_MS;
            sig->phase = SIG_GREEN;
        }
    }
}

/* ── Query: is the signal at a position currently red? ───────────── */
/* Vehicles check their stop-line; we look for the nearest signal
   within 120 px in the direction of travel.                          */
int signal_is_red_at(const SignalArray *sa, float x, float y, Direction dir) {
    float check_dist = SIGNAL_LOOKAHEAD_PX;
    float px = x, py = y;

    /* Project forward to find approach point */
    switch (dir) {
        case DIR_RIGHT: px = x + check_dist; break;
        case DIR_LEFT:  px = x - check_dist; break;
        case DIR_DOWN:  py = y + check_dist; break;
        case DIR_UP:    py = y - check_dist; break;
    }

    /* Find nearest intersection */
    float best_d2 = 9999999.0f;
    int   best_i  = -1;
    for (int i = 0; i < sa->count; i++) {
        float dx = px - sa->signals[i].ix;
        float dy = py - sa->signals[i].iy;
        float d2 = dx*dx + dy*dy;
        if (d2 < best_d2) { best_d2 = d2; best_i = i; }
    }

    if (best_i < 0) return 0;
    if (best_d2 > SIGNAL_DETECT_RADIUS_PX * SIGNAL_DETECT_RADIUS_PX) return 0;

    const Signal *sig = &sa->signals[best_i];
    return (sig->phase == SIG_RED || sig->phase == SIG_YELLOW);
}

/* ── Drawing ─────────────────────────────────────────────────────── */

/* Draw a single signal post */
static void draw_signal(SDL_Renderer *r, const Signal *sig) {
    int x = sig->ix, y = sig->iy;

    /* Signal housing */
    SDL_Color housing = {30, 30, 35, 255};
    draw_rounded_rect(r, x - 10, y - 28, 20, 56, 4, housing);

    /* Three lights */
    struct { int dy; SDL_Color on; } lights[3] = {
        {-18, {255,  50,  50, 255}},   /* Red    */
        {  0, {255, 200,   0, 255}},   /* Yellow */
        { 18, { 50, 220,  80, 255}},   /* Green  */
    };

    for (int i = 0; i < 3; i++) {
        int active = (i==0 && sig->phase==SIG_RED)   ||
                     (i==1 && sig->phase==SIG_YELLOW) ||
                     (i==2 && sig->phase==SIG_GREEN);
        SDL_Color off = {30, 30, 30, 255};
        SDL_Color col = active ? lights[i].on : off;
        draw_circle_filled(r, x, y + lights[i].dy, 6, col);

        if (active) {
            SDL_Color glow = col; glow.a = (Uint8)(120 * sig->glow);
            draw_glow_circle(r, x, y + lights[i].dy, 6, glow, 3);
        }
    }

    /* Countdown timer text — show remaining time in current phase */
    char buf[16];
    int phase_limit;
    switch (sig->phase) {
        case SIG_GREEN:  phase_limit = sig->green_dur;  break;
        case SIG_YELLOW: phase_limit = sig->yellow_dur; break;
        case SIG_RED:    phase_limit = sig->red_dur;    break;
        default:         phase_limit = sig->red_dur;    break;
    }
    int remaining  = phase_limit - sig->phase_ticks;
    int countdown  = CLAMP(remaining / 60 + 1, 0, 99);
    snprintf(buf, sizeof(buf), "%d", countdown);
    SDL_Color txt = {200, 200, 200, 255};
    draw_text_simple(r, buf, x - 6, y + 32, 1, txt);
}

void signal_draw_all(SDL_Renderer *r, const SignalArray *sa) {
    for (int i = 0; i < sa->count; i++)
        draw_signal(r, &sa->signals[i]);
}
