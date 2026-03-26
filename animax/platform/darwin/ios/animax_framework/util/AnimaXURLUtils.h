// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXPORT NSString *const AnimaXMonitorUnknownUrl;

/**
 * A utility class providing URL manipulation methods
 */
@interface AnimaXURLUtils : NSObject

/**
 * Remove all query parameters from a URL string
 * @param originUrl The original URL string
 * @return URL string with query parameters removed, returns "unknown" if processing fails
 */
+ (NSString *)clearUrlQuery:(nullable NSString *)originUrl;

@end

NS_ASSUME_NONNULL_END
