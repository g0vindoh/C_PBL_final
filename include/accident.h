#ifndef ACCIDENT_H
#define ACCIDENT_H

#include "utils.h"

typedef struct {
    float  x, y;
    int    active;
    Uint32 start_time;
    int    duration_ticks;   /* ticks until cleared             */
    float  flash;            /* 0-1 warning flash               */
    int    responder_sent;
} Accident;

typedef struct {
    Accident accidents[MAX_ACCIDENTS];
    int      count;
    int      total_occurred;
} AccidentArray;

void accident_array_init(AccidentArray *aa);
void accident_trigger(AccidentArray *aa, float x, float y);
void accident_update_all(AccidentArray *aa);
void accident_draw_all(SDL_Renderer *r, const AccidentArray *aa);
int  accident_blocks_pos(const AccidentArray *aa, float x, float y, float radius);

#endif /* ACCIDENT_H */
