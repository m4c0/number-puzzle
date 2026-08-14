// You can get this path with 'xcrun --show-sdk-path --sdk iphoneos'
#define SDK_PATH "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk"
#define TARGET "arm64-apple-ios26.0"

#define CFLAGS "-g", "-O3", "-target", TARGET, "-isysroot", SDK_PATH
#define RES_PATH(X) "export.xcarchive/Products/Applications/"X".app"
#include "build.h"

#include <time.h>

static time_t bundle_version;
static int uploading;

static void print_key(FILE * f, const char * p) {
  char * env = getenv(p);
  if (strncmp(p, "IOS_", 4)) {
    assert(fprintf(f, "&%s;", p));
  } else if (0 == strcmp(p, "IOS_APP_NAME")) {
    assert(fprintf(f, APP));
  } else if (0 == strcmp(p, "IOS_BUNDLE_VERSION")) {
    assert(fprintf(f, "%ld", bundle_version));
  } else if (0 == strcmp(p, "IOS_METHOD")) {
    if (uploading) {
      assert(fprintf(f, "app-store-connect"));
    } else {
      assert(fprintf(f, "debugging"));
    }
  } else if (env) {
    assert(fprintf(f, "%s", env));
  } else {
    fprintf(stderr, "Missing environment: %s\n", p);
    exit(1);
  }
}

static int codesign() {
  char * team = getenv("IOS_TEAM");
  assert(team && "Missing IOS_TEAM environment variable");

  RUN("codesign", "-f", "-s", strdup(team), RES_PATH(APP));
  return 0;
}
 
static int symbols() {
  RUN("dsymutil", RES_PATH(APP)"/"APP, "-o", "export.xcarchive/dSYMS/"APP".app.dSYM");
  return 0;
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
    "--compile", RES_PATH(APP),
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
    "-o", RES_PATH(APP)"/"APP, 
    "sfx.o", "snd.o", "vlk.o",
    "stb_image.o", "puzzle-ios.o",
    "MoltenVK.xcframework/ios-arm64/libMoltenVK.a",
    "-lc++",
    0 };
  return run(args);
}

int main(int argc, char ** argv) {
  bundle_version = time(NULL);
  uploading = getenv("IOS_UPLOAD") != NULL;

  mkdir("export.xcarchive", 0777);
  mkdir("export.xcarchive/Products", 0777);
  mkdir("export.xcarchive/Products/Applications", 0777);
  mkdir(RES_PATH(APP), 0777);

  if (pch()) return 1;

  HDR("stb_image", "STB_IMAGE_IMPLEMENTATION");
  HDR("vlk",       "VLK_IMPL");

  CM("puzzle-ios");
  if (compile_and_link_exe()) return 1;

  if (shaders()) return 1;

  if (apply("export.plist.in",    "export.plist")) return 1;
  if (apply("xcarchive.plist.in", "export.xcarchive/Info.plist")) return 1;
  if (apply("app.plist.in",       "export.xcarchive/Products/Applications/puzzle.app/Info.plist")) return 1;

  for (int i = 1; i <= 31; i++) {
    char buf[128];
    snprintf(buf, 128, "imgs/bg-%003d.jpg", i);
    RUN("cp", buf, RES_PATH(APP));
  }

  if (getenv("IOS_BUILD_ONLY")) return 0;

  if (actool())   return 1;
  if (codesign()) return 1;
  if (symbols())  return 1;
  if (export())   return 1;

  if (uploading) {
    if (validate("--upload-app")) return 1;
  } else {
    if (install()) return 1;
    if (validate("--validate-app")) return 1;
  }

  return 0;
}
