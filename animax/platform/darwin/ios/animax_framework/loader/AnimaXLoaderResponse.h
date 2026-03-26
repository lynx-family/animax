// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderRequest.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface AnimaXLoaderResponse<__covariant PayloadType> : NSObject

+ (AnimaXLoaderResponse*)responseWithError:(NSError*)error;
+ (AnimaXLoaderResponse*)responseWithErrorMessage:(NSString*)errorMessage;
+ (AnimaXLoaderResponse*)responseWithPayload:(PayloadType)payload;

@property(nonatomic, readonly) PayloadType payload;
@property(nonatomic, readonly, nullable) NSError* error;

@end

NS_ASSUME_NONNULL_END
