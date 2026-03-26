// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "LottieAdapterLottieCreator.h"
#import "LottieAdapterLottie.h"

@implementation LottieAdapterLottieCreator

- (nonnull id<LottieAdapterProtocol>)createAdapterWithRequest:
    (nonnull LottieAdapterRequest *)request {
  if (request.sourceType == LottieAdapterAnimationFilePath) {
    return [[LottieAdapterLottie alloc] initWithFilePath:request.filePath];
  }
  LottieAdapterLottie *view = [[LottieAdapterLottie alloc] init];
  switch (request.sourceType) {
    case LottieAdapterAnimationNamed:
      [view setAnimationNamed:request.name inBundle:request.bundle];
      break;
    case LottieAdapterAnimationJSON:
      [view setAnimationFromJSON:request.json inBundle:request.bundle];
      break;
    case LottieAdapterAnimationURL:
    // LottieAdapterLottie *view = [[LottieAdapterLottie alloc] init];
    //  if (self) {
    // TODO(aiyongbiao.rick) completed in later version
    // Since Lottie doesn't have direct URL loading, we'll need to implement
    // the JSON downloading and loading logic here
    //  }
    default:
      break;
  }
  return view;
}

@end
