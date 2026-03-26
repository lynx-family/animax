// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LottieKeyPathListCallbackAdapter.h"
#import <AnimaX/AnimaXKeyPath.h>

@interface LottieKeyPathListCallbackAdapter ()

@property(nonatomic, copy) void (^callbackBlock)(NSArray<AnimaXKeyPath *> *keyPaths);

@end

@implementation LottieKeyPathListCallbackAdapter

- (instancetype)initWithBlock:(void (^)(NSArray<AnimaXKeyPath *> *keyPaths))block {
  self = [super init];
  if (self) {
    _callbackBlock = [block copy];
  }
  return self;
}

- (void)onCallback:(NSArray<AnimaXKeyPath *> *)keyPaths {
  if (self.callbackBlock) {
    self.callbackBlock(keyPaths);
  }
}

@end
