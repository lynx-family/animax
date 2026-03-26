// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderRequest.h>
#import <AnimaX/AnimaXLoaderResponse.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, AnimaXLoaderScheme) {
  AnimaXLoaderSchemeAsset = 0,
  AnimaXLoaderSchemeFile = 1,
  AnimaXLoaderSchemeHttp = 2,
  AnimaXLoaderSchemeDataURL = 3,
};

typedef void (^AnimaXLoaderCompletionHandler)(AnimaXLoaderResponse* response);

@protocol AnimaXLoaderProtocol <NSObject>
@required
- (void)handleRequest:(AnimaXLoaderRequest*)request
           completion:(AnimaXLoaderCompletionHandler)completion;
- (AnimaXLoaderScheme)getScheme;
@end

NS_ASSUME_NONNULL_END
