#define CFLAGS "-g"
#define RES_PATH(X) X".app/Contents/Resources"
#include "build.h"

#define CROSS(X) RUN("spirv-cross", "shader."X".spv", "--msl", "--output", APP".app/Contents/Resources/shader."X".metal", "--flip-vert-y");

static void print_key(FILE * f, const char * key) {}

static int pch() {
  RUN("clang", "-Wall", "-x", "c-header", "-o", "pch.pch", "pch.h", CFLAGS);
  return 0;
}

static int link_exe() {
  RUN("clang", "-Wall",
    "-framework", "AppKit",
    "-framework", "AudioToolbox",
    "-framework", "Metal",
    "-framework", "MetalKit",
    "-o", APP".app/Contents/MacOS/main", 
    OBJS, "app-osx.o");
  return 0;
}

static int link_shots_exe() {
  RUN("clang", "-Wall",
    "-framework", "AppKit",
    "-framework", "AudioToolbox",
    "-framework", "Metal",
    "-framework", "MetalKit",
    "-o", APP".app/Contents/MacOS/shots", 
    OBJS, "stb_image.o", "shots-osx.o");
  return 0;
}


int main(int argc, char ** argv) {
  mkdir(APP".app", 0777);
  mkdir(APP".app/Contents", 0777);
  mkdir(APP".app/Contents/MacOS", 0777);
  mkdir(APP".app/Contents/Resources", 0777);

  if (pch()) return 1;

  // It's nearly mandatory to use "modules" with ObjC.
  // The compilation speed without it is abismal.
  HDR("stb_image", "STB_IMAGE_IMPLEMENTATION");
  CM("app-osx");
  if (compile_and_link_exe()) return 1;
  if (shaders()) return 1;
  CROSS("vert");
  CROSS("frag");

  CM("shots-osx");
  if (link_shots_exe()) return 1;

  return 0;
}
