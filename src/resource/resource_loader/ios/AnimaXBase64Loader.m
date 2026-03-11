// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "AnimaXBase64Loader.h"

@implementation AnimaXBase64Loader
- (void)handleRequest:(AnimaXLoaderRequest*)request
           completion:(AnimaXLoaderCompletionHandler)completion {
  AnimaXLoaderResponse* response = nil;
  do {
    if (nil == request || nil == request.url || [request.url length] == 0) {
      response = [AnimaXLoaderResponse responseWithErrorMessage:@"Invalid AnimaXLoaderRequest"];
      break;
    }
    NSURL* url = [NSURL URLWithString:request.url];
    if (nil == url) {
      response = [AnimaXLoaderResponse responseWithErrorMessage:@"Invalid URL"];
      break;
    }
    NSError* error = nil;
    NSData* data = [NSData dataWithContentsOfURL:url options:0 error:&error];
    if (nil != error) {
      response = [AnimaXLoaderResponse responseWithError:error];
      break;
    }

    if (nil == data || [data length] == 0) {
      response =
          [AnimaXLoaderResponse responseWithErrorMessage:@"Cannot convert base64 URL to NSData."];
      break;
    }

    response = [AnimaXLoaderResponse responseWithPayload:data];
  } while (0);

  completion(response);
}

- (AnimaXLoaderScheme)getScheme {
  return AnimaXLoaderSchemeDataURL;
}
@end
