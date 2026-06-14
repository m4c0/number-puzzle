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

static int run(char ** args) {
  assert(args && args[0]);

  if (0 == _spawnvp(_P_WAIT, args[0], (const char * const *)args)) {
    return 0;
  }

  fprintf(stderr, "failed to run child process: %s\n", args[0]);
  return 1;
}

static int shader(char * name) {
  char spv[1024]; snprintf(spv, 1024, "app/%s.spv", name);

  char * args[] = { "glslang", "-V", name, "-o", spv, 0 };
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
    "-o", "app/puzzle.exe", 
    "vlk.o",
    "volk.o", "microui.o", "puzzle-win.o",
    "-ladvapi32", "-lole32", "-lshell32", "-luser32",
    0 };
  return run(args);
}

int main(int argc, char ** argv) {
  if (argc != 1) return (usage(), 1);

  _mkdir("app");

  if (pch()) return 1;

  if (hdr("volk.h", "volk.o", "VOLK_IMPLEMENTATION")) return 1;

  if (hdr("vlk.h", "vlk.o", "VLK_IMPL")) return 1;

  if (cc_nopch("microui.c", "microui.o")) return 1;

  if (cc("puzzle-win.c", "puzzle-win.o")) return 1;
  if (link_exe()) return 1;

  if (shader("puzzle.frag")) return 1;
  if (shader("puzzle.vert")) return 1;

  return 0;
}
