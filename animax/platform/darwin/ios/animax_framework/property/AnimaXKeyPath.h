// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A path that can target specific layers or elements in an AnimaX animation.
 *
 * KeyPath supports wildcards ('*') to match exactly one item and globstars ('**')
 * to match zero or more items, enabling flexible targeting of animation elements.
 * Nil or empty keys provided in the initializer will be ignored.
 *
 * Example hierarchy:
 * MainScene (Layer)
 *     Background (Layer)
 *         Sky (Layer)
 *             Fill (Property)
 *         Mountains (Layer)
 *             Fill (Property)
 *     Character (Layer)
 *         Body (Layer)
 *             Fill (Property)
 *         Face (Layer)
 *             Eyes (Layer)
 *                 Fill (Property)
 *
 * Example usage:
 * - Target Character's Body Fill:
 *   [[AnimaXKeyPath alloc] initWithKeys:@[@"Character", @"Body", @"Fill"]]
 * - Target all Fills in Character:
 *   [[AnimaXKeyPath alloc] initWithKeys:@[@"Character", @"**", @"Fill"]]
 * - Target all Fills in the animation:
 *   [[AnimaXKeyPath alloc] initWithKeys:@[@"**", @"Fill"]]
 */
@interface AnimaXKeyPath : NSObject

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

/**
 * Creates a KeyPath with the specified key segments.
 * @param keys The key segments that define the path
 */
- (instancetype)initWithKeys:(NSArray<NSString *> *)keys NS_DESIGNATED_INITIALIZER;

/**
 * Gets the keys array
 * @return NSArray of key strings
 */
@property(nonatomic, readonly) NSArray<NSString *> *keys;

@end

NS_ASSUME_NONNULL_END
