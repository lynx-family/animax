// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "LottieAdapterAnimaXCreator.h"
#import "LottieAdapterAnimaX.h"

@implementation LottieAdapterAnimaXCreator

- (id<LottieAdapterProtocol>)createAdapter {
  return [[LottieAdapterAnimaX alloc] init];
}

- (id<LottieAdapterProtocol>)createAdapterWithAnimationNamed:(NSString *)name {
  LottieAdapterAnimaX *view = [[LottieAdapterAnimaX alloc] init];
  [view setAnimationNamed:name];
  return view;
}

- (id<LottieAdapterProtocol>)createAdapterWithAnimationNamed:(NSString *)name
                                                    inBundle:(NSBundle *)bundle {
  LottieAdapterAnimaX *view = [[LottieAdapterAnimaX alloc] init];
  [view setAnimationNamed:name inBundle:bundle];
  return view;
}

- (id<LottieAdapterProtocol>)createAdapterWithJSON:(NSDictionary *)json {
  LottieAdapterAnimaX *view = [[LottieAdapterAnimaX alloc] init];
  [view setAnimationFromJSON:json];
  return view;
}

- (id<LottieAdapterProtocol>)createAdapterWithJSON:(NSDictionary *)json
                                          inBundle:(NSBundle *)bundle {
  LottieAdapterAnimaX *view = [[LottieAdapterAnimaX alloc] init];
  [view setAnimationFromJSON:json inBundle:bundle];
  return view;
}

- (id<LottieAdapterProtocol>)createAdapterWithFilePath:(NSString *)filePath {
  LottieAdapterAnimaX *view = [[LottieAdapterAnimaX alloc] init];
  // TODO(aiyongbiao.rick) completed in later version
  // NSURL *url = [NSURL fileURLWithPath:filePath];
  // [view.animaXView setSrc:url.absoluteString];
  return view;
}

- (id<LottieAdapterProtocol>)createAdapterWithURL:(NSURL *)url {
  LottieAdapterAnimaX *view = [[LottieAdapterAnimaX alloc] init];
  // TODO(aiyongbiao.rick) completed in later version
  // [view.animaXView setSrc:url.absoluteString];
  return view;
}

- (nonnull id<LottieAdapterProtocol>)createAdapterWithRequest:
    (nonnull LottieAdapterRequest *)request {
  LottieAdapterAnimaX *view = [[LottieAdapterAnimaX alloc] initWithOptions:request.options];
  switch (request.sourceType) {
    case LottieAdapterAnimationNamed:
      [view setAnimationNamed:request.name inBundle:request.bundle];
      break;
    case LottieAdapterAnimationJSON:
      [view setAnimationFromJSON:request.json inBundle:request.bundle];
      break;
    case LottieAdapterAnimationFilePath:
    // TODO(aiyongbiao.rick) completed in later version
    // NSURL *url = [NSURL fileURLWithPath:filePath];
    // [view.animaXView setSrc:url.absoluteString];
    case LottieAdapterAnimationURL:
    // TODO(aiyongbiao.rick) completed in later version
    // [view.animaXView setSrc:url.absoluteString];
    default:
      break;
  }
  return view;
}

@end
