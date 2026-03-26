// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXServiceCenter.h>
#import <AnimaX/AnimaXServiceRegistry.h>

@interface AnimaXServiceRegistry ()
@property(nonatomic, strong) NSMutableDictionary<NSString *, id<AnimaXService>> *services;
@end

@implementation AnimaXServiceRegistry

- (instancetype)initWithScope:(AnimaXServiceScope *)scope {
  self = [super init];
  if (self) {
    _scope = scope;
    _services = [NSMutableDictionary dictionary];
  }
  return self;
}

- (nullable id<AnimaXService>)getService:(Protocol *)proto {
  if (!proto) {
    return nil;
  }

  NSString *key = NSStringFromProtocol(proto);
  // 1) Attempt retrieval from instance-level registry
  id<AnimaXService> service = self.services[key];
  // 2) If not found, fallback to global service center with the same scope
  if (!service) {
    service = [[AnimaXServiceCenter sharedInstance] getServiceWithScope:_scope protocol:proto];
  }
  return service;
}

- (void)registerService:(Protocol *)proto withImpl:(id<AnimaXService>)impl {
  if (!proto || !impl) {
    return;
  }
  NSString *key = NSStringFromProtocol(proto);
  self.services[key] = impl;
}

- (void)unregisterService:(Protocol *)proto {
  if (!proto) {
    return;
  }
  NSString *key = NSStringFromProtocol(proto);
  [self.services removeObjectForKey:key];
}

- (void)releaseAll {
  [self.services removeAllObjects];
}

@end
