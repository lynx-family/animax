// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXKeyPathListCallback.h>
#import <Foundation/Foundation.h>

@class AnimaXKeyPath;

NS_ASSUME_NONNULL_BEGIN

/**
 * Adapter class that implements AnimaXKeyPathListCallback protocol
 * to bridge between block-based callbacks and protocol-based callbacks.
 */
@interface LottieKeyPathListCallbackAdapter : NSObject <AnimaXKeyPathListCallback>

/**
 * Initializes the adapter with a callback block
 * @param block The block to be executed when callback is received
 */
- (instancetype)initWithBlock:(void (^)(NSArray<AnimaXKeyPath *> *keyPaths))block;

@end

NS_ASSUME_NONNULL_END
