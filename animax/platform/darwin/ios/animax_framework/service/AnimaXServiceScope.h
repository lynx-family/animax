// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Defines the scope of services in the AnimaX service system.
 * This class allows both predefined and custom service scopes.
 */
@interface AnimaXServiceScope : NSObject <NSCopying>

/**
 * Default scope for public services.
 */
@property(class, readonly, strong) AnimaXServiceScope *defaultScope;

/**
 * Creates a new AnimaXServiceScope with the specified value.
 * The value will be converted to lowercase to ensure consistent comparison.
 *
 * @param value The scope value
 * @return A new AnimaXServiceScope instance
 */
+ (instancetype)scopeWithValue:(NSString *)value;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
