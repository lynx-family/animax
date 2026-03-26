// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "LottieAdapterLottie.h"
#import <Foundation/Foundation.h>
#if __has_include(<Lottie/LOTAnimationView.h>)
#import <Lottie/LOTAnimationView.h>
#import <Lottie/LOTKeypath.h>
#import <Lottie/LOTValueCallback.h>
#import <Lottie/Lottie.h>
#elif __has_include(<lottie-ios/Lottie/LOTAnimationView.h>)
#import <lottie-ios/Lottie/LOTAnimationView.h>
#import <lottie-ios/Lottie/LOTKeypath.h>
#import <lottie-ios/Lottie/LOTValueCallback.h>
#import <lottie-ios/Lottie/Lottie.h>
#endif
#import <UIKit/UIKit.h>
#import "LottieAdapterProtocol.h"
#import "LottieKeyPath.h"
#import "LottieValueDelegate.h"

@interface LottieAdapterLottie ()
@property(nonatomic, strong) LOTAnimationView *animationView;
@end

@implementation LottieAdapterLottie

@synthesize completionBlock;

#pragma mark - Initialization

- (instancetype)init {
  self = [super init];
  if (self) {
    _animationView = [[LOTAnimationView alloc] init];
  }
  return self;
}

- (nonnull instancetype)initWithFilePath:(nonnull NSString *)filePath {
  self = [super init];
  if (self) {
    _animationView = [LOTAnimationView animationWithFilePath:filePath];
  }
  return self;
}

#pragma mark - LottieAdapterProtocol

- (void)setAnimationNamed:(NSString *)animationName {
  [_animationView setAnimationNamed:animationName];
}

