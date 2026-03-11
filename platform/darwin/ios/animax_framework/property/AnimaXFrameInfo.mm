// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXFrameInfo.h>

@implementation AnimaXFrameInfo

#pragma mark - Initialization

- (instancetype)initWithStartFrame:(float)startFrame
                          endFrame:(float)endFrame
                        startValue:(nullable AnimaXValueParam *)startValue
                          endValue:(nullable AnimaXValueParam *)endValue
                    linearProgress:(float)linearProgress
              interpolatedProgress:(float)interpolatedProgress
                   overallProgress:(float)overallProgress {
  self = [super init];
  if (self) {
    _startFrame = startFrame;
    _endFrame = endFrame;
    _startValue = startValue;
    _endValue = endValue;
    _linearProgress = linearProgress;
    _interpolatedProgress = interpolatedProgress;
    _overallProgress = overallProgress;
  }
  return self;
}

#pragma mark - NSObject

- (NSString *)description {
  return [NSString
      stringWithFormat:@"AnimaXFrameInfo{frames=%.2f->%.2f, startValue=%@, endValue=%@, "
                       @"progress=[linear=%.2f, interpolated=%.2f, overall=%.2f]}",
                       _startFrame, _endFrame, _startValue ? _startValue.description : @"nil",
                       _endValue ? _endValue.description : @"nil", _linearProgress,
                       _interpolatedProgress, _overallProgress];
}

@end
