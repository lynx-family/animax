// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXFrameInfo.h>
#import <AnimaX/AnimaXValueParam.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Abstract base class for callbacks that can dynamically modify animation property values.
 */
@interface AnimaXValueCallback : NSObject

/**
 * Default initializer
 */
- (instancetype)init;

/**
 * Override this to provide custom values for specific frames.
 * If you haven't set a static value, this will be called for each frame.
 *
 * @param frameInfo Information about the current frame
 * @return The value to use for this frame, or nil to use the original value
 */
- (nullable AnimaXValueParam *)getValue:(AnimaXFrameInfo *)frameInfo;

@end

NS_ASSUME_NONNULL_END
