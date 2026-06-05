/* main.c – Entry point, SDL2 init, game loop
 * ─────────────────────────────────────────── */
#include "../include/simulation.h"
#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#  include <direct.h>   /* _mkdir */
#else
#  include <sys/stat.h> /* mkdir  */
#endif

#define TARGET_FPS   60
#define FRAME_MS     (1000 / TARGET_FPS)

/* ── SDL2 bootstrap ──────────────────────────────────────────────── */
static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;

static int sdl_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 0;
    }
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        return 0;
    }
    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        return 0;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

static void sdl_shutdown(void) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}

/* ── Loading screen animation (fake progress over ~2 s) ─────────── */
static void run_loading_screen(void) {
    Uint32 start = SDL_GetTicks();
    Uint32 duration = 2000; /* ms */
    SDL_Event e;

    while (1) {
        Uint32 elapsed = SDL_GetTicks() - start;
        float progress = (float)elapsed / duration;
        if (progress >= 1.0f) progress = 1.0f;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { sdl_shutdown(); exit(0); }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        ui_draw_loading(renderer, progress, SDL_GetTicks());
        SDL_RenderPresent(renderer);
        SDL_Delay(16);

        if (elapsed >= duration) break;
    }
}

/* ════════════════════════════════════════════════════════════════════
   MAIN
   ════════════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    srand((unsigned)time(NULL));

    /* Ensure log directory exists (no system() shell — use direct API) */
#ifdef _WIN32
    _mkdir("logs");
#else
    mkdir("logs", 0755);
#endif

    /* SDL init */
    if (!sdl_init()) {
        sdl_shutdown();
        return 1;
    }

    /* Loading screen */
    run_loading_screen();

    /* Simulation state */
    SimState sim;
    sim_init(&sim);

    /* ── Main loop ────────────────────────────────────────────────── */
    int running = 1;
    SDL_Event event;
    Uint32 frame_start;

    while (running) {
        frame_start = SDL_GetTicks();

        /* Events */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else {
                    simulation_handle_input(&sim, &event);
                }
            }
        }

        /* Update */
        simulation_step(&sim, 1.0f / TARGET_FPS);

        /* Draw */
        simulation_draw(renderer, &sim);
        SDL_RenderPresent(renderer);

        /* Cap framerate */
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_MS) SDL_Delay(FRAME_MS - frame_time);
    }

    log_event("Simulation exited cleanly. Total vehicles: %d, Accidents: %d",
              sim.total_vehicles, sim.total_accidents);

    sdl_shutdown();
    return 0;
}
