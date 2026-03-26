// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXSurfaceDrawable.h>
#import "CVPixelBufferWrapper.h"

NS_ASSUME_NONNULL_BEGIN
@interface AnimaXSurfaceDrawable (Internal)

@property(nonatomic, readonly) AnimaXRenderTarget target;
@property(nonatomic, readonly) AnimaXRenderBackend backend;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability"
@property(nonatomic, weak, readonly) CAMetalLayer* layer;
#pragma clang diagnostic pop
@property(nonatomic, strong, readonly) CVPixelBufferWrapper* bufferWrapper;

// Every change to size should call update size, which returns a BOOL indicating
@property(nonatomic, readonly) CGSize size;

- (instancetype)initWithView:(nonnull UIView<AnimaXPixelBufferUpdateListener>*)view
                        size:(CGSize)size
                     backend:(AnimaXRenderBackend)backend;

@end

NS_ASSUME_NONNULL_END
