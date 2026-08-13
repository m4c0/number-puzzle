#define CFLAGS "-g"
#define RES_PATH(X) X".app/Contents/Resources"
#include "build.h"

#define CROSS(X) RUN("spirv-cross", "shader."X".spv", "--msl", "--output", APP".app/Contents/Resources/shader."X".metal", "--flip-vert-y");

static void print_key(FILE * f, const char * key) {}

static int pch() {
  char * args[] = {
    "clang", "-Wall", "-g", "-x", "c-header",
    "-IVulkan-Headers/include",
    "-D", "VK_USE_PLATFORM_METAL_EXT",
    "-D", "VLK_USE_VOLK",
    "-o", "pch.pch", "pch.h", 0 };
  return run(args);
}

static int link_exe() {
  char * args[] = {
    "clang", "-Wall",
    "-framework", "AppKit",
    "-framework", "AudioToolbox",
    "-framework", "MetalKit",
    "-o", "puzzle.app/Contents/MacOS/puzzle", 
    OBJS, "vlk.o", "stb_image.o", "volk.o", "puzzle-osx.o",
    0 };
  return run(args);
}

static int link_shots_exe() {
  char * args[] = {
    "clang", "-Wall",
    "-framework", "AppKit",
    "-framework", "AudioToolbox",
    "-framework", "MetalKit",
    "-o", "puzzle.app/Contents/MacOS/shots", 
    OBJS, "vlk.o", "stb_image.o", "volk.o", "shots.o",
    0 };
  return run(args);
}

static void mkd(const char * n, const char * p) {
  char buf[1024];
  snprintf(buf, 1024, "%s.app/%s", n, p);
  mkdir(buf, 0777);
}
static int app(const char * n) {
  mkd(n, "");
  mkd(n, "Contents");
  mkd(n, "Contents/MacOS");
  mkd(n, "Contents/Resources");

  char buf[1024];
  snprintf(buf, 1024, "%s.app/Contents/MacOS/", n);

  char * args[] = { "cp", "libvulkan.dylib", buf, 0 };
  return run(args);
}

int main(int argc, char ** argv) {
  if (pch()) return 1;

  HDR("stb_image", "STB_IMAGE_IMPLEMENTATION");
  HDR("volk",      "VOLK_IMPLEMENTATION");
  HDR("vlk",       "VLK_IMPL");

  if (app("puzzle")) return 1;
  CM("puzzle-osx");
  if (compile_and_link_exe()) return 1;

  CC("shots");
  if (link_shots_exe()) return 1;

  if (shaders()) return 1;

  for (int i = 1; i <= 31; i++) {
    char buf[128];
    snprintf(buf, 128, "imgs/bg-%003d.jpg", i);
    RUN("cp", buf, "puzzle.app/Contents/Resources");
  }

  return 0;
}
