#import <CoreFoundation/CoreFoundation.h>
#import <MetalKit/MetalKit.h>

#include "gme.h"

static id<MTLLibrary> load_library(id<MTLDevice> device, NSString * name) {
  NSString * path = [[NSBundle mainBundle] pathForResource:name ofType:@"metal"];
  NSString * src = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
  MTLCompileOptions * opts = [MTLCompileOptions new];
  NSError * err;
  id<MTLLibrary> lib = [device newLibraryWithSource:src options:opts error:&err];
  if (err) {
    NSLog(@"Error compiling shader: %@", err);
    return nil;
  }
  return lib;
}

void gme_load_atlas(const char * name) {
}

@interface POCStuff : NSObject
@property (nonatomic,strong) id<MTLCommandQueue> queue;
@property (nonatomic,strong) id<MTLRenderPipelineState> pipeline;
@property (nonatomic,strong) id<MTLBuffer> grid;
@property (nonatomic,strong) id<MTLTexture> texture;
@property (nonatomic,strong) id<MTLSamplerState> sampler;
+ (id)newWithDevice:(id<MTLDevice>)device;
- (void)resize:(CGSize)size;
- (void)draw:(CGSize)size rpd:(MTLRenderPassDescriptor *)rpd into:(id<CAMetalDrawable>)drawable;
@end

@implementation POCStuff
+ (id)newWithDevice:(id<MTLDevice>)device {
  POCStuff * d = [POCStuff new];
  d.queue = [device newCommandQueue];
  d.grid = [device newBufferWithLength:GME_BUF_SIZE options:MTLResourceStorageModeShared];

  id<MTLLibrary> vert = load_library(device, @"shader.vert");
  id<MTLLibrary> frag = load_library(device, @"shader.frag");
  if (!vert || !frag) return nil;

  MTLRenderPipelineDescriptor * pd = [MTLRenderPipelineDescriptor new];
  pd.vertexFunction   = [vert newFunctionWithName:@"main0"];
  pd.fragmentFunction = [frag newFunctionWithName:@"main0"];
  pd.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
  NSError * err;
  d.pipeline = [device newRenderPipelineStateWithDescriptor:pd error:&err];
  if (err) return (NSLog(@"Error creating pipeline: %@", err), nil);

  MTLTextureDescriptor * td = [MTLTextureDescriptor new];
  td.pixelFormat = MTLPixelFormatRGBA8Unorm;
  td.width       = 1024;
  td.height      = 1024;
  d.texture = [device newTextureWithDescriptor:td];

  MTLSamplerDescriptor * sd = [MTLSamplerDescriptor new];
  sd.minFilter = sd.magFilter = MTLSamplerMinMagFilterLinear;
  d.sampler = [device newSamplerStateWithDescriptor:sd];

  return d;
}
- (void)resize:(CGSize)size {
  gme_resize(size.width, size.height);
}
- (void)draw:(CGSize)size rpd:(MTLRenderPassDescriptor *)rpd into:(id<CAMetalDrawable>)drawable {
  if (rpd == nil) return;

  gme_load(self.grid.contents);
  gme_frame();

  id<MTLCommandBuffer> cb = [self.queue commandBuffer];

  id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:rpd];
  [enc setRenderPipelineState:self.pipeline];
  [enc setVertexBytes:gme_pc() length:sizeof(gme_upc_t) atIndex:0];
  [enc setFragmentBytes:gme_pc() length:sizeof(gme_upc_t) atIndex:0];
  [enc setFragmentBuffer:self.grid offset:0 atIndex:1];
  [enc setFragmentTexture:self.texture atIndex:0];
  [enc setFragmentSamplerState:self.sampler atIndex:0];
  [enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
  [enc endEncoding];

  if (drawable) [cb presentDrawable:drawable];
  [cb commit];
  if (!drawable) [cb waitUntilCompleted];
}
@end

@interface POCViewDelegate : MTKView<MTKViewDelegate>
@property (nonatomic,strong) POCStuff * stuff;
@property (nonatomic) BOOL ready;
+ (id)new;
@end
@implementation POCViewDelegate
+ (id)new {
  POCViewDelegate * d = [[POCViewDelegate alloc] init];
  d.device     = MTLCreateSystemDefaultDevice();
  d.stuff      = [POCStuff newWithDevice:d.device];
  d.clearColor = MTLClearColorMake(0.01, 0.02, 0.03, 1.0);
  d.delegate   = d;
  return d;
}
- (BOOL)acceptsFirstResponder {
  return YES;
}
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
  if (self.ready) [self.stuff resize:size];
}
- (void)drawInMTKView:(MTKView *)view {
  if (!self.ready) {
    gme_init(view.frame.size.width, view.frame.size.height);
    self.ready = YES;
  }

  MTLRenderPassDescriptor * rpd = view.currentRenderPassDescriptor;
  [self.stuff draw:view.frame.size rpd:rpd into:view.currentDrawable];
}
@end
