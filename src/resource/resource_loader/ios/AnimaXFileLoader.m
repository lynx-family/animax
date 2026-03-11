// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "AnimaXFileLoader.h"

@implementation AnimaXFileLoader {
  NSBundle* _assetBundle;
}
- (void)handleRequest:(AnimaXLoaderRequest*)request
           completion:(AnimaXLoaderCompletionHandler)completion {
  AnimaXLoaderResponse* response = nil;
  do {
    if (nil == request || nil == request.url || [request.url length] == 0) {
      response = [AnimaXLoaderResponse responseWithErrorMessage:@"Invalid AnimaXLoaderRequest"];
      break;
    }

    NSString* filePath = [self findExistingPathFromUrl:request.url];
    if (filePath == nil) {
      response = [AnimaXLoaderResponse responseWithErrorMessage:@"File does not exist."];
      break;
    }

    NSError* error = nil;
    NSData* fileContent = [NSData dataWithContentsOfFile:filePath options:0 error:&error];

    if (nil != error) {
      response = [AnimaXLoaderResponse responseWithError:error];
      break;
    }

    if (nil == fileContent || [fileContent length] == 0) {
      response = [AnimaXLoaderResponse responseWithErrorMessage:@"Cannot read file."];
      break;
    }

    response = [AnimaXLoaderResponse responseWithPayload:fileContent];
  } while (0);

  completion(response);
}

- (AnimaXLoaderScheme)getScheme {
  return AnimaXLoaderSchemeFile;
}

- (void)setAssetBundle:(NSBundle*)bundle {
  _assetBundle = bundle;
}

- (NSString*)findExistingPathFromUrl:(NSString*)requestUrl {
  NSString* filePath = requestUrl;
  NSURL* url = [NSURL URLWithString:requestUrl];
  if ([url isFileURL]) {
    filePath = [url path];
  }

  NSFileManager* manager = [NSFileManager defaultManager];
  if ([manager fileExistsAtPath:filePath]) {
    return filePath;
  }

  NSString* fileName = [filePath lastPathComponent];

  if (_assetBundle && fileName.length > 0) {
    return [_assetBundle pathForResource:fileName ofType:nil];
  }

  NSRange bundleRange = [filePath rangeOfString:@".bundle" options:NSBackwardsSearch];
  if (bundleRange.location == NSNotFound) {
    return nil;
  }
  NSUInteger bundleEndIndex = bundleRange.location + bundleRange.length;
  NSString* bundlePath = [filePath substringToIndex:bundleEndIndex];
  NSString* newFilePath = [bundlePath stringByAppendingPathComponent:fileName];
  if ([manager fileExistsAtPath:newFilePath]) {
    return newFilePath;
  }
  return nil;
}

@end
