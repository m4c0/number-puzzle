//#define OPT "-gdwarf"
#define OPT "-O3"

#define CFLAGS OPT
#define RES_PATH(X) "."
#include "build.h"

#define CROSS(X) RUN("spirv-cross", "shader."X".spv", "--hlsl", "--output", "shader."X".hlsl", "--shader-model", "50", "--flip-vert-y");

static int rc() {
  char * args[] = { "llvm-rc.exe", "/FO", "main.res", "main.rc", 0 };
  return run(args);
}

static int pch() {
  char * args[] = {
    "clang", "-Wall", OPT, "-x", "c-header",
    "-IVulkan-Headers/include",
    "-D", "VK_USE_PLATFORM_WIN32_KHR",
    "-D", "VLK_USE_VOLK",
    "-o", "pch.pch", "pch.h", 0 };
  return run(args);
}

static int cc(char * src, char * o) {
  char * args[] = {
    "clang", "-Wall", OPT, "-include-pch", "pch.pch",
    "-o", o, "-c", src, 0 };
  return run(args);
}

static int hdr(char * src, char * o, char * d) {
  char * args[] = {
    "clang", "-Wall", "-x", "c", OPT, "-include-pch", "pch.pch",
    "-D", d, "-o", o, "-c", src, 0
  };
  return run(args);
}

static int link_exe() {
  char * args[] = {
    "clang", "-Wall", OPT,
    "-o", "puzzle.exe", "main.res",
    "sfx.o", "snd.o", "vlk.o",
    "stb_image.o", "volk.o", "puzzle-win.o",
    "-ladvapi32", "-lole32", "-lshell32", "-luser32",
    0 };
  return run(args);
}

static int link_shots_exe() {
  char * args[] = {
    "clang", "-Wall", OPT,
    "-o", "shots.exe", 
    "sfx.o", "snd.o", "vlk.o",
    "stb_image.o", "volk.o", "shots.o",
    "-ladvapi32", "-lole32", "-lshell32", "-luser32",
    0 };
  return run(args);
}

int icon() {
  unsigned sz;
  char * img = slurp("Assets.xcassets\\AppIcon.appiconset\\Icon-1024.png", &sz);

  FILE * f = fopen("icon.ico", "wb");
  fwrite("\0\0\1\0\1\0", 6, 1, f); // 0=Reserved; 1=ICO; 1 Image
  fwrite("\0\0\0\0\0\0\x20\0", 8, 1, f); // W/H/C/Res. Planes/Bits

  fwrite(&sz, 4, 1, f);
  fwrite("\x16\0\0\0", 4, 1, f); // 20=offset from BOS
  fwrite(img, sz, 1, f);

  fclose(f);
  return 0;
}

int main(int argc, char ** argv) {
  if (pch()) return 1;

  if (hdr("stb_image.h", "stb_image.o", "STB_IMAGE_IMPLEMENTATION")) return 1;
  if (hdr("volk.h",      "volk.o",      "VOLK_IMPLEMENTATION"))      return 1;

  if (hdr("sfx.h", "sfx.o", "SFX_IMPL")) return 1;
  if (hdr("snd.h", "snd.o", "SND_IMPL")) return 1;
  if (hdr("vlk.h", "vlk.o", "VLK_IMPL")) return 1;

  if (icon()) return 1;
  if (shaders()) return 1;
  if (rc()) return 1;

  if (cc("puzzle-win.c", "puzzle-win.o")) return 1;
  if (link_exe()) return 1;

  if (cc("shots.c", "shots.o")) return 1;
  if (link_shots_exe()) return 1;

  return 0;
}
