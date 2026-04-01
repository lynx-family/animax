// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "CVPixelBufferWrapper.h"

#import <CoreVideo/CVMetalTextureCache.h>

#include "include/player/animax_surface.h"
#include "src/base/log/log.h"
#include "src/base/util/buffer_copy_helper.h"

ANIMAX_SCOPED_OBJECT_IMPLEMENTATION(AnimaXScopedCVPixelBuffer, CVPixelBufferRef,
                                    CVPixelBufferRetain, CVPixelBufferRelease)

@interface CVPixelBufferWrapper ()

@property(atomic, readwrite) NSUInteger generation;
@property(atomic, readwrite, nullable) id<MTLTexture> metalTexture;

@property(nonatomic, assign, nullable) CVMetalTextureCacheRef textureCache;
@property(nonatomic, weak, nullable) UIView<AnimaXPixelBufferUpdateListener> *targetView;
@property(nonatomic, assign, nullable) CVPixelBufferPoolRef pixelBufferPool;

@end

@implementation CVPixelBufferWrapper

#pragma mark - Lifecycle

- (instancetype)initWithView:(UIView<AnimaXPixelBufferUpdateListener> *)view {
  self = [super init];
  if (self) {
    self.targetView = view;
    self.backend = AnimaXMetal;
  }
  return self;
}

- (instancetype)init {
  self = [super init];
  if (self) {
    self.backend = AnimaXMetal;
  }
  return self;
}

- (void)dealloc {
  [self destroyAll];
}

#pragma mark - Destruction
- (void)destroyTexture {
  self.metalTexture = NULL;
}

- (void)destroyTextureCache {
  if (self.textureCache) {
    CVMetalTextureCacheFlush(self.textureCache, 0);
    CFRelease(self.textureCache);
    self.textureCache = NULL;
  }
}

- (void)destroyPixelBuffers {
  self.renderPixelBufferScope = nil;
  self.displayPixelBufferScope = nil;
  self.pixelBufferPool = NULL;
}

- (void)destroyAll {
  [self destroyTexture];
  [self destroyTextureCache];
  [self destroyPixelBuffers];
}

#pragma mark - Public API

+ (CVPixelBufferRef)createPixelBufferWithWidth:(size_t)width
                                        height:(size_t)height
                                       backend:(AnimaXRenderBackend)backend {
  NSMutableDictionary *attrs = [@{
    // Enable to create CGImage
    (id)kCVPixelBufferCGImageCompatibilityKey : @YES,
    (id)kCVPixelBufferCGBitmapContextCompatibilityKey : @YES,
  } mutableCopy];
  if (backend == AnimaXMetal) {
    // Interact with Metal
    attrs[(id)kCVPixelBufferMetalCompatibilityKey] = @YES;
    // Interact with OpenGL ES
    attrs[(id)kCVPixelBufferOpenGLESCompatibilityKey] = @YES;
    // Zero copy. Currently useless when software rendering.
    attrs[(id)kCVPixelBufferIOSurfacePropertiesKey] = @{};
  }

  CVPixelBufferRef buffer = NULL;
  CVReturn rc = CVPixelBufferCreate(kCFAllocatorDefault, width, height, kCVPixelFormatType_32BGRA,
                                    (__bridge CFDictionaryRef)attrs, &buffer);
  return (rc == kCVReturnSuccess) ? buffer : NULL;
}

