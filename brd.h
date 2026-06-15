#ifndef BRD_H
#define BRD_H

static int brd_w;
static int brd_w2;

static int brd[100];

static void brd_init(int w) {
  brd_w = w;
  brd_w2 = w * w;

  for (int i = 0; i < brd_w2 - 1; i++) brd[i] = i + 1;
  for (int i = 0; i < brd_w2; i++) {
    for (int j = 0; j < brd_w2; j++) {
      if (rand() % 2) continue;
      int tmp = brd[i];
      brd[i] = brd[j];
      brd[j] = tmp;
    }
  }
}

static int brd_swap(int a, int b) {
  int tmp = brd[a];
  brd[a] = brd[b];
  brd[b] = tmp;

  int won = brd[brd_w2 - 1] == 0;;
  for (int i = 0; i < brd_w2 - 1; i++) {
    if (brd[i] != i + 1) won = 0;
  }
  return won;
}

#endif
