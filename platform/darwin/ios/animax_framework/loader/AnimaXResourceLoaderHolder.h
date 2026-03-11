// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderProtocol.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class AnimaXContext;

@interface AnimaXResourceLoaderHolder : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

+ (AnimaXResourceLoaderHolder *)loaderForPlayer:(void *)builderPtr context:(AnimaXContext *)context;
- (void)setAssetBundle:(NSBundle *)bundle;

@end

NS_ASSUME_NONNULL_END
