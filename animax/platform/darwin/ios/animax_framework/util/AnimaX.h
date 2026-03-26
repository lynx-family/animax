// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
/// Shared instance of AnimaX
@interface AnimaX : NSObject
+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype _Nonnull)shareInstance;

/// Register logger for animax
- (void)registerLoggerOnce;

@end
NS_ASSUME_NONNULL_END
