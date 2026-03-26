// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXValueParam.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Contains information about the current animation frame for value callbacks.
 * Similar to Lottie's LottieFrameInfo, this provides context about where in
 * the animation timeline a callback is being invoked.
 */
@interface AnimaXFrameInfo : NSObject

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

/**
 * Create a new frame info object
 *
 * @param startFrame The composition start frame
 * @param endFrame The composition end frame
 * @param startValue The starting value of the current keyframe
 * @param endValue The ending value of the current keyframe
 * @param linearProgress The linear (non-interpolated) progress in the current keyframe (0-1)
 * @param interpolatedProgress The interpolated progress in the current keyframe (0-1)
 * @param overallProgress The overall progress of the animation (0-1)
 */
- (instancetype)initWithStartFrame:(float)startFrame
                          endFrame:(float)endFrame
                        startValue:(nullable AnimaXValueParam *)startValue
                          endValue:(nullable AnimaXValueParam *)endValue
                    linearProgress:(float)linearProgress
              interpolatedProgress:(float)interpolatedProgress
                   overallProgress:(float)overallProgress NS_DESIGNATED_INITIALIZER;

#pragma mark - Properties

/**
 * The starting frame of the composition
 */
@property(nonatomic, readonly) float startFrame;

/**
 * The ending frame of the composition
 */
@property(nonatomic, readonly) float endFrame;

/**
 * The starting value of the current keyframe
 */
@property(nonatomic, readonly, nullable) AnimaXValueParam *startValue;

/**
 * The ending value of the current keyframe
 */
@property(nonatomic, readonly, nullable) AnimaXValueParam *endValue;

/**
 * The linear (non-interpolated) progress in the current keyframe (0-1)
 */
@property(nonatomic, readonly) float linearProgress;

/**
 * The interpolated progress in the current keyframe (0-1)
 */
@property(nonatomic, readonly) float interpolatedProgress;

/**
 * The overall progress of the animation (0-1)
 */
@property(nonatomic, readonly) float overallProgress;

@end

NS_ASSUME_NONNULL_END