- (void)notifyBufferUpdateWithGeneration:(NSUInteger)currentGeneration {
  DCHECK(self.backend == AnimaXMetal);

  if (!self.targetView) {
    return;
  }

  AnimaXScopedCVPixelBuffer *bufferScoped = self.renderPixelBufferScope;
  CVPixelBufferRef srcBuffer = bufferScoped.object;
  if (!srcBuffer) {
    return;
  }

  const size_t width = CVPixelBufferGetWidth(srcBuffer);
  const size_t height = CVPixelBufferGetHeight(srcBuffer);
  const size_t stride = CVPixelBufferGetBytesPerRow(srcBuffer);
  uint8_t *srcPixels = NULL;

  if (CVPixelBufferLockBaseAddress(srcBuffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess) {
    srcPixels = (uint8_t *)CVPixelBufferGetBaseAddress(srcBuffer);
    [self notifyBufferUpdateWithGeneration:currentGeneration
                                 srcPixels:srcPixels
                                     width:width
                                    height:height
                                    stride:stride];
    CVPixelBufferUnlockBaseAddress(srcBuffer, kCVPixelBufferLock_ReadOnly);
  } else {
    ANIMAX_LOGE("Failed to notify buffer updated: unable to lock src pixel buffer.")
  }
}

- (void)notifyBufferUpdateWithGeneration:(NSUInteger)currentGeneration
                               srcPixels:(nullable uint8_t *)srcPixels
                                   width:(size_t)width
                                  height:(size_t)height
                                  stride:(size_t)stride {
  if (!(srcPixels && width && height && stride)) {
    ANIMAX_LOGE("Failed to notify buffer updated: invalid pixel info.")
    return;
  }

  if (currentGeneration != self.generation) {
    return;
  }

  if (self.backend == AnimaXSoftware && !self.targetView) {
    // only here if using software backend to render buffer.
    AnimaXScopedCVPixelBuffer *bufferScoped = self.renderPixelBufferScope;
    [self copyFromPixels:srcPixels
                   width:width
                  height:height
                  stride:stride
                toBuffer:bufferScoped.object];
    return;
  }

  CVPixelBufferRef distBuffer = [self acquirePixelBufferFromPool];
  if (!distBuffer) {
    distBuffer = [CVPixelBufferWrapper createPixelBufferWithWidth:width
                                                           height:height
                                                          backend:self.backend];
  }

  [self copyFromPixels:srcPixels width:width height:height stride:stride toBuffer:distBuffer];
  self.displayPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:distBuffer];

  __weak typeof(self) weakSelf = self;
  dispatch_async(dispatch_get_main_queue(), ^{
    __strong typeof(weakSelf) self = weakSelf;
    if (!self) {
      return;
    }

    if (currentGeneration != self.generation) {
      return;
    }
    UIView<AnimaXPixelBufferUpdateListener> *view = self.targetView;
    if (!view) {
      return;
    }
    [view onBufferUpdated:self.displayPixelBufferScope];
  });
}

- (CVPixelBufferRef)acquirePixelBufferFromPool {
  if (!self.pixelBufferPool) {
    return NULL;
  }
  CVPixelBufferRef buffer = NULL;
  CVReturn rc =
      CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, self.pixelBufferPool, &buffer);
  if (rc != kCVReturnSuccess) {
    buffer = NULL;
    ANIMAX_LOGE("CVPixelBufferPoolCreatePixelBuffer failed.")
  }
  return buffer;
}

- (void)resizePixelBufferWrapperWithWidth:(size_t)width height:(size_t)height {
  [self rebuildPixelBufferPoolWithWidth:width height:height];
}

#pragma mark - CVPixelBuffer

- (void)copyFromPixels:(uint8_t *)pixels
                 width:(size_t)width
                height:(size_t)height
                stride:(size_t)stride
              toBuffer:(CVPixelBufferRef)buffer {
  DCHECK(pixels && width && height && stride);
  if (buffer == NULL) {
    ANIMAX_LOGE("Failed to copy from pixels: invalid buffer.")
    return;
  }
  size_t dstWidth = CVPixelBufferGetWidth(buffer);
  size_t dstHeight = CVPixelBufferGetHeight(buffer);
  if (!lynx::animax::BufferCopyHelper::IsSizeMatch(width, height, dstWidth, dstHeight)) {
    return;
  }
  size_t dstStride = CVPixelBufferGetBytesPerRow(buffer);
  CVReturn rc = CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
  if (rc != kCVReturnSuccess) {
    ANIMAX_LOGE("Failed to copy from pixels: unable to lock buffer.")
    return;
  }
  uint8_t *dstPixels = (uint8_t *)CVPixelBufferGetBaseAddress(buffer);
  if (!dstPixels) {
    ANIMAX_LOGE("Failed to copy from pixels: unable to get pixels of buffer.")
    return;
  }
  lynx::animax::BufferCopyHelper::CopyBuffer(pixels, dstPixels, height, stride, dstStride);
  CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
}

