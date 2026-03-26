// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXContext.h>
#import "AnimaXResourceLoaderHolder.h"

NS_ASSUME_NONNULL_BEGIN

@interface AnimaXResourceLoaderHolder (Internal)

+ (AnimaXResourceLoaderHolder *)loaderForCompositionFactory:(AnimaXContext *)context;

@end

NS_ASSUME_NONNULL_END
