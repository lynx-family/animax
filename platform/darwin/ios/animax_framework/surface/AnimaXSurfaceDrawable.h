// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXRenderTypes.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, AnimaXRenderTarget) { Layer = 0, Buffer = 1, View = 2 };

@interface AnimaXSurfaceDrawable : NSObject

- (instancetype)initWithBuffer:(nonnull CVPixelBufferRef)buffer
                          size:(CGSize)size
                       backend:(AnimaXRenderBackend)backend;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability"
- (instancetype)initWithMetalLayer:(nonnull CAMetalLayer*)layer scale:(CGFloat)scale;
#pragma clang diagnostic pop

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (BOOL)updateSize:(CGSize)size;

// Called when trying to resize a new pixel buffer.
- (CVPixelBufferRef)createPixelBufferWithWidth:(size_t)width height:(size_t)height;

// Called when trying to build a new pixel buffer.
+ (CVPixelBufferRef)createPixelBufferWithWidth:(size_t)width
                                        height:(size_t)height
                                       backend:(AnimaXRenderBackend)backend;

- (void)setBuffer:(nonnull CVPixelBufferRef)buffer;

// The handler is dispatched to the main thread after copying the frame. View
// targets use the CPU copy from the view update without waiting for the GPU;
// Metal Buffer targets notify after GPU completion. The supplied buffer is an
// independent copy, valid for the callback; retain it to use beyond the callback.
- (void)setFrameAvailableHandler:
    (nullable AnimaXPixelBufferFrameAvailableHandler)frameAvailableHandler;

@end

NS_ASSUME_NONNULL_END