- (void)setRenderPixelBufferScope:(AnimaXScopedCVPixelBuffer *)renderPixelBufferScope {
  if (_renderPixelBufferScope.object == renderPixelBufferScope.object) {
    return;
  }

  _renderPixelBufferScope = renderPixelBufferScope;
  if (renderPixelBufferScope.object) {
    self.generation++;
    if (self.backend == AnimaXMetal) {
      [self destroyTexture];
      [self rebuildMetalTextureIfNeeded];
    }
  }
}

- (void)rebuildPixelBufferPoolWithWidth:(size_t)width height:(size_t)height {
  NSDictionary *poolAttributes = @{(id)kCVPixelBufferPoolMinimumBufferCountKey : @2};
  NSMutableDictionary *pixelBufferAttributes = [@{
    (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
    (id)kCVPixelBufferWidthKey : @(width),
    (id)kCVPixelBufferHeightKey : @(height),
    (id)kCVPixelBufferCGImageCompatibilityKey : @YES,
    (id)kCVPixelBufferCGBitmapContextCompatibilityKey : @YES,
  } mutableCopy];
  if (self.backend == AnimaXMetal) {
    pixelBufferAttributes[(id)kCVPixelBufferMetalCompatibilityKey] = @YES;
    pixelBufferAttributes[(id)kCVPixelBufferOpenGLESCompatibilityKey] = @YES;
    pixelBufferAttributes[(id)kCVPixelBufferIOSurfacePropertiesKey] = @{};
  }
  CVPixelBufferPoolRef pool = NULL;
  CVReturn rc =
      CVPixelBufferPoolCreate(kCFAllocatorDefault, (__bridge CFDictionaryRef)poolAttributes,
                              (__bridge CFDictionaryRef)pixelBufferAttributes, &pool);
  if (rc != kCVReturnSuccess) {
    ANIMAX_LOGE("CVPixelBufferPoolCreate failed.")
  } else {
    self.pixelBufferPool = pool;
  }
  CVPixelBufferPoolRelease(pool);
}

- (void)setPixelBufferPool:(CVPixelBufferPoolRef)newPool {
  // only accessed in GPU thread.
  if (_pixelBufferPool == newPool) {
    return;
  }
  CVPixelBufferPoolRetain(newPool);
  CVPixelBufferPoolRelease(_pixelBufferPool);
  _pixelBufferPool = newPool;
}

#pragma mark - Metal Texture

- (void)createTextureCacheIfNeeded {
  if (self.textureCache) {
    return;
  }

  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  if (!device) {
    return;
  }
  CVMetalTextureCacheRef cache = nullptr;
  CVReturn rc = CVMetalTextureCacheCreate(kCFAllocatorDefault, nil, device, nil, &cache);
  if (rc == kCVReturnSuccess && cache) {
    self.textureCache = cache;
  } else if (cache) {
    CFRelease(cache);
  }
}

- (void)rebuildMetalTextureIfNeeded {
  DCHECK(self.backend == AnimaXMetal);

  AnimaXScopedCVPixelBuffer *bufferScoped = self.renderPixelBufferScope;
  CVPixelBufferRef buffer = bufferScoped.object;
  if (!buffer) {
    return;
  }

  [self createTextureCacheIfNeeded];
  if (!self.textureCache) {
    return;
  }

  const size_t w = CVPixelBufferGetWidth(buffer);
  const size_t h = CVPixelBufferGetHeight(buffer);

  CVMetalTextureRef cvTex = nullptr;
  CVReturn rc =
      CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault, self.textureCache, buffer, nil,
                                                MTLPixelFormatBGRA8Unorm, w, h, 0, &cvTex);
  if (rc == kCVReturnSuccess && cvTex) {
    self.metalTexture = CVMetalTextureGetTexture(cvTex);
  }

  if (cvTex) {
    CFRelease(cvTex);
  }
}

@end
