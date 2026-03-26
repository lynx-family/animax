// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/DefaultAnimaXAbility.h>
#import <AnimaX/DefaultAnimaXMonitorService.h>

@implementation DefaultAnimaXAbility

- (instancetype)init {
  self = [super init];
  if (self) {
    DefaultAnimaXMonitorService *monitor = [[DefaultAnimaXMonitorService alloc] init];
    [self registerService:@protocol(AnimaXMonitorService) withImpl:monitor];
  }
  return self;
}

@end
