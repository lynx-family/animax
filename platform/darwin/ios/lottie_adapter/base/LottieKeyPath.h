// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface LottieKeyPath : NSObject <NSCopying>

@property(nonatomic, copy, readonly) NSString *keypath;
@property(nonatomic, strong, readonly) NSArray<NSString *> *keys;

+ (instancetype)keypathWithKeypath:(NSString *)keypath;

- (instancetype)initWithKeypath:(NSString *)keypath;

@end

NS_ASSUME_NONNULL_END