- (void)setAnimationNamed:(NSString *)animationName inBundle:(NSBundle *)bundle {
  [_animationView setAnimationNamed:animationName inBundle:bundle];
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON {
  [_animationView setAnimationFromJSON:animationJSON];
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON inBundle:(NSBundle *)bundle {
  [_animationView setAnimationFromJSON:animationJSON inBundle:bundle];
}

- (BOOL)isAnimationPlaying {
  return _animationView.isAnimationPlaying;
}

- (BOOL)loopAnimation {
  return _animationView.loopAnimation;
}

- (void)setLoopAnimation:(BOOL)loopAnimation {
  _animationView.loopAnimation = loopAnimation;
}

- (BOOL)autoReverseAnimation {
  return _animationView.autoReverseAnimation;
}

- (void)setAutoReverseAnimation:(BOOL)autoReverseAnimation {
  _animationView.autoReverseAnimation = autoReverseAnimation;
}

- (CGFloat)animationProgress {
  return _animationView.animationProgress;
}

- (void)setAnimationProgress:(CGFloat)animationProgress {
  _animationView.animationProgress = animationProgress;
}

- (CGFloat)animationSpeed {
  return _animationView.animationSpeed;
}

- (void)setAnimationSpeed:(CGFloat)animationSpeed {
  _animationView.animationSpeed = animationSpeed;
}

- (CGFloat)animationDuration {
  return _animationView.animationDuration;
}

- (BOOL)cacheEnable {
  return _animationView.cacheEnable;
}

- (void)setCacheEnable:(BOOL)cacheEnable {
  _animationView.cacheEnable = cacheEnable;
}

- (BOOL)shouldRasterizeWhenIdle {
  return _animationView.shouldRasterizeWhenIdle;
}

- (void)setShouldRasterizeWhenIdle:(BOOL)shouldRasterizeWhenIdle {
  _animationView.shouldRasterizeWhenIdle = shouldRasterizeWhenIdle;
}

- (NSUInteger)repeatCount {
  return _animationView.repeatCount;
}

- (void)setRepeatCount:(NSUInteger)repeatCount {
  _animationView.repeatCount = repeatCount;
}

- (void)play {
  [_animationView play];
}

- (void)playWithCompletion:(LottieAdapterCompletionBlock)completion {
  [_animationView playWithCompletion:completion];
}

- (void)playToProgress:(CGFloat)toProgress withCompletion:(LottieAdapterCompletionBlock)completion {
  [_animationView playToProgress:toProgress withCompletion:completion];
}

- (void)playFromProgress:(CGFloat)fromStartProgress
              toProgress:(CGFloat)toEndProgress
          withCompletion:(LottieAdapterCompletionBlock)completion {
  [_animationView playFromProgress:fromStartProgress
                        toProgress:toEndProgress
                    withCompletion:completion];
}

- (void)playToFrame:(NSNumber *)toFrame withCompletion:(LottieAdapterCompletionBlock)completion {
  [_animationView playToFrame:toFrame withCompletion:completion];
}

- (void)playFromFrame:(NSNumber *)fromStartFrame
              toFrame:(NSNumber *)toEndFrame
       withCompletion:(LottieAdapterCompletionBlock)completion {
  [_animationView playFromFrame:fromStartFrame toFrame:toEndFrame withCompletion:completion];
}

- (void)stop {
  [_animationView stop];
}

- (void)pause {
  [_animationView pause];
}

- (void)setProgressWithFrame:(NSNumber *)currentFrame {
  [_animationView setProgressWithFrame:currentFrame];
}

- (void)forceDrawingUpdate {
  [_animationView forceDrawingUpdate];
}

#pragma mark - Frame Progress

- (void)setProgressWithFrame:(NSNumber *)currentFrame
    callCompletionIfNecessary:(BOOL)callCompletion {
  [_animationView setProgressWithFrame:currentFrame];
  if (callCompletion && self.completionBlock) {
    self.completionBlock(YES);
  }
}

- (UIView *)contentView {
  return _animationView;
}

- (UIViewContentMode)contentMode {
  return [_animationView contentMode];
}

- (void)setContentMode:(UIViewContentMode)contentMode {
  _animationView.contentMode = contentMode;
}

- (void)setMaxFrameRate:(double)maxFrameRate {
  // do not supported
}

- (void)setValueDelegate:(id<LottieValueDelegate>)delegate forKeypath:(LottieKeyPath *)keypath {
  if (!delegate || !keypath) {
    return;
  }

  // Convert LottieKeyPath (adapter) to LOTKeypath (Lottie OC)
  LOTKeypath *lotKeypath = [LOTKeypath keypathWithString:keypath.keypath];

  // Map LottieColorValueDelegate to LOTColorValueDelegate
  if ([delegate conformsToProtocol:@protocol(LottieColorValueDelegate)]) {
    id<LottieColorValueDelegate> colorDelegate = (id<LottieColorValueDelegate>)delegate;
    LOTColorBlockCallback *callback = [LOTColorBlockCallback
        withBlock:^CGColorRef(CGFloat currentFrame, CGFloat startKeyFrame, CGFloat endKeyFrame,
                              CGFloat interpolatedProgress, CGColorRef startColor,
                              CGColorRef endColor, CGColorRef interpolatedColor) {
          // Forward call to LottieAdapter delegate
          return [colorDelegate colorForFrame:currentFrame
                                startKeyframe:startKeyFrame
                                  endKeyframe:endKeyFrame
                         interpolatedProgress:interpolatedProgress
                                   startColor:startColor
                                     endColor:endColor
                                 currentColor:interpolatedColor];
        }];
    [self.animationView setValueDelegate:callback forKeypath:lotKeypath];
    return;
  }

  // Map LottieNumberValueDelegate to LOTNumberValueDelegate
  if ([delegate conformsToProtocol:@protocol(LottieNumberValueDelegate)]) {
    id<LottieNumberValueDelegate> numberDelegate = (id<LottieNumberValueDelegate>)delegate;
    LOTNumberBlockCallback *callback = [LOTNumberBlockCallback
        withBlock:^CGFloat(CGFloat currentFrame, CGFloat startKeyFrame, CGFloat endKeyFrame,
                           CGFloat interpolatedProgress, CGFloat startValue, CGFloat endValue,
                           CGFloat interpolatedValue) {
          // Forward call to LottieAdapter delegate
          return [numberDelegate floatValueForFrame:currentFrame
                                      startKeyframe:startKeyFrame
                                        endKeyframe:endKeyFrame
                               interpolatedProgress:interpolatedProgress
                                         startValue:startValue
                                           endValue:endValue
                                       currentValue:interpolatedValue];
        }];
    [self.animationView setValueDelegate:callback forKeypath:lotKeypath];
    return;
  }

  // Map LottiePointValueDelegate to LOTPointValueDelegate
  if ([delegate conformsToProtocol:@protocol(LottiePointValueDelegate)]) {
    id<LottiePointValueDelegate> pointDelegate = (id<LottiePointValueDelegate>)delegate;
    LOTPointBlockCallback *callback = [LOTPointBlockCallback
        withBlock:^CGPoint(CGFloat currentFrame, CGFloat startKeyFrame, CGFloat endKeyFrame,
                           CGFloat interpolatedProgress, CGPoint startPoint, CGPoint endPoint,
                           CGPoint interpolatedPoint) {
          // Forward call to LottieAdapter delegate
          return [pointDelegate pointForFrame:currentFrame
                                startKeyframe:startKeyFrame
                                  endKeyframe:endKeyFrame
                         interpolatedProgress:interpolatedProgress
                                   startPoint:startPoint
                                     endPoint:endPoint
                                 currentPoint:interpolatedPoint];
        }];
    [self.animationView setValueDelegate:callback forKeypath:lotKeypath];
    return;
  }

  // Map LottieSizeValueDelegate to LOTSizeValueDelegate
  if ([delegate conformsToProtocol:@protocol(LottieSizeValueDelegate)]) {
    id<LottieSizeValueDelegate> sizeDelegate = (id<LottieSizeValueDelegate>)delegate;
    LOTSizeBlockCallback *callback = [LOTSizeBlockCallback
        withBlock:^CGSize(CGFloat currentFrame, CGFloat startKeyFrame, CGFloat endKeyFrame,
                          CGFloat interpolatedProgress, CGSize startSize, CGSize endSize,
                          CGSize interpolatedSize) {
          // Forward call to LottieAdapter delegate
          return [sizeDelegate sizeForFrame:currentFrame
                              startKeyframe:startKeyFrame
                                endKeyframe:endKeyFrame
                       interpolatedProgress:interpolatedProgress
                                  startSize:startSize
                                    endSize:endSize
                                currentSize:interpolatedSize];
        }];
    [self.animationView setValueDelegate:callback forKeypath:lotKeypath];
    return;
  }

  // Map LottiePathValueDelegate to LOTPathValueDelegate
  if ([delegate conformsToProtocol:@protocol(LottiePathValueDelegate)]) {
    id<LottiePathValueDelegate> pathDelegate = (id<LottiePathValueDelegate>)delegate;
    LOTPathBlockCallback *callback = [LOTPathBlockCallback
        withBlock:^CGPathRef(CGFloat currentFrame, CGFloat startKeyFrame, CGFloat endKeyFrame,
                             CGFloat interpolatedProgress) {
          // Forward call to LottieAdapter delegate
          return [pathDelegate pathForFrame:currentFrame
                              startKeyframe:startKeyFrame
                                endKeyframe:endKeyFrame
                       interpolatedProgress:interpolatedProgress];
        }];
    [self.animationView setValueDelegate:callback forKeypath:lotKeypath];
    return;
  }
}

- (NSArray *)keysForKeyPath:(LottieKeyPath *)keypath {
  if (!keypath) {
    return nil;
  }
  LOTKeypath *lotKeypath = [LOTKeypath keypathWithString:keypath.keypath];
  NSArray<NSString *> *searchResults = [self.animationView keysForKeyPath:lotKeypath];
  return [searchResults copy];
}
@end
