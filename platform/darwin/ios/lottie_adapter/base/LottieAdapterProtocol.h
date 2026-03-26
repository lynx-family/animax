// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>
#import "LottieKeyPath.h"
#import "LottieValueDelegate.h"

NS_ASSUME_NONNULL_BEGIN

typedef void (^LottieAdapterCompletionBlock)(BOOL animationFinished);

@protocol LottieAdapterProtocol <NSObject>

// Animation loading methods
- (void)setAnimationNamed:(nonnull NSString *)animationName NS_SWIFT_NAME(setAnimation(named:));
- (void)setAnimationNamed:(nonnull NSString *)animationName
                 inBundle:(nullable NSBundle *)bundle NS_SWIFT_NAME(setAnimation(named:bundle:));
- (void)setAnimationFromJSON:(nonnull NSDictionary *)animationJSON
    NS_SWIFT_NAME(setAnimation(json:));
- (void)setAnimationFromJSON:(nonnull NSDictionary *)animationJSON
                    inBundle:(nullable NSBundle *)bundle NS_SWIFT_NAME(setAnimation(json:bundle:));

// Animation state properties
@property(nonatomic, readonly) BOOL isAnimationPlaying;
@property(nonatomic, assign) BOOL loopAnimation;
@property(nonatomic, assign) BOOL autoReverseAnimation;
@property(nonatomic, assign) CGFloat animationProgress;
@property(nonatomic, assign) CGFloat animationSpeed;
@property(nonatomic, readonly) CGFloat animationDuration;
@property(nonatomic, assign) BOOL cacheEnable;
@property(nonatomic, copy, nullable) LottieAdapterCompletionBlock completionBlock;
@property(nonatomic, assign) BOOL shouldRasterizeWhenIdle;
@property(nonatomic, assign) NSUInteger repeatCount;
@property(nonatomic) UIViewContentMode contentMode;
@property(nonatomic, readonly) UIView *contentView;

// Playback control methods
- (void)playToProgress:(CGFloat)toProgress
        withCompletion:(nullable LottieAdapterCompletionBlock)completion;
- (void)playFromProgress:(CGFloat)fromStartProgress
              toProgress:(CGFloat)toEndProgress
          withCompletion:(nullable LottieAdapterCompletionBlock)completion;
- (void)playToFrame:(nonnull NSNumber *)toFrame
     withCompletion:(nullable LottieAdapterCompletionBlock)completion;
- (void)playFromFrame:(nonnull NSNumber *)fromStartFrame
              toFrame:(nonnull NSNumber *)toEndFrame
       withCompletion:(nullable LottieAdapterCompletionBlock)completion;
- (void)playWithCompletion:(nullable LottieAdapterCompletionBlock)completion;
- (void)play;
- (void)pause;
- (void)stop;

// Frame/Progress control
- (void)setProgressWithFrame:(nonnull NSNumber *)currentFrame;
- (void)setProgressWithFrame:(nonnull NSNumber *)currentFrame
    callCompletionIfNecessary:(BOOL)callCompletion;
- (void)forceDrawingUpdate;
- (void)setMaxFrameRate:(double)maxFrameRate;

// Dynamic proeprty update
- (void)setValueDelegate:(id<LottieValueDelegate> _Nonnull)delegates
              forKeypath:(LottieKeyPath *_Nonnull)keypath;
- (nullable NSArray *)keysForKeyPath:(nonnull LottieKeyPath *)keypath;

@end

NS_ASSUME_NONNULL_END
