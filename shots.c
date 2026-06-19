#include "vlk.h"

FILE * vlk_open(const char * name, const char * ext) {
  char buf[1024];
  sprintf(buf, "puzzle.app/Contents/Resources/%s.%s", name, ext);
  return fopen(buf, "rb");
}

void vlk_log(int r, const char * msg) {
  printf("Vulkan call failed (code=%d): %s\n", r, msg);
  exit(1);
}

void * vlk_metal_layer() { return NULL; }

int main() {
  vlk_init(0);

  char * buf = malloc(800 * 600 * 4);
  vlk_headless(800, 600, buf);

  vlk_deinit();
}
