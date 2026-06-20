#import <CoreFoundation/CoreFoundation.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

#include "vlk.h"

CAMetalLayer * g_layer;

@interface POCViewDelegate : NSObject<MTKViewDelegate>
@property (nonatomic) BOOL ready;
@end
@implementation POCViewDelegate
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
}
- (void)drawInMTKView:(MTKView *)view {
  if (!self.ready) {
    g_layer = (CAMetalLayer *)view.layer;

    vlk_init(1);
    self.ready = YES;
  }
  vlk_frame();
}
@end

@interface POCViewController : UIViewController
@end
@implementation POCViewController
- (BOOL)canBecomeFirstResponder {
  return YES;
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event {
  vlk_reset();
}

- (void)touchesBegan:(NSSet<UITouch *> *) touches withEvent:(UIEvent *) event {
  CGPoint p = [[touches anyObject] locationInView:[self view]];
  vlk_mouse_move(p.x, p.y);
  vlk_mouse_down(p.x, p.y);
}
@end

@interface POCWindowSceneDelegate : NSObject<UIWindowSceneDelegate>
@property (nonatomic, strong) UIWindow * window;
@end
@implementation POCWindowSceneDelegate
- (void) scene:(UIScene *) scene willConnectToSession:(UISceneSession *) session options:(UISceneConnectionOptions *) connectionOptions
{
  UIWindowScene * windowScene = (UIWindowScene *)scene;

  MTKView * view = [MTKView new];
  view.delegate = [POCViewDelegate new];

  POCViewController * vc = [POCViewController new];
  vc.view = view;

  self.window = [[UIWindow alloc] initWithWindowScene:windowScene];
  self.window.rootViewController = vc;
  [self.window makeKeyAndVisible];
}
@end

@interface POCAppDelegate : NSObject<UIApplicationDelegate>
@end
@implementation POCAppDelegate
- (UISceneConfiguration *) application:(UIApplication *) application 
configurationForConnectingSceneSession:(UISceneSession *) connectingSceneSession 
                               options:(UISceneConnectionOptions *) options
{
  application.applicationSupportsShakeToEdit = YES;

  UISceneConfiguration * res = [[UISceneConfiguration alloc] initWithName:@"Default"
                                                              sessionRole:connectingSceneSession.role];
  res.sceneClass = [UIWindowScene class];
  res.delegateClass = [POCWindowSceneDelegate class];
  return res;
}

- (void)applicationWillTerminate:(UIApplication *)app 
{
  // TODO: is this still the right place in this UIScene world?
  vlk_deinit();
}
@end

CAMetalLayer * vlk_metal_layer() { return g_layer; }

__strong static NSData * last_resource;
unsigned vlk_open(const char * name, const char * ext, const void ** ptr) {
  NSString * n = [NSString stringWithFormat:@"%s", name];
  NSString * e = [NSString stringWithFormat:@"%s", ext];
  NSString * path = [[NSBundle mainBundle] pathForResource:n ofType:e];
  last_resource = [NSData dataWithContentsOfFile:path];
  *ptr = [last_resource bytes];
  return [last_resource length];
}

void vlk_log(int r, const char * msg) {
  NSLog(@"Vulkan call failed (code=%d): %s\n", r, msg);
  exit(1);
}

int main(int argc, char ** argv) {
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil, @"POCAppDelegate");
  }
}
