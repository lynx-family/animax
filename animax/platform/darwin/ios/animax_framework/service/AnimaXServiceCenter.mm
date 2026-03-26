// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXServiceCenter.h>
#include "src/base/log/log.h"

@interface AnimaXServiceCenter ()
@property(nonatomic, strong)
    NSMutableDictionary<AnimaXServiceScope *, NSMutableDictionary<NSString *, id<AnimaXService>> *>
        *scopedServices;
@end

@implementation AnimaXServiceCenter

+ (instancetype)sharedInstance {
  static AnimaXServiceCenter *sInstance = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    sInstance = [[self alloc] initPrivate];
  });
  return sInstance;
}

- (instancetype)init {
  // Use +[AnimaXServiceCenter sharedInstance] instead.
  DCHECK(false);
  return nil;
}

- (instancetype)initPrivate {
  self = [super init];
  if (self) {
    _scopedServices = [NSMutableDictionary dictionary];
    // Initialize default scope
    _scopedServices[[AnimaXServiceScope defaultScope]] = [NSMutableDictionary dictionary];
  }
  return self;
}

#pragma mark - Public Methods

- (void)registerService:(Protocol *)proto withImpl:(id<AnimaXService>)impl {
  [self registerServiceWithScope:[AnimaXServiceScope defaultScope] protocol:proto withImpl:impl];
}

- (nullable id<AnimaXService>)getService:(Protocol *)proto {
  return [self getServiceWithScope:[AnimaXServiceScope defaultScope] protocol:proto];
}

- (void)unregisterService:(Protocol *)proto {
  [self unregisterServiceWithScope:[AnimaXServiceScope defaultScope] protocol:proto];
}

- (void)unregisterAllServices {
  [self unregisterAllServicesWithScope:[AnimaXServiceScope defaultScope]];
}

- (void)registerServiceWithScope:(AnimaXServiceScope *)scope
                        protocol:(Protocol *)proto
                        withImpl:(id<AnimaXService>)impl {
  if (!proto || !impl || !scope) {
    return;
  }

  NSMutableDictionary<NSString *, id<AnimaXService>> *servicesForScope = _scopedServices[scope];
  if (!servicesForScope) {
    servicesForScope = [NSMutableDictionary dictionary];
    _scopedServices[scope] = servicesForScope;
  }

  NSString *key = NSStringFromProtocol(proto);
  if (servicesForScope[key]) {
    ANIMAX_LOGE("Failed to register: " << key.UTF8String << " is already registered in scope "
                                       << scope.description.UTF8String);
    return;
  }
  servicesForScope[key] = impl;
}

- (nullable id<AnimaXService>)getServiceWithScope:(AnimaXServiceScope *)scope
                                         protocol:(Protocol *)proto {
  if (!proto || !scope) {
    return nil;
  }
  NSMutableDictionary<NSString *, id<AnimaXService>> *servicesForScope = _scopedServices[scope];
  if (!servicesForScope) {
    return nil;
  }
  NSString *key = NSStringFromProtocol(proto);
  return servicesForScope[key];
}

- (void)unregisterServiceWithScope:(AnimaXServiceScope *)scope protocol:(Protocol *)proto {
  if (!scope || !proto) {
    return;
  }
  NSMutableDictionary<NSString *, id<AnimaXService>> *servicesForScope = _scopedServices[scope];
  if (servicesForScope) {
    NSString *key = NSStringFromProtocol(proto);
    [servicesForScope removeObjectForKey:key];
  }
}

- (void)unregisterAllServicesWithScope:(AnimaXServiceScope *)scope {
  if (!scope) {
    return;
  }
  NSMutableDictionary<NSString *, id<AnimaXService>> *servicesForScope = _scopedServices[scope];
  if (servicesForScope) {
    [servicesForScope removeAllObjects];
  }
}

@end
