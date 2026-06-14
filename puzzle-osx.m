#import <AppKit/AppKit.h>
#import <CoreFoundation/CoreFoundation.h>
#import <MetalKit/MetalKit.h>

#include "vlk.h"

@interface POCViewDelegate : NSObject<MTKViewDelegate>
@property (nonatomic) BOOL ready;
@end
@implementation POCViewDelegate
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
}
- (void)drawInMTKView:(MTKView *)view {
  if (!self.ready) {
    vlk_init();
    self.ready = YES;
  }
  vlk_frame();
}
@end

@interface POCView : MTKView
@end
@implementation POCView
- (BOOL)acceptsFirstResponder {
  return YES;
}
- (void) mouseDown:(NSEvent *)event {
  CGPoint liw = [event locationInWindow];
  CGPoint p = [self convertPoint:liw fromView:nil];
  vlk_mouse_down(p.x, self.frame.size.height - p.y);
}
// - (void) mouseUp:(NSEvent *)event {
//   CGPoint liw = [event locationInWindow];
//   CGPoint p = [self convertPoint:liw fromView:nil];
//   mu_input_mouseup(&mui_ctx, p.x, self.frame.size.height - p.y, 1);
// }
- (void) mouseMoved:(NSEvent *)event {
  CGPoint liw = [event locationInWindow];
  CGPoint p = [self convertPoint:liw fromView:nil];
  vlk_mouse_move(p.x, self.frame.size.height - p.y);
}
// - (void) mouseDragged:(NSEvent *)event {
//   CGPoint liw = [event locationInWindow];
//   CGPoint p = [self convertPoint:liw fromView:nil];
//   mu_input_mousemove(&mui_ctx, p.x, self.frame.size.height - p.y);
// }
@end

@interface POCAppDelegate : NSObject<NSApplicationDelegate>
@end
@implementation POCAppDelegate
- (void)applicationWillTerminate:(NSApplication *)app {
  vlk_deinit();
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
  return YES;
}
@end

CAMetalLayer * vlk_metal_layer() {
  return (CAMetalLayer *)[NSApplication sharedApplication].windows[0].contentView.layer;
}

FILE * vlk_open(const char * name, const char * ext) {
  NSString * n = [NSString stringWithFormat:@"%s", name];
  NSString * e = [NSString stringWithFormat:@"%s", ext];
  NSString * path = [[NSBundle mainBundle] pathForResource:n ofType:e];
  return fopen(path.UTF8String, "rb");
}

void sav_get_path(char * buf, unsigned buf_sz) {
  NSArray * arr = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
  NSString * dir = [arr firstObject];
  [[NSFileManager defaultManager] createDirectoryAtPath:dir
                            withIntermediateDirectories:YES
                                             attributes:nil
                                                  error:nil];
  strncpy(buf, dir.UTF8String, buf_sz);
}

void vlk_log(int r, const char * msg) {
  NSLog(@"Vulkan call failed (code=%d): %s\n", r, msg);
  exit(1);
}

static void run() {
  MTKView * v = [POCView new];
  v.delegate = [POCViewDelegate new];

  NSWindow * w = [NSWindow new];
  w.acceptsMouseMovedEvents = YES;
  w.contentView = v;
  w.styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

  NSRect crect = NSMakeRect(0, 0, 800, 600);
  NSRect frect = [w frameRectForContentRect:crect];
  [w setFrame:frect display:YES];
  [w center];
  [w makeKeyAndOrderFront:w];

  // Apple menu
  NSMenu * menu = [NSMenu new];
  [menu       addItem:[[NSMenuItem alloc]
        initWithTitle:@"Quit Sokoban"
               action:@selector(terminate:)
        keyEquivalent:@"q"]];

  NSMenuItem * item = [NSMenuItem new];
  item.submenu = menu;

  NSMenu * bar = [NSMenu new];
  [bar addItem:item];

  NSApplication * a = [NSApplication sharedApplication];
  a.delegate = [POCAppDelegate new];
  a.mainMenu = bar;
  [a activateIgnoringOtherApps:YES];
  [a run];
}

int main() {
  @autoreleasepool {
    run();
  }
}
