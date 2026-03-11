// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

@class AnimaXKeyPath;

NS_ASSUME_NONNULL_BEGIN

/**
 * Protocol for operations that return a list of AnimaXKeyPath objects.
 * Returns an empty list on failure.
 */
@protocol AnimaXKeyPathListCallback <NSObject>

/**
 * Called when the operation completes (success or failure)
 *
 * @param keyPaths List of AnimaXKeyPath objects returned by the operation.
 *                 Empty list if the operation failed.
 */
- (void)onCallback:(NSArray<AnimaXKeyPath *> *)keyPaths;

@end

NS_ASSUME_NONNULL_END
