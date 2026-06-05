#ifndef SIGNAL_H
#define SIGNAL_H

#include "utils.h"

typedef enum { SIG_RED=0, SIG_YELLOW, SIG_GREEN } SignalPhase;

typedef struct {
    int         id;
    int         ix, iy;       /* intersection centre (px)          */
    SignalPhase phase;
    int         phase_ticks;  /* ticks in current phase            */
    int         green_dur;    /* adaptive green duration (ticks)   */
    int         yellow_dur;
    int         red_dur;
    int         queue_len;    /* vehicles queued in covered lanes  */
    int         forced_green; /* 1 = emergency override            */
    Uint32      forced_until;
    float       glow;         /* 0–1 glow intensity (animated)     */
} Signal;

typedef struct {
    Signal signals[MAX_SIGNALS];
    int    count;
} SignalArray;

/* ── API ─────────────────────────────────────────────────────────── */
void signal_array_init(SignalArray *sa);
void signal_update_all(SignalArray *sa, int *queue_lens);
void signal_draw_all(SDL_Renderer *r, const SignalArray *sa);
void signal_force_green(SignalArray *sa, float ex, float ey, float radius);
int  signal_is_red_at(const SignalArray *sa, float x, float y, Direction dir);

#endif /* SIGNAL_H */
