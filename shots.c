#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
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

  void * buf = vlk_headless(800, 600);

  vlk_deinit();

  stbi_write_png("shot.png", 800, 600, 4, buf, 800 * 4);
}
