#ifndef BRD_H
#define BRD_H

static int brd_w;
static int brd_w2;

static int brd[100];

static void brd_init(int w) {
  brd_w = w;
  brd_w2 = w * w;

  for (int i = 0; i < brd_w2 - 1; i++) brd[i] = i + 1;
  brd[brd_w2 - 1] = 0;

  for (int i = 0; i < brd_w2; i++) {
    for (int j = 0; j < brd_w2; j++) {
      if (rand() % 2) continue;
      int tmp = brd[i];
      brd[i] = brd[j];
      brd[j] = tmp;
    }
  }

  // https://www.lukelavalva.com/theoryofsliding
  // The number of cycles and the manhattan distance of the empty slot can't
  // have the same parity. Otherwise it is unsolvable.

  int zero = 0;
  int cycles = 0;
  int cycled[100] = {0};
  for (int i = 0; i < brd_w2; i++) {
    if (!brd[i]) zero = i;
    if (cycled[i]) continue;
    cycles++;

    int start = i;
    int j = i;
    do {
      cycled[j] = 1;
      j = brd[j];
    } while (j != start);
  }

  int zx = brd_w - 1 - (zero % brd_w);
  int zy = brd_w - 1 - (zero / brd_w);
  int mht = zx + zy;
  if ((mht % 2) != (cycles % 2)) return;

  // Try again
  brd_init(w);
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
