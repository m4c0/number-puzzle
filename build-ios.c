#include <sys/stat.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// You can get this path with 'xcrun --show-sdk-path --sdk iphoneos'
#define SDK_PATH "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk"
#define TARGET "arm64-apple-ios26.0"

#define APP_PATH "export.xcarchive/Products/Applications/puzzle.app"

#define UPLOAD 0

static void usage() {
  fprintf(stderr, "just call 'build' without arguments\n");
}

static char * slurp(const char * file) {
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
  return data;
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

static int apply(char * src, char * tgt) {
  char * file = slurp(src);

  FILE * f = fopen(tgt, "wb");
  assert(f);

  char * p = file;
  while (*p) {
    p = strchr(file, '&');
    if (!p) break;

    assert(1 == fwrite(file, p-file, 1, f));
    file = ++p;

    char * pp = strchr(p, ';');
    if (!pp) {
      assert(0 == fputc('&', f));
      file++;
      continue;
    }
    *pp = 0;

    char * env = getenv(p);
    if (strncmp(p, "IOS_", 4)) {
      assert(fprintf(f, "&%s;", file));
      file = ++pp;
    } else if (env) {
      assert(fprintf(f, "%s", env));
      file = ++pp;
    } else {
      fprintf(stderr, "Missing environment: %s\n", p);
      exit(1);
    }
  }

  assert(fprintf(f, "%s", file));
  fclose(f);
  return 0;
}

static int shader(char * name) {
  char spv[1024];
  sprintf(spv, APP_PATH "/%s.spv", name);

  char * args[] = { "glslang", "-V", name, "-o", spv, 0 };
  return run(args);
}

static int codesign() {
  char * team = getenv("IOS_TEAM");
  assert(team && "Missing IOS_TEAM environment variable");

  char * args[] = { "codesign", "-f", "-s", strdup(team), APP_PATH, 0 };
  return run(args);
}
 
static int symbols() {
  char * args[] = {
    "dsymutil", 
    APP_PATH "/puzzle", 
    "-o", "export.xcarchive/dSYMS/puzzle.app.dSYM",
    0 };
  return run(args);
}

static int export() {
  char * args[] = {
    "xcodebuild", "-exportArchive",
    "-archivePath", "export.xcarchive",
    "-exportPath", "export",
    "-exportOptionsPlist", "export.plist",
    0 };
  return run(args);
}

static int actool() {
  char * args[] = {
    "actool",
    "--notices", "--warnings", "--errors",
    "--output-format", "human-readable-text",
    "--app-icon", "AppIcon",
    "--accent-color", "AccentColor",
    "--compress-pngs",
    "--enable-on-demand-resources", "YES",
    "--target-device", "iphone",
    "--target-device", "ipad",
    "--platform", "iphoneos",
    //"--filter-for-thinning-device-configuration", "iPhone16,1"
    //"--filter-for-device-os-version", "17.0"
    "--development-region", "en",
    "--minimum-deployment-target", "26",
    "--output-partial-info-plist", "icon-partial.plist",
    "--compile", APP_PATH,
    "Assets.xcassets",
    0
  };
  return run(args);
}

static int install() {
  char * device = getenv("IOS_DEVICE");
  if (!device) {
    fprintf(stderr, "Missing IOS_DEVICE - skipping install\n");
    return 0;
  }

  char * args[] = {
    "xcrun", "devicectl", "device", "install", "app", "--device", device, "export/puzzle.ipa", 0
  };
  return run(args);
}

static int validate(char * verb) {
  char * api_key = getenv("IOS_API_KEY");
  assert(api_key && "Missing IOS_API_KEY environment variable");
  char * api_issuer = getenv("IOS_API_ISSUER");
  assert(api_issuer && "Missing IOS_API_ISSUER environment variable");

  char * args[] = {
    "xcrun", "altool", verb, "-t", "iphoneos",
    "-f", "export/puzzle.ipa",
    "--apiKey", strdup(api_key),
    "--apiIssuer", strdup(api_issuer),
    0 };
  return run(args);
}

static int pch() {
  char * args[] = {
    "clang", "-Wall", "-O3", "-x", "c-header",
    "-target", TARGET, "-isysroot", SDK_PATH,
    "-IVulkan-Headers/include",
    "-D", "VK_USE_PLATFORM_METAL_EXT",
    "-o", "pch.pch", "pch.h", 0 };
  return run(args);
}

static int cc(char * src, char * o) {
  char * args[] = {
    "clang", "-Wall", "-O3", "-target", TARGET, "-isysroot", SDK_PATH,
    "-include-pch", "pch.pch", "-o", o, "-c", src, 0 };
  return run(args);
}

static int cm(char * src, char * o) {
  char * args[] = {
    "clang", "-Wall", "-O3", "-target", TARGET, "-isysroot", SDK_PATH,
    "-fmodules", "-o", o, "-c", src, 0 };
  return run(args);
}

static int hdr(char * src, char * o, char * d) {
  char * args[] = {
    "clang", "-Wall", "-O3", "-target", TARGET, "-isysroot", SDK_PATH,
    "-include-pch", "pch.pch",
    "-x", "c", "-g", "-D", d, "-o", o, "-c", src, 0
  };
  return run(args);
}

static int link_exe() {
  char * args[] = {
    "clang", "-Wall", "-O3", "-target", TARGET, "-isysroot", SDK_PATH,
    "-framework", "AudioToolbox",
    "-framework", "CoreFoundation",
    "-framework", "CoreGraphics",
    "-framework", "Foundation",
    "-framework", "IOSurface",
    "-framework", "Metal",
    "-framework", "MetalKit",
    "-framework", "QuartzCore",
    "-framework", "UIKit",
    "-o", APP_PATH "/puzzle", 
    "sfx.o", "snd.o", "vlk.o",
    "stb_image.o", "puzzle-ios.o",
    "MoltenVK.xcframework/ios-arm64/libMoltenVK.a",
    "-lc++",
    0 };
  return run(args);
}

static int cp(char * name) {
  char * args[] = { "cp", name, APP_PATH, 0 };
  return run(args);
}

int main(int argc, char ** argv) {
  if (argc != 1) return (usage(), 1);

  mkdir("export.xcarchive", 0777);
  mkdir("export.xcarchive/Products", 0777);
  mkdir("export.xcarchive/Products/Applications", 0777);
  mkdir(APP_PATH, 0777);

  if (pch()) return 1;

  if (hdr("stb_image.h", "stb_image.o", "STB_IMAGE_IMPLEMENTATION")) return 1;

  if (hdr("sfx.h", "sfx.o", "SFX_IMPL")) return 1;
  if (hdr("snd.h", "snd.o", "SND_IMPL")) return 1;
  if (hdr("vlk.h", "vlk.o", "VLK_IMPL")) return 1;

  if (cm("puzzle-ios.m", "puzzle-ios.o")) return 1;
  if (link_exe()) return 1;

  if (shader("shader.frag")) return 1;
  if (shader("shader.vert")) return 1;

  if (apply("export.plist.in",    "export.plist")) return 1;
  if (apply("xcarchive.plist.in", "export.xcarchive/Info.plist")) return 1;
  if (apply("app.plist.in",       "export.xcarchive/Products/Applications/puzzle.app/Info.plist")) return 1;

  for (int i = 1; i <= 31; i++) {
    char buf[128];
    snprintf(buf, 128, "imgs/bg-%003d.jpg", i);
    if (cp(buf)) return 1;
  }

  if (getenv("IOS_BUILD_ONLY")) return 0;

  if (actool())   return 1;
  if (codesign()) return 1;
  if (symbols())  return 1;
  if (export())   return 1;
#if UPLOAD
  if (validate("--upload-app")) return 1;
#else
  if (install())  return 1;
  if (validate("--validate-app")) return 1;
#endif

  return 0;
}
