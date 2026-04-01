// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXRenderTypes.h>
#import <AnimaX/AnimaXScopedObject.h>
#import <CoreVideo/CVMetalTextureCache.h>
#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

ANIMAX_SCOPED_OBJECT_INTERFACE(AnimaXScopedCVPixelBuffer, CVPixelBufferRef)

@protocol AnimaXPixelBufferUpdateListener <NSObject>

- (void)onBufferUpdated:(AnimaXScopedCVPixelBuffer *)bufferScope;

@end

/**
 CVPixelBufferWrapper
 - A wrapper that manages a CVPixelBufferRef and its associated Metal texture.
 - The object starts empty after init. Resources are created or re-created on demand via
 resizeToWidth:height:.
 - Resize only reallocates; it does not preserve previous content.
 - After a successful resize, pixelBuffer and metalTexture become available.
 - Note: Callers can perform CPU writes on pixelBuffer (lock/unlock) or GPU rendering on
 metalTexture.
 */
@interface CVPixelBufferWrapper : NSObject

@property(nonatomic, readwrite) AnimaXRenderBackend backend;

// Underlying pixel buffer scope (NULL until a successful resize).
// Note: In software mode, if the wrapper is initialized with a view, it won't have a
// renderPixelBufferScope.
@property(atomic, strong, nullable) AnimaXScopedCVPixelBuffer *renderPixelBufferScope;

// Only available when this wrapper is init with a View.
// The surface produces a copy of renderPixelBufferScope.object and pushes it on
// displayPixelBufferScope.
@property(atomic, strong, nullable) AnimaXScopedCVPixelBuffer *displayPixelBufferScope;

// Monotonically increasing frame/version tag.
// Increment on every resize to invalidate in-flight frames.
// UI updates must match current generation; mismatched frames are dropped.
@property(atomic, readonly) NSUInteger generation;

// Metal texture created from the pixel buffer (nil until a successful resize).
// Note: In software mode, metalTexture is not created and is unavailable.
@property(atomic, readonly, nullable) id<MTLTexture> metalTexture;

- (instancetype)initWithView:(nonnull UIView<AnimaXPixelBufferUpdateListener> *)view;

// Called when native rendering is finished. Produce a UIImage and dispatch it to imageView.
- (void)notifyBufferUpdateWithGeneration:(NSUInteger)currentGeneration
                               srcPixels:(nullable uint8_t *)srcPixels
                                   width:(size_t)width
                                  height:(size_t)height
                                  stride:(size_t)stride;

- (void)notifyBufferUpdateWithGeneration:(NSUInteger)currentGeneration;

// Called after rebuild pixel buffer pool, only in GPU thread.
// Returns a pixel buffer whose size is same with the rendering pixelbuffer;
- (CVPixelBufferRef)acquirePixelBufferFromPool;

// Called after the render pixel buffer is changed, only in GPU thread.
- (void)resizePixelBufferWrapperWithWidth:(size_t)width height:(size_t)height;

+ (CVPixelBufferRef)createPixelBufferWithWidth:(size_t)width
                                        height:(size_t)height
                                       backend:(AnimaXRenderBackend)backend;

@end

NS_ASSUME_NONNULL_END
