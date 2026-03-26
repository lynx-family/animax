// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LottieKeyPath.h"
#import <Foundation/Foundation.h>

@implementation LottieKeyPath

- (instancetype)initWithKeypath:(NSString *)keypath {
  self = [super init];
  if (self) {
    _keypath = [keypath copy];
    _keys = [keypath componentsSeparatedByString:@"."];
  }
  return self;
}

+ (instancetype)keypathWithKeypath:(NSString *)keypath {
  return [[LottieKeyPath alloc] initWithKeypath:keypath];
}

- (NSString *)description {
  return self.keypath;
}

- (NSString *)stringValue {
  return self.keypath;
}

- (id)copyWithZone:(NSZone *)zone {
  return [[LottieKeyPath alloc] initWithKeypath:self.keypath];
}

- (NSUInteger)hash {
  return [self.keypath hash];
}

- (BOOL)isEqual:(id)object {
  if (self == object) {
    return YES;
  }

  if (![object isKindOfClass:[LottieKeyPath class]]) {
    return NO;
  }

  LottieKeyPath *other = (LottieKeyPath *)object;
  return [self.keypath isEqualToString:other.keypath];
}

@end
