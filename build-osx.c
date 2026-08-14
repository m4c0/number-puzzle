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
    OBJS, "shots-osx.o");
  return 0;
}


int main(int argc, char ** argv) {
  mkdir(APP".app", 0777);
  mkdir(APP".app/Contents", 0777);
  mkdir(APP".app/Contents/MacOS", 0777);
  mkdir(APP".app/Contents/Resources", 0777);

  if (pch()) return 1;

  CM("app-osx");
  if (compile_and_link_exe()) return 1;
  if (shaders()) return 1;
  CROSS("vert");
  CROSS("frag");

  CM("shots-osx");
  if (link_shots_exe()) return 1;

  for (int i = 1; i <= 31; i++) {
    char buf[128];
    snprintf(buf, 128, "imgs/bg-%003d.jpg", i);
    RUN("cp", buf, RES_PATH(APP));
  }

  return 0;
}
