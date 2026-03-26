// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXContext.h>
#import <AnimaX/DefaultAnimaXAbility.h>
#import "AnimaXContext+Internal.h"

@implementation AnimaXContext

+ (instancetype)contextWithDefaultAbility {
  return [[self alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];
}

- (instancetype)initWithAbility:(BaseAnimaXAbility *)ability {
  if (self = [super init]) {
    _ability = ability;
  }
  return self;
}

- (AnimaXMonitorAbilityDelegate *)monitorDelegate {
  return [self.ability getMonitorDelegate];
}

@end
