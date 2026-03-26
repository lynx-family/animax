// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderProtocol.h>
#import <AnimaX/AnimaXService.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Protocol defining a factory service for creating AnimaX resource loaders.
 * This service is responsible for instantiating and managing AnimaX resource loaders
 * that handle different types of animation resources.
 */
@protocol AnimaXResourceFactoryService <AnimaXService>

/**
 * Creates and returns an array of AnimaX loaders.
 *
 * @return An array of objects conforming to AnimaXLoaderProtocol that can handle
 *         different types of animation resources and formats.
 */
- (NSArray<id<AnimaXLoaderProtocol>> *)createAnimaXLoaders;

@end

NS_ASSUME_NONNULL_END
