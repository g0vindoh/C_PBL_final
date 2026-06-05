#ifndef UTILS_H
#define UTILS_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ── Window & Rendering ─────────────────────────────────────────── */
#define WINDOW_W   1280
#define WINDOW_H    800
#define WINDOW_TITLE "Smart Traffic Control System"

/* ── Grid / Road geometry ───────────────────────────────────────── */
#define ROAD_W       80          /* road width in pixels              */
#define LANE_W       36          /* single lane width                 */
#define INTERSECTION 90          /* intersection square side          */

/* Intersection centre points (px) */
#define INT_X1  320
#define INT_Y1  280
#define INT_X2  640
#define INT_Y2  280
#define INT_X3  960
#define INT_Y3  280
#define INT_X4  320
#define INT_Y4  520
#define INT_X5  640
#define INT_Y5  520
#define INT_X6  960
#define INT_Y6  520

/* ── Simulation tuning ──────────────────────────────────────────── */
#define MAX_VEHICLES      200
#define MAX_SIGNALS         6
#define MAX_ACCIDENTS       4
#define BASE_SPEED          2.0f
#define SAFE_DIST          40
#define SPAWN_INTERVAL_MS       800
#define LOG_FILE                "logs/traffic_log.txt"

/* ── Signal tuning (named so they're easy to change) ────────────── */
#define EMERGENCY_OVERRIDE_MS   4000   /* how long a forced-green lasts  */
#define SIGNAL_LOOKAHEAD_PX     140.0f /* how far ahead a vehicle looks   */
#define SIGNAL_DETECT_RADIUS_PX  80.0f /* max dist from intersection edge */

/* ── Color palette ──────────────────────────────────────────────── */
#define COL_BG_DAY       { 18,  22,  36, 255}
#define COL_BG_NIGHT     {  8,   8,  18, 255}
#define COL_ROAD_DAY     { 42,  47,  65, 255}
#define COL_ROAD_NIGHT   { 22,  25,  38, 255}
#define COL_LANE_MARK    { 90,  95, 110, 255}
#define COL_DASH_BG      { 10,  12,  22, 230}
#define COL_ACCENT       {  0, 210, 255, 255}
#define COL_WARN         {255, 180,   0, 255}
#define COL_DANGER       {255,  50,  60, 255}
#define COL_OK           { 50, 220, 100, 255}
#define COL_WHITE        {255, 255, 255, 255}

/* ── Utility macros ─────────────────────────────────────────────── */
#define CLAMP(v,lo,hi)  ((v)<(lo)?(lo):((v)>(hi)?(hi):(v)))
#define LERP(a,b,t)     ((a)+(((b)-(a))*(t)))
#define ABS(x)          ((x)<0?-(x):(x))
#define MIN(a,b)        ((a)<(b)?(a):(b))
#define MAX(a,b)        ((a)>(b)?(a):(b))
#define RAND_RANGE(a,b) ((a)+rand()%((b)-(a)+1))
#define DEG2RAD(d)      ((d)*3.14159265f/180.0f)

/* ── Direction enum ─────────────────────────────────────────────── */
typedef enum { DIR_RIGHT=0, DIR_LEFT, DIR_DOWN, DIR_UP } Direction;

/* ── Utility functions ──────────────────────────────────────────── */
void   draw_rect_filled(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c);
void   draw_rect_outline(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c, int thickness);
void   draw_circle(SDL_Renderer *r, int cx, int cy, int radius, SDL_Color c);
void   draw_circle_filled(SDL_Renderer *r, int cx, int cy, int radius, SDL_Color c);
void   draw_rounded_rect(SDL_Renderer *r, int x, int y, int w, int h, int rad, SDL_Color c);
void   draw_text_simple(SDL_Renderer *r, const char *text, int x, int y, int size, SDL_Color c);
void   draw_glow_circle(SDL_Renderer *r, int cx, int cy, int radius, SDL_Color c, int layers);
float  dist2(float x1,float y1,float x2,float y2);
void   log_event(const char *fmt, ...);
Uint32 get_ticks(void);

#endif /* UTILS_H */
