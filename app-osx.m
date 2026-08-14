#import <AppKit/AppKit.h>

#include "mtl.h"

@interface POCViewController : NSViewController
@end
@implementation POCViewController
- (void) mouseDown:(NSEvent *)event {
  CGPoint liw = [event locationInWindow];
  CGPoint p = [self.view convertPoint:liw fromView:nil];
  gme_mouse_down(p.x, self.view.frame.size.height - p.y);
}
- (void) mouseMoved:(NSEvent *)event {
  CGPoint liw = [event locationInWindow];
  CGPoint p = [self.view convertPoint:liw fromView:nil];
  gme_mouse_move(p.x, self.view.frame.size.height - p.y);
}
- (void) mouseDragged:(NSEvent *)event {
  CGPoint liw = [event locationInWindow];
  CGPoint p = [self.view convertPoint:liw fromView:nil];
  gme_mouse_move(p.x, self.view.frame.size.height - p.y);
}
@end

@interface POCAppDelegate : NSObject<NSApplicationDelegate>
@end
@implementation POCAppDelegate
- (void)applicationWillTerminate:(NSApplication *)app {
  gme_deinit();
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
  return YES;
}
@end

static void run() {
  POCViewController * vc = [POCViewController new];
  vc.view = [POCViewDelegate new];

  NSWindow * w = [NSWindow new];
  w.acceptsMouseMovedEvents = YES;
  w.contentViewController = vc;
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
