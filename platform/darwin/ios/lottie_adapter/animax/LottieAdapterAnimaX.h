// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LottieAdapterOption.h"
#import "LottieAdapterProtocol.h"

NS_ASSUME_NONNULL_BEGIN

@interface LottieAdapterAnimaX : NSObject <LottieAdapterProtocol>

- (instancetype)initWithOptions:(nullable LottieAdapterOption*)options NS_DESIGNATED_INITIALIZER;

@end

NS_ASSUME_NONNULL_END
