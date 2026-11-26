// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface AnimaXLoaderRequest : NSObject

+ (AnimaXLoaderRequest*)requestWithURL:(NSString*)url;
+ (AnimaXLoaderRequest*)requestWithURL:(NSString*)url
                     requestProperties:(nullable NSDictionary<NSString*, id>*)properties;

@property(nonatomic, copy, readonly) NSString* url;
@property(nonatomic, nullable) NSDictionary<NSString*, id>* requestProperties;
/// Optional diagnostics owned by the SDK, with a separate dictionary for each request.
/// The host may set cache_source before calling completion: memory, disk,
/// offline, http_cache, http_revalidated, cdn_cache (unspecified layer), network, in_flight,
/// builtin or unknown. Set cancelled to YES for cancellation. This information must not be
/// forwarded as network parameters or included in cache keys.
@property(nonatomic, strong, nullable) NSMutableDictionary<NSString*, id>* resourceLoadInfo;

@end

NS_ASSUME_NONNULL_END
