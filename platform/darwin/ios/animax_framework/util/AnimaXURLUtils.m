// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXURLUtils.h>

NSString *const AnimaXMonitorUnknownUrl = @"Unknown";

@implementation AnimaXURLUtils

+ (NSString *)clearUrlQuery:(nullable NSString *)originUrl {
  if (originUrl.length == 0) {
    return AnimaXMonitorUnknownUrl;
  }

  @try {
    NSURLComponents *components = [[NSURLComponents alloc] initWithString:originUrl];
    if (!components) {
      return AnimaXMonitorUnknownUrl;
    }

    // Clear query parameters
    components.query = nil;

    NSString *result = components.URL.absoluteString;
    return result ?: AnimaXMonitorUnknownUrl;
  } @catch (NSException *exception) {
    return AnimaXMonitorUnknownUrl;
  }
}

@end
