// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "EmptyLottieAdapter.h"

@implementation EmptyLottieAdapter

#pragma mark - LottieAdapterCreator

- (id<LottieAdapterProtocol>)createAdapter {
  return [[EmptyLottieAdapter alloc] init];
}

- (id<LottieAdapterProtocol>)createAdapterWithAnimationNamed:(NSString *)name {
  return [[EmptyLottieAdapter alloc] init];
}

- (id<LottieAdapterProtocol>)createAdapterWithAnimationNamed:(NSString *)name
                                                    inBundle:(NSBundle *)bundle {
  return [[EmptyLottieAdapter alloc] init];
}

- (id<LottieAdapterProtocol>)createAdapterWithJSON:(NSDictionary *)json {
  return [[EmptyLottieAdapter alloc] init];
}

- (id<LottieAdapterProtocol>)createAdapterWithJSON:(NSDictionary *)json
                                          inBundle:(NSBundle *)bundle {
  return [[EmptyLottieAdapter alloc] init];
}

- (id<LottieAdapterProtocol>)createAdapterWithFilePath:(NSString *)filePath {
  return [[EmptyLottieAdapter alloc] init];
}

- (id<LottieAdapterProtocol>)createAdapterWithURL:(NSURL *)url {
  return [[EmptyLottieAdapter alloc] init];
}

#pragma mark - LottieAdapterProtocol

- (void)setAnimationNamed:(NSString *)animationName {
}

- (void)setAnimationNamed:(NSString *)animationName inBundle:(NSBundle *)bundle {
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON {
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON inBundle:(NSBundle *)bundle {
}

- (BOOL)isAnimationPlaying {
  return NO;
}

- (BOOL)loopAnimation {
  return NO;
}

- (void)setLoopAnimation:(BOOL)loopAnimation {
}

- (BOOL)autoReverseAnimation {
  return NO;
}

- (void)setAutoReverseAnimation:(BOOL)autoReverseAnimation {
}

- (CGFloat)animationProgress {
  return 0;
}

- (void)setAnimationProgress:(CGFloat)animationProgress {
}

- (CGFloat)animationSpeed {
  return 0;
}

- (void)setAnimationSpeed:(CGFloat)animationSpeed {
}

- (CGFloat)animationDuration {
  return 0;
}

- (BOOL)cacheEnable {
  return NO;
}

- (void)setCacheEnable:(BOOL)cacheEnable {
}

- (void)setCompletionBlock:(LottieAdapterCompletionBlock)completionBlock {
}

- (LottieAdapterCompletionBlock)completionBlock {
  return nil;
}

- (BOOL)shouldRasterizeWhenIdle {
  return NO;
}

- (void)setShouldRasterizeWhenIdle:(BOOL)shouldRasterizeWhenIdle {
}

- (NSUInteger)repeatCount {
  return 0;
}

- (void)setRepeatCount:(NSUInteger)repeatCount {
}

- (void)play {
}

- (void)playWithCompletion:(LottieAdapterCompletionBlock)completion {
}

- (void)playToProgress:(CGFloat)toProgress withCompletion:(LottieAdapterCompletionBlock)completion {
}

- (void)playFromProgress:(CGFloat)fromStartProgress
              toProgress:(CGFloat)toEndProgress
          withCompletion:(LottieAdapterCompletionBlock)completion {
}

- (void)playToFrame:(NSNumber *)toFrame withCompletion:(LottieAdapterCompletionBlock)completion {
}

- (void)playFromFrame:(NSNumber *)fromStartFrame
              toFrame:(NSNumber *)toEndFrame
       withCompletion:(LottieAdapterCompletionBlock)completion {
}

- (void)stop {
}

- (void)pause {
}

- (void)setProgressWithFrame:(NSNumber *)currentFrame {
}

- (void)forceDrawingUpdate {
}

- (void)setProgressWithFrame:(nonnull NSNumber *)currentFrame
    callCompletionIfNecessary:(BOOL)callCompletion {
}

- (UIView *)contentView {
  return nil;
}

- (UIViewContentMode)contentMode {
  return UIViewContentModeScaleToFill;
}

- (void)setContentMode:(UIViewContentMode)contentMode {
}

- (void)setMaxFrameRate:(double)maxFrameRate {
}

- (void)setValueDelegate:(id<LottieValueDelegate>)delegate forKeyPath:(LottieKeyPath *)keyPath {
}

- (nullable NSArray *)keysForKeyPath:(nonnull LottieKeyPath *)keyPath {
  return nil;
}

- (void)setValueDelegate:(id<LottieValueDelegate> _Nonnull)delegates
              forKeypath:(LottieKeyPath *_Nonnull)keypath {
}

@end
