// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LottiePropertyCallbackAdapter.h"

@implementation LottiePropertyCallbackAdapter

+ (instancetype)sharedInstance {
  static LottiePropertyCallbackAdapter *instance = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    instance = [[LottiePropertyCallbackAdapter alloc] init];
  });
  return instance;
}

- (void)onSuccess {
  // Empty implementation for Lottie adapter
}

- (void)onError:(NSArray<NSString *> *)errorMessages {
  // Empty implementation for Lottie adapter
}

@end
