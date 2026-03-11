// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/ios/animax_surface_ios.h"
#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/base/util/ios/ca_util.h"
#include "src/render/drawable_mtl.h"
#include "src/render/surface.h"

@interface animax_surface_ios_unittest : XCTestCase

@end

using namespace lynx::animax;

@implementation animax_surface_ios_unittest

- (void)setUp {
  // Put setup code here. This method is called before the invocation of each test method in the
  // class.
}

- (void)tearDown {
}

- (void)checkAnimaXSurface:(AnimaXSurface *)animax_surface
         createdWithCGSize:(CGSize)size
                      type:(AnimaXBackend)type {
  XCTAssertNotEqual(nullptr, animax_surface);
  XCTAssertEqual(type, animax_surface->Type());
  XCTAssertTrue(animax_surface->Valid());
  XCTAssertEqual(animax_surface->Width(), size.width);
  XCTAssertEqual(animax_surface->Height(), size.height);
  auto *canvas = animax_surface->Canvas();
  XCTAssertNotEqual(nullptr, canvas);
  animax_surface->Flush();
}

- (void)testMakeMetal {
  CAMetalLayer *layer = [[CAMetalLayer alloc] init];
  CGRect frame = CGRectMake(0, 0, 10, 10);
  layer.frame = frame;
  AnimaXSurfaceDrawable *drawable = [[AnimaXSurfaceDrawable alloc] initWithMetalLayer:layer
                                                                                scale:1];
  auto animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get()
         createdWithCGSize:frame.size
                      type:AnimaXBackend::kMetal];
  animax_surface.reset();
  animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get()
         createdWithCGSize:frame.size
                      type:AnimaXBackend::kMetal];
}

- (void)testReconfigureMetal {
  CAMetalLayer *layer = [[CAMetalLayer alloc] init];
  CGRect frame = CGRectMake(0, 0, 10, 10);
  layer.frame = frame;
  AnimaXSurfaceDrawable *drawable = [[AnimaXSurfaceDrawable alloc] initWithMetalLayer:layer
                                                                                scale:1];
  auto animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get()
         createdWithCGSize:frame.size
                      type:AnimaXBackend::kMetal];
  CGSize new_size = CGSizeMake(20, 20);
  auto new_surface = AnimaXSurfaceIOS::Reconfigure(std::move(animax_surface), {new_size});
  [self checkAnimaXSurface:new_surface.get() createdWithCGSize:new_size type:AnimaXBackend::kMetal];
}

- (void)testReconfigMetalWithoutChangingBufferSize {
  CAMetalLayer *layer = [[CAMetalLayer alloc] init];
  CGRect frame = CGRectMake(0, 0, 10, 10);
  layer.frame = frame;
  AnimaXSurfaceDrawable *drawable = [[AnimaXSurfaceDrawable alloc] initWithMetalLayer:layer
                                                                                scale:1];
  auto animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get()
         createdWithCGSize:frame.size
                      type:AnimaXBackend::kMetal];
  CGSize new_size = CGSizeMake(10, 10);
  auto new_surface = AnimaXSurfaceIOS::Reconfigure(std::move(animax_surface), {new_size});
  [self checkAnimaXSurface:new_surface.get() createdWithCGSize:new_size type:AnimaXBackend::kMetal];
}

- (void)testMakeImageMetal {
  auto size = CGSizeMake(10, 10);
  CVPixelBufferRef buffer = [AnimaXSurfaceDrawable createPixelBufferWithWidth:10
                                                                       height:10
                                                                      backend:AnimaXMetal];
  AnimaXSurfaceDrawable *drawable = [[AnimaXSurfaceDrawable alloc] initWithBuffer:buffer
                                                                             size:size
                                                                          backend:AnimaXMetal];

  auto animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get() createdWithCGSize:size type:AnimaXBackend::kMetal];
}

- (void)testReconfigureImageMetal {
  auto size = CGSizeMake(10, 10);
  CVPixelBufferRef buffer = [AnimaXSurfaceDrawable createPixelBufferWithWidth:10
                                                                       height:10
                                                                      backend:AnimaXMetal];
  AnimaXSurfaceDrawable *drawable = [[AnimaXSurfaceDrawable alloc] initWithBuffer:buffer
                                                                             size:size
                                                                          backend:AnimaXMetal];

  auto animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get() createdWithCGSize:size type:AnimaXBackend::kMetal];

  auto size2 = CGSizeMake(20, 20);
  animax_surface = AnimaXSurfaceIOS::Reconfigure(std::move(animax_surface), {size2});
  [self checkAnimaXSurface:animax_surface.get() createdWithCGSize:size2 type:AnimaXBackend::kMetal];
}

#ifdef ENABLE_IOS_SOFTWARE_RENDER
- (void)testMakeImageSoftware {
  auto size = CGSizeMake(10, 10);
  CVPixelBufferRef buffer = [AnimaXSurfaceDrawable createPixelBufferWithWidth:10
                                                                       height:10
                                                                      backend:AnimaXSoftware];
  AnimaXSurfaceDrawable *drawable = [[AnimaXSurfaceDrawable alloc] initWithBuffer:buffer
                                                                             size:size
                                                                          backend:AnimaXSoftware];

  auto animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get()
         createdWithCGSize:size
                      type:AnimaXBackend::kSoftware];
}

- (void)testReconfigureImageSoftware {
  auto size = CGSizeMake(10, 10);
  CVPixelBufferRef buffer = [AnimaXSurfaceDrawable createPixelBufferWithWidth:10
                                                                       height:10
                                                                      backend:AnimaXSoftware];
  AnimaXSurfaceDrawable *drawable = [[AnimaXSurfaceDrawable alloc] initWithBuffer:buffer
                                                                             size:size
                                                                          backend:AnimaXSoftware];

  auto animax_surface = AnimaXSurfaceIOS::Make(drawable);
  [self checkAnimaXSurface:animax_surface.get()
         createdWithCGSize:size
                      type:AnimaXBackend::kSoftware];

  auto size2 = CGSizeMake(20, 20);
  animax_surface = AnimaXSurfaceIOS::Reconfigure(std::move(animax_surface), {size2});
  [self checkAnimaXSurface:animax_surface.get()
         createdWithCGSize:size2
                      type:AnimaXBackend::kSoftware];
}
#endif

@end
