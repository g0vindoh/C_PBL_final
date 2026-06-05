#ifndef SIMULATION_H
#define SIMULATION_H

#include "traffic.h"

/* High-level simulation logic callable from main */
void simulation_handle_input(SimState *s, SDL_Event *e);
void simulation_step(SimState *s, float dt);
void simulation_draw(SDL_Renderer *r, SimState *s);

#endif /* SIMULATION_H */
