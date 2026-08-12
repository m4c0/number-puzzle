#define _CRT_SECURE_NO_WARNINGS
#include <sys/stat.h>
#include <assert.h>
#include <direct.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>

//#define OPT "-gdwarf"
#define OPT "-O3"

static void usage() {
  fprintf(stderr, "just call 'build' without arguments\n");
}

static char * slurp(const char * file, unsigned * sz_ptr) {
  FILE * f = fopen(file, "rb");
  assert(f);

  assert(0 == fseek(f, 0, SEEK_END));
  long sz = ftell(f);
  assert(sz);
  assert(0 == fseek(f, 0, SEEK_SET));

  char * data = malloc(sz + 1);
  assert(1 == fread(data, sz, 1, f));
  data[sz] = 0;

  fclose(f);
  *sz_ptr = sz;
  return data;
}

static int run(char ** args) {
  assert(args && args[0]);

  if (0 == _spawnvp(_P_WAIT, args[0], (const char * const *)args)) {
    return 0;
  }

  fprintf(stderr, "failed to run child process: %s\n", args[0]);
  return 1;
}

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

static int cc_nopch(char * src, char * o) {
  char * args[] = {
    "clang", "-Wall", OPT, "-o", o, "-c", src, 0 };
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
  if (argc != 1) return (usage(), 1);

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
