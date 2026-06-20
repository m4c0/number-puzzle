#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "vlk.h"

#define W 2064
#define H 2752

FILE * vlk_open(const char * name, const char * ext) {
  char buf[1024];
#ifdef _WIN32
  sprintf(buf, "app/%s.%s", name, ext);
#else
  sprintf(buf, "puzzle.app/Contents/Resources/%s.%s", name, ext);
#endif
  return fopen(buf, "rb");
}

void vlk_log(int r, const char * msg) {
  printf("Vulkan call failed (code=%d): %s\n", r, msg);
  exit(1);
}

#ifdef _WIN32
HWND vlk_hwnd;
#else
void * vlk_metal_layer() { return NULL; }
#endif

int main() {
  vlk_init(0);

  void * buf = vlk_headless(W, H);

  vlk_deinit();

  stbi_write_png("shot.png", W, H, 4, buf, W * 4);
}
