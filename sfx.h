#ifndef SFX_H
#define SFX_H

void sfx_filler(float * buf, unsigned sz);

void sfx_move();
void sfx_shuffle();
void sfx_win();

#ifdef SFX_IMPL

static unsigned sp = 0;
static unsigned d = 1;

void sfx_filler(float * buf, unsigned sz) {
  int ssp = sp;
  float mult;
  if (ssp < 1000) {
    mult = ssp / 1000.0f;
  } else if (ssp < 4000) {
    mult = 1.0;
  } else if (ssp < 5000) {
    mult = (5000 - ssp) / 1000.0f;
  } else {
    mult = 0;
  }
  for (unsigned i = 0; i < sz; ++i) {
    buf[i] = 0.25f * mult * (((ssp / d) % 2) - 0.5f);
    ssp++;
  }
  sp = ssp;
}

static void sfx_play(unsigned div) {
  d = div;
  sp = 0;
}
void sfx_move() { sfx_play(200); }
void sfx_shuffle() { sfx_play( 50); }
void sfx_win() { sfx_play(100); }

#endif
#endif
