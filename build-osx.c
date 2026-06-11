#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage() {
  fprintf(stderr, "just call 'build' without arguments\n");
}

static int run(char ** args) {
  assert(args && args[0]);

  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    abort();
  } else if (pid > 0) {
    int sl = 0;
    assert(0 <= waitpid(pid, &sl, 0));
    if (WIFEXITED(sl)) return WEXITSTATUS(sl);
  }

  fprintf(stderr, "failed to run child process: %s\n", args[0]);
  return 1;
}

static int shader(char * name) {
  char spv[1024]; snprintf(spv, 1024, "puzzle.app/Contents/Resources/%s.spv", name);
  char * args[] = { "glslang", "-V", name, "-o", spv, 0 };
  return run(args);
}

static int pch() {
  char * args[] = {
    "clang", "-Wall", "-g", "-x", "c-header",
    "-IVulkan-Headers/include",
    "-D", "VK_USE_PLATFORM_METAL_EXT",
    "-D", "VLK_USE_VOLK",
    "-o", "pch.pch", "pch.h", 0 };
  return run(args);
}

static int cc(char * src, char * o) {
  char * args[] = {
    "clang", "-Wall", "-g", "-include-pch", "pch.pch",
    "-o", o, "-c", src, 0 };
  return run(args);
}

static int cm(char * src, char * o) {
  // It's nearly mandatory to use "modules" with ObjC.
  // The compilation speed without it is abismal.
  char * args[] = {
    "clang", "-Wall", "-g", "-fmodules",
    "-o", o, "-c", src, 0 };
  return run(args);
}

static int hdr(char * src, char * o, char * d) {
  char * args[] = {
    "clang", "-Wall", "-x", "c", "-g", "-include-pch", "pch.pch",
    "-D", d, "-o", o, "-c", src, 0
  };
  return run(args);
}

static int link_exe() {
  char * args[] = {
    "clang", "-Wall",
    "-framework", "AppKit",
    "-framework", "AudioToolbox",
    "-framework", "MetalKit",
    "-o", "puzzle.app/Contents/MacOS/puzzle", 
    "volk.o", "microui.o", "puzzle-osx.o",
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
  if (argc != 1) return (usage(), 1);

  if (pch()) return 1;

  if (hdr("volk.h", "volk.o", "VOLK_IMPLEMENTATION")) return 1;

  // if (hdr("snd.h", "snd.o", "SND_IMPL")) return 1;

  if (cc("microui.c", "microui.o")) return 1;

  if (app("puzzle")) return 1;
  if (cm("puzzle-osx.m", "puzzle-osx.o")) return 1;
  // if (hdr("vlk-puzzle.h", "vlk-puzzle.o", "VLK_IMPL")) return 1;
  if (link_exe()) return 1;

  if (shader("puzzle.frag")) return 1;
  if (shader("puzzle.vert")) return 1;

  return 0;
}
