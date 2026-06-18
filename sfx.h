#ifndef SFX_H
#define SFX_H

void sfx_init();
void sfx_filler(float * buf, unsigned sz);

void sfx_move();
void sfx_shuffle();
void sfx_win();

#ifdef SFX_IMPL

static float sfx_rand_buf[1024];

static unsigned sp = 0;
static unsigned d = 1;

float sfx_rand(unsigned ssp) {
  return sfx_rand_buf[(ssp / 9) % 1024] * 2;
}
float sfx_sqr(unsigned ssp) {
  return ((ssp / d) % 2);
}

void sfx_filler(float * buf, unsigned sz) {
  int ssp = sp;
  float mult;
  if (ssp < 1000) {
    mult = ssp / 1000.0f;
  } else if (ssp < 2000) {
    mult = 1.0;
  } else if (ssp < 3000) {
    mult = (3000 - ssp) / 1000.0f;
  } else {
    mult = 0;
  }
  for (unsigned i = 0; i < sz; ++i) {
    buf[i] = 0.25f * mult * (sfx_rand(ssp) - 0.5f);
    ssp++;
  }
  sp = ssp;
}

static void sfx_play(unsigned div) {
  d = div;
  sp = 0;
}
void sfx_move() { sfx_play(2); }
void sfx_shuffle() {}
void sfx_win() { sfx_move(); }

void sfx_init() {
  for (int i = 0; i < 1024; i++) {
    sfx_rand_buf[i] = (float)rand() / (float)RAND_MAX;
  }
}

#endif
#endif
