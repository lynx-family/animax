// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderProtocol.h>
#import <AnimaX/AnimaXLoaderResponse.h>
#import "AnimaXLoaderResponse+Internal.h"

@interface AnimaXLoaderResponse ()
@property(nonatomic, readwrite) id payload;
@property(nonatomic, readwrite) NSError* error;
@end

NSError* AnimaXErrorMakeWithNSString(NSString* message) {
  return [NSError errorWithDomain:NSCocoaErrorDomain
                             code:-1
                         userInfo:@{NSLocalizedDescriptionKey : message}];
}

AnimaXLoaderPayloadType AnimaXLoaderResponsePayloadTypeForPayload(id payload) {
  if ([payload isKindOfClass:[NSString class]]) {
    return AnimaXLoaderPayloadTypeFilePathNSString;
  } else if ([payload isKindOfClass:[NSData class]]) {
    return AnimaXLoaderPayloadTypeNSData;
  } else {
    return AnimaXLoaderPayloadTypeInvalid;
  }
}

@implementation AnimaXLoaderResponse

- (instancetype)initWithPayload:(id)payload {
  self = [super init];
  if (self) {
    _payload = payload;
    _payloadType = AnimaXLoaderResponsePayloadTypeForPayload(_payload);
    if (_payloadType == AnimaXLoaderPayloadTypeInvalid) {
      _payload = nil;
      _error = AnimaXErrorMakeWithNSString(
          @"Invalid payload: payload must be either NSString* or NSData*.");
    }
  }
  return self;
}

- (instancetype)initWithError:(NSError*)error {
  self = [super init];
  if (self) {
    _payloadType = AnimaXLoaderPayloadTypeInvalid;
    _payload = nil;
    if (error != nil) {
      _error = error;
    } else {
      _error = AnimaXErrorMakeWithNSString(@"Invalid error: error must not be nil.");
    }
  }
  return self;
}

+ (AnimaXLoaderResponse*)responseWithPayload:(nonnull id)payload {
  return [[AnimaXLoaderResponse alloc] initWithPayload:payload];
}

+ (AnimaXLoaderResponse*)responseWithError:(nonnull NSError*)error {
  return [[AnimaXLoaderResponse alloc] initWithError:error];
}

+ (AnimaXLoaderResponse*)responseWithErrorMessage:(NSString*)errorMessage {
  return [[AnimaXLoaderResponse alloc]
      initWithError:[NSError errorWithDomain:@"com.lynx.animax"
                                        code:1
                                    userInfo:@{NSLocalizedDescriptionKey : errorMessage}]];
}

@end
