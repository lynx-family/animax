// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderRequest.h>

@interface AnimaXLoaderRequest ()
@property(nonatomic, copy, readwrite) NSString *url;
@end

@implementation AnimaXLoaderRequest

- (instancetype)initWithURL:(nonnull NSString *)url
          requestProperties:(NSDictionary<NSString *, id> *)properties {
  self = [super init];
  if (self) {
    _url = url;
    _requestProperties = properties;
  }
  return self;
}

+ (nonnull AnimaXLoaderRequest *)requestWithURL:(nonnull NSString *)url {
  return [[AnimaXLoaderRequest alloc] initWithURL:url requestProperties:nil];
}

+ (nonnull AnimaXLoaderRequest *)requestWithURL:(nonnull NSString *)url
                              requestProperties:(NSDictionary<NSString *, id> *)properties {
  return [[AnimaXLoaderRequest alloc] initWithURL:url requestProperties:properties];
}

@end
