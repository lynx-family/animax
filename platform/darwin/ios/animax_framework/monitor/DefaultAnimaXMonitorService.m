// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXURLUtils.h>
#import <AnimaX/DefaultAnimaXMonitorService.h>

@interface DefaultAnimaXMonitorService ()
- (NSMutableDictionary *)getBaseCategory;
@end

@implementation DefaultAnimaXMonitorService

- (NSMutableDictionary *)getBaseCategory {
  NSString *sourceUrl = [AnimaXURLUtils clearUrlQuery:[self.urlHolder getCurrentUrl]];
  return [@{
    AnimaXMonitorSourceUrl : sourceUrl,
  } mutableCopy];
}

- (void)reportError:(NSDictionary<NSString *, NSObject *> *)params {
  if (params.count == 0) {
    return;
  }

  NSMutableDictionary *category = [self getBaseCategory];
  [category addEntriesFromDictionary:params];
  [self reportWithName:@"animax_native_error" category:category metric:@{}];
}

- (void)reportPerformance:(NSDictionary<NSString *, NSObject *> *)params {
  if (params.count == 0) {
    return;
  }

  NSMutableDictionary *category = [self getBaseCategory];
  [self reportWithName:@"animax_native_performance" category:category metric:params];
}

- (void)reportWithName:(NSString *)eventName
              category:(NSDictionary *)category
                metric:(NSDictionary *)metric {
  // TODO(aiyongbiao.rick): implement native side report
}

@end
