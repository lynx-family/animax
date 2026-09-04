// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXSurfaceDrawable.h>
#import "AnimaXSurfaceDrawable+Internal.h"
#import "CVPixelBufferWrapper.h"

#include "src/base/log/log.h"
#include "src/base/util/ios/ca_util.h"

@interface AnimaXSurfaceDrawable ()

@property(nonatomic, readwrite) AnimaXRenderTarget target;
@property(nonatomic, readwrite) AnimaXRenderBackend backend;

@property(nonatomic, weak, readwrite) CAMetalLayer* layer;
@property(nonatomic, strong, readwrite) CVPixelBufferWrapper* bufferWrapper;

@property(nonatomic, readwrite) CGSize size;

@end

@implementation AnimaXSurfaceDrawable

#pragma mark Init

- (instancetype)initWithBuffer:(nonnull CVPixelBufferRef)buffer
                          size:(CGSize)size
                       backend:(AnimaXRenderBackend)backend {
  if (self = [super init]) {
    self.target = Buffer;
    self.size = size;
    self.bufferWrapper = [[CVPixelBufferWrapper alloc] init];
    self.backend = backend;
    [self setBuffer:buffer];
  }
  return self;
}

- (instancetype)initWithView:(nonnull UIView<AnimaXPixelBufferUpdateListener>*)view
                        size:(CGSize)size
                     backend:(AnimaXRenderBackend)backend {
  if (self = [super init]) {
    self.target = View;
    self.size = size;
    self.bufferWrapper = [[CVPixelBufferWrapper alloc] initWithView:view];
    self.backend = backend;
    if (backend == AnimaXMetal) {
      CVPixelBufferRef buffer = [self createPixelBufferWithWidth:size.width height:size.height];
      [self setBuffer:buffer];
    }
  }
  return self;
}

- (instancetype)initWithMetalLayer:(nonnull CAMetalLayer*)layer scale:(CGFloat)scale {
  if (self = [super init]) {
    self.layer = layer;
    self.target = Layer;
    self.backend = AnimaXMetal;
    self.size = CGSizeMake(layer.frame.size.width * scale, layer.frame.size.height * scale);
  }
  return self;
}

#pragma mark Setter

- (void)setBackend:(AnimaXRenderBackend)backend {
  _backend = backend;
  if (self.bufferWrapper) {
    self.bufferWrapper.backend = backend;
  }
}

- (void)setBuffer:(nonnull CVPixelBufferRef)buffer {
  if (!self.bufferWrapper) {
    return;
  }
  // When target is Buffer, the caller retains ownership of the buffer and is responsible
  // for its lifecycle. We wrap with an additional retain (newWrap) so our scope holds
  // its own independent reference.
  // When target is View, the buffer is an internally managed intermediate product with
  // no external owner. We take over ownership directly (newWrapOwned) without extra retain.
  if (self.target == Buffer) {
    self.bufferWrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrap:buffer];
  } else if (self.target == View) {
    self.bufferWrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:buffer];
  }
  self.bufferWrapper.generation++;
}

- (void)setFrameAvailableHandler:
    (nullable AnimaXPixelBufferFrameAvailableHandler)frameAvailableHandler {
  self.bufferWrapper.frameAvailableHandler = frameAvailableHandler;
}

#pragma mark Reconfigure

/**
 * The layout engine might generate nearly identical widths or heights consecutively.
 * For example, it might first generate a width of 129, then 130, causing the surface to constantly
 * recreate after each layout, leading to performance issues. This method helps to avoid such
 * situations by returning YES only if the absolute difference in either dimension is within a
 * predefined limit.
 *
 * @param newWidth The new width to be compared.
 * @param newHeight The new height to be compared.
 * @return YES if the absolute difference in both width and height are within the limit, NO
 * otherwise.
 */
- (BOOL)isSizeChangeWithinRange:(CGFloat)newWidth height:(CGFloat)newHeight {
  return (abs(self.size.width - newWidth) <= 1.5f && abs(self.size.height - newHeight) <= 1.5f);
}

- (BOOL)updateSize:(CGSize)size {
  if ([self isSizeChangeWithinRange:size.width height:size.height]) {
    ANIMAX_LOGI("Skip same size, width: " << size.width << " height: " << size.height)
    return NO;
  }
  ANIMAX_LOGI("Update size success, width: " << size.width << " height: " << size.height)
  self.size = size;
  if (self.target == View && self.backend == AnimaXMetal) {
    CVPixelBufferRef buffer = [self createPixelBufferWithWidth:size.width height:size.height];
    [self setBuffer:buffer];
  }
  return YES;
}

#pragma mark CVPixelBuffer Factory

- (CVPixelBufferRef)createPixelBufferWithWidth:(size_t)width height:(size_t)height {
  return [CVPixelBufferWrapper createPixelBufferWithWidth:width height:height backend:self.backend];
}

+ (CVPixelBufferRef)createPixelBufferWithWidth:(size_t)width
                                        height:(size_t)height
                                       backend:(AnimaXRenderBackend)backend {
  return [CVPixelBufferWrapper createPixelBufferWithWidth:width height:height backend:backend];
}

@end
