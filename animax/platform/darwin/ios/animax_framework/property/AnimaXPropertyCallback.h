// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Protocol for property update operation callbacks.
 * Provides separate methods for success and error cases.
 */
@protocol AnimaXPropertyCallback <NSObject>

/**
 * Called when a property update operation completes successfully
 */
- (void)onSuccess;

/**
 * Called when a property update operation fails
 *
 * @param errorMessages A list of error messages indicating the reasons for
 * failure.
 */
- (void)onError:(NSArray<NSString*>*)errorMessages;
@end

NS_ASSUME_NONNULL_END
