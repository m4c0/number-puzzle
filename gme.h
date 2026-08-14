#ifndef GME_H
#define GME_H

typedef struct gme_upc_s {
  float    aspect_x, aspect_y;
  float    time;
  float    won;
  unsigned sel_id;
  unsigned brd_w;
} gme_upc_t;

void gme_init(int w, int h);
void gme_deinit(void);
void gme_frame(void);

void gme_resize(int w, int h);

void gme_load(void *);

const gme_upc_t * gme_pc(void);

void gme_mouse_move(int x, int y);
void gme_mouse_down(int x, int y);

void gme_reset(void);

extern void gme_load_atlas(const char * file);

#define GME_BUF_SIZE 400

#ifdef GME_IMPL
#include "brd.h"
#include "sfx.h"
#include "snd.h"
#include "tim.h"

static gme_upc_t gme_upc;
static unsigned gme_width;
static unsigned gme_height;

void gme_init(int w, int h) {
  sfx_init();
  snd_init(sfx_filler);

  gme_upc.won = 1;
  gme_resize(w, h);
  gme_reset();
}

void gme_deinit(void) {
  snd_deinit();
}

void gme_frame(void) {
  float a = (float)gme_width / (float)gme_height;
  gme_upc.aspect_x = a > 1 ? a : 1;
  gme_upc.aspect_y = a > 1 ? 1 : (1.0 / a);
  gme_upc.time = tim_now();
}

void gme_resize(int w, int h) {
  gme_width = w;
  gme_height = h;
}

const gme_upc_t * gme_pc(void) {
  return &gme_upc;
}

void gme_load(void * tgt) {
  memcpy(tgt, brd, GME_BUF_SIZE);
}

static float gme_mouse(float p, float a) {
#ifdef __APPLE__
  p *= 2;
#endif

  p = p * 2 - 1;
  p *= a;

  p /= 0.9;
  p = p * 0.5 + 0.5;
  p *= brd_w;
  return p;
}
void gme_mouse_move(int x, int y) {
  if (gme_upc.won) return;

  float px = gme_mouse((float)x / (float)gme_width,  gme_upc.aspect_x);
  float py = gme_mouse((float)y / (float)gme_height, gme_upc.aspect_y);
  
  if (px < 0 || px >= brd_w || py < 0 || py >= brd_w) px = py = 10;
  gme_upc.sel_id = (int)px + (int)py * brd_w;
}

static int gme_board_swap(int a, int dx, int dy) {
  int bx = dx + (a % brd_w);
  int by = dy + (a / brd_w);
  if (bx < 0 || bx >= brd_w || by < 0 || by >= brd_w) return 0;
  int b = by * brd_w + bx;
  
  if (b >= brd_w2) return 0;
  if (brd[b]) return 0;

  int won = brd_swap(a, b);
  gme_upc.sel_id = 1000;
  gme_upc.won = won ? tim_now() : 0;
  if (gme_upc.won) sfx_win(); else sfx_move();
  return 1;
}
void gme_mouse_down(int x, int y) {
  if (gme_upc.won) {
    float px = gme_mouse((float)x / (float)gme_width,  gme_upc.aspect_x);
    float py = gme_mouse((float)y / (float)gme_height, gme_upc.aspect_y);
    if (px > brd_w || py > brd_w) gme_reset();
    return;
  }

  int id = gme_upc.sel_id;
  if (id >= brd_w2) return;

  if (gme_board_swap(id, +1,  0)) return;
  if (gme_board_swap(id, -1,  0)) return;
  if (gme_board_swap(id,  0, +1)) return;
  if (gme_board_swap(id,  0, -1)) return;
}

#define GME_ATLAS_COUNT 31
void gme_reset(void) {
  if (!gme_upc.won) return;

  int n = 1 + (rand() % GME_ATLAS_COUNT);
  char name[128];
  snprintf(name, 128, "bg-%003d", n);
  gme_load_atlas(name);

  brd_init(4);
  sfx_shuffle();

  gme_upc.won    = 0;
  gme_upc.sel_id = 1000;
  gme_upc.brd_w  = brd_w;
}

#endif
#endif
