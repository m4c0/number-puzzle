#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "vlk.h"

#define W 2064
#define H 2752

unsigned vlk_open(const char * name, const char * ext, const void ** ptr) {
  char fname[1024];
#ifdef _WIN32
  sprintf(fname, "app/%s.%s", name, ext);
#else
  sprintf(fname, "puzzle.app/Contents/Resources/%s.%s", name, ext);
#endif
  FILE * f = fopen(fname, "rb");

  assert(0 == fseek(f, 0, SEEK_END));
  long sz = ftell(f);
  assert(sz);
  assert(0 == fseek(f, 0, SEEK_SET));

  void * data = malloc(sz + 1);
  assert(1 == fread(data, sz, 1, f));
  *ptr = data;

  fclose(f);
  return sz;
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
