// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "LottieAdapterAnimaX.h"
#import <AnimaX/AnimaXAnimationListener.h>
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXImageView.h>
#import <AnimaX/AnimaXKeyPath.h>
#import <AnimaX/AnimaXKeyPathListCallback.h>
#import <AnimaX/AnimaXPropertyUpdater.h>
#import <AnimaX/AnimaXView.h>
#import <AnimaX/DefaultAnimaXAbility.h>
#import <AnimaX/LayerPropertyType.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "LottieAdapterProtocol.h"
#import "LottieKeyPath.h"
#import "LottieKeyPathListCallbackAdapter.h"
#import "LottiePropertyCallbackAdapter.h"
#import "LottieValueDelegate.h"
#import "LottieValueDelegateAdapter.h"

@interface LottieAdapterAnimaXAnimationListener : NSObject <AnimaXAnimationListener>
@property(nonatomic, copy) void (^animationStopCallback)(BOOL completion);
@property(nonatomic, weak) LottieAdapterAnimaX *adapter;
@end

@implementation LottieAdapterAnimaX {
  UIView<AnimaXPlayerProtocol, AnimaXViewProtocol> *_animaXView;
  LottieAdapterAnimaXAnimationListener *_animationListener;
  CGFloat _animationSpeed;
  BOOL _autoReverseAnimation;
  NSUInteger _repeatCount;
  BOOL _loopAnimation;
  NSMutableArray<void (^)(void)> *_operationQueue;
  BOOL _isReady;
  CGFloat _modelStartFrame, _modelEndFrame;
}

@synthesize completionBlock;
@synthesize shouldRasterizeWhenIdle;

#pragma mark - Initialization

- (instancetype)init {
  return [self initWithOptions:nil];
}

- (instancetype)initWithOptions:(nullable LottieAdapterOption *)options {
  self = [super init];
  if (self) {
    DefaultAnimaXAbility *ability = [[DefaultAnimaXAbility alloc] init];
    AnimaXContext *context = [[AnimaXContext alloc] initWithAbility:ability];
    if (options) {
      context.enableMultiThreadAccelerate = options.multiThreadAccelerate;
    }
    if (options && options.useImageMode) {
      _animaXView = [[AnimaXImageView alloc] initWithContext:context];
    } else {
      _animaXView = [[AnimaXView alloc] initWithContext:context];
    }
    [_animaXView setAutoplay:NO];
    [_animaXView setMaxFrameRate:60];
    _animationSpeed = 1;
    _operationQueue = [NSMutableArray array];
    _isReady = NO;
    _modelStartFrame = _modelEndFrame = 0;
    _animationListener = [[LottieAdapterAnimaXAnimationListener alloc] init];
    _animationListener.adapter = self;
    __weak typeof(self) weakSelf = self;
    _animationListener.animationStopCallback = ^(BOOL completion) {
      [weakSelf callCompletionIfNecessary:completion];
    };
    [_animaXView addAnimationEventListener:_animationListener];
  }
  return self;
}

#pragma mark - Queue Management

- (void)enqueueOperation:(void (^)(void))operation {
  if (_isReady) {
    operation();
  } else {
    [_operationQueue addObject:operation];
  }
}

- (void)flushOperationQueue {
  _isReady = YES;

  for (void (^operation)(void) in _operationQueue) {
    operation();
  }

  [_operationQueue removeAllObjects];
}

#pragma mark - Modified Animation Control Methods

- (void)play {
  __weak typeof(_animaXView) animaXView = _animaXView;
  [self enqueueOperation:^{
    [animaXView play];
  }];
}

- (void)stop {
  __weak typeof(_animaXView) animaXView = _animaXView;
  [self enqueueOperation:^{
    [animaXView stop];
  }];
}

- (void)pause {
  __weak typeof(_animaXView) animaXView = _animaXView;
  [self enqueueOperation:^{
    [animaXView pause];
  }];
}

#pragma mark - Internal functions

- (void)callCompletionIfNecessary:(BOOL)complete {
  LottieAdapterCompletionBlock completionBlock = self.completionBlock;
  if (completionBlock) {
    self.completionBlock = nil;
    completionBlock(complete);
  }
}

- (void)postSetLoopOrRepeatCount {
  if (!_loopAnimation) {
    [_animaXView setLoop:NO];
    [_animaXView setLoopCount:1];
  } else if (_repeatCount == 0) {
    [_animaXView setLoop:YES];  // loop for ever
    [_animaXView setLoopCount:0];
  } else {
    [_animaXView setLoop:NO];
    [_animaXView setLoopCount:(int)_repeatCount];
  }
}

- (void)setModelStartFrame:(CGFloat)start andEndFrame:(CGFloat)end {
  _modelStartFrame = start;
  _modelEndFrame = end;
}

- (double)progressForFrame:(double)frame {
  static const double kEpsilon = 1e-6;
  if (_modelEndFrame - _modelStartFrame < kEpsilon) {
    return frame <= _modelStartFrame ? 0 : 1;
  }
  double result = (frame - _modelStartFrame) / (_modelEndFrame - _modelStartFrame);
  return MAX(MIN(result, 1), 0);
}

- (double)frameForProgress:(double)progress {
  int resultFrame = _modelStartFrame + (_modelEndFrame - _modelStartFrame) * progress + 0.5;
  return MAX(MIN(resultFrame, _modelEndFrame), _modelStartFrame);
}

- (double)frameAdjusted:(NSNumber *)frame {
  return MAX(MIN(frame.doubleValue, _modelEndFrame), _modelStartFrame);
}

#pragma mark - LottieAdapterProtocol

- (void)setAnimationNamed:(NSString *)animationName {
  [self setAnimationNamed:animationName inBundle:[NSBundle mainBundle]];
}

- (void)setAnimationNamed:(NSString *)animationName inBundle:(NSBundle *)bundle {
  NSString *jsonPath = [bundle pathForResource:animationName ofType:@"json"];
  if (jsonPath) {
    NSURL *url = [NSURL fileURLWithPath:jsonPath];
    [_animaXView setSrc:url.absoluteString inBundle:bundle];
  }
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON {
  [self setAnimationFromJSON:animationJSON inBundle:[NSBundle mainBundle]];
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON inBundle:(NSBundle *)bundle {
  NSData *jsonData = [NSJSONSerialization dataWithJSONObject:animationJSON options:0 error:nil];
  if (jsonData) {
    NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    [_animaXView setSrc:jsonString inBundle:bundle];
  }
}

- (BOOL)isAnimationPlaying {
  return [_animaXView isAnimating];
}

- (BOOL)loopAnimation {
  return _loopAnimation;
}

- (void)setLoopAnimation:(BOOL)loopAnimation {
  _loopAnimation = loopAnimation;
  [self postSetLoopOrRepeatCount];
}

- (BOOL)autoReverseAnimation {
  return _autoReverseAnimation;
}

- (void)setAutoReverseAnimation:(BOOL)autoReverseAnimation {
  _autoReverseAnimation = autoReverseAnimation;
  [_animaXView setAutoReverse:autoReverseAnimation];
}

- (CGFloat)animationProgress {
  if (!_isReady) {
    return 0;
  }
  return [self progressForFrame:[_animaXView currentFrame]];
}

- (void)setAnimationProgress:(CGFloat)animationProgress {
  __weak typeof(_animaXView) animaXView = _animaXView;
  [self enqueueOperation:^{
    [animaXView setProgress:animationProgress];
  }];
}

- (CGFloat)animationSpeed {
  return _animationSpeed;
}

- (void)setAnimationSpeed:(CGFloat)animationSpeed {
  _animationSpeed = animationSpeed;
  [_animaXView setSpeed:animationSpeed];
}

- (CGFloat)animationDuration {
  // TODO(aiyongbiao.rick) fix later
  return [_animaXView durationInMS] + 0.01;
}

- (BOOL)cacheEnable {
  return NO;
}

- (void)setCacheEnable:(BOOL)cacheEnable {
}

- (NSUInteger)repeatCount {
  return _repeatCount;
}

- (void)setRepeatCount:(NSUInteger)repeatCount {
  _repeatCount = repeatCount;
  [self postSetLoopOrRepeatCount];
}

- (void)playWithCompletion:(LottieAdapterCompletionBlock)completion {
  [self setCompletionBlock:completion];
  __weak typeof(_animaXView) animaXView = _animaXView;
  [self enqueueOperation:^{
    [animaXView play];
  }];
}

- (void)playToProgress:(CGFloat)toProgress withCompletion:(LottieAdapterCompletionBlock)completion {
  [self setCompletionBlock:completion];
  __weak typeof(self) weakSelf = self;
  [self enqueueOperation:^{
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf->_animaXView playFrom:strongSelf->_modelStartFrame
                                     to:[strongSelf frameForProgress:toProgress]];
    }
  }];
}

- (void)playFromProgress:(CGFloat)fromStartProgress
              toProgress:(CGFloat)toEndProgress
          withCompletion:(LottieAdapterCompletionBlock)completion {
  [self setCompletionBlock:completion];
  __weak typeof(self) weakSelf = self;
  [self enqueueOperation:^{
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf->_animaXView playFrom:[strongSelf frameForProgress:fromStartProgress]
                                     to:[strongSelf frameForProgress:toEndProgress]];
    }
  }];
}

- (void)playToFrame:(NSNumber *)toFrame withCompletion:(LottieAdapterCompletionBlock)completion {
  [self setCompletionBlock:completion];
  __weak typeof(self) weakSelf = self;
  [self enqueueOperation:^{
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf->_animaXView playFrom:strongSelf->_modelStartFrame
                                     to:[strongSelf frameAdjusted:toFrame]];
    }
  }];
}

- (void)playFromFrame:(NSNumber *)fromStartFrame
              toFrame:(NSNumber *)toEndFrame
       withCompletion:(LottieAdapterCompletionBlock)completion {
  [self setCompletionBlock:completion];
  __weak typeof(self) weakSelf = self;
  [self enqueueOperation:^{
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf->_animaXView playFrom:[strongSelf frameAdjusted:fromStartFrame]
                                     to:[strongSelf frameAdjusted:toEndFrame]];
    }
  }];
}

- (void)setProgressWithFrame:(NSNumber *)currentFrame {
  __weak typeof(self) weakSelf = self;
  [self enqueueOperation:^{
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf->_animaXView setProgress:[strongSelf progressForFrame:currentFrame.doubleValue]];
    }
  }];
}

- (void)forceDrawingUpdate {
  // AnimaX doesn't have direct equivalent
}

- (void)setProgressWithFrame:(nonnull NSNumber *)currentFrame
    callCompletionIfNecessary:(BOOL)callCompletion {
  __weak typeof(self) weakSelf = self;
  [self enqueueOperation:^{
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf->_animaXView setProgress:[strongSelf progressForFrame:currentFrame.doubleValue]];
      if (callCompletion) {
        [strongSelf callCompletionIfNecessary:NO];
      }
    }
  }];
}

- (UIView *)contentView {
  return _animaXView;
}

- (UIViewContentMode)contentMode {
  return [_animaXView contentMode];
}

- (void)setContentMode:(UIViewContentMode)contentMode {
  _animaXView.contentMode = contentMode;

  NSString *fit = @"contain";
  switch (contentMode) {
    case UIViewContentModeScaleAspectFit:
      fit = @"contain";
      break;
    case UIViewContentModeScaleAspectFill:
      fit = @"cover";
      break;
    case UIViewContentModeScaleToFill:
      fit = @"fill";
      break;
    default:
      fit = @"contain";
      break;
  }
  [_animaXView setObjectfit:fit];

  NSString *pos = @"center";
  switch (contentMode) {
    case UIViewContentModeLeft:
      pos = @"left";
      break;
    case UIViewContentModeRight:
      pos = @"right";
      break;
    case UIViewContentModeTop:
      pos = @"top";
      break;
    case UIViewContentModeBottom:
      pos = @"bottom";
      break;
    case UIViewContentModeTopLeft:
      pos = @"top-left";
      break;
    case UIViewContentModeTopRight:
      pos = @"top-right";
      break;
    case UIViewContentModeBottomLeft:
      pos = @"bottom-left";
      break;
    case UIViewContentModeBottomRight:
      pos = @"bottom-right";
      break;
    default:
      pos = @"center";
      break;
  }
  [_animaXView setObjectPosition:pos];
}

- (void)setMaxFrameRate:(double)maxFrameRate {
  if (maxFrameRate >= 1.f) {
    [_animaXView setMaxFrameRate:maxFrameRate];
  }
}

- (void)setValueDelegate:(id<LottieValueDelegate> _Nonnull)delegates
              forKeypath:(LottieKeyPath *_Nonnull)keypath {
  if (!delegates || !keypath) {
    return;
  }

  AnimaXValueCallback *valueCallback =
      [LottieValueDelegateAdapter createAnimaXValueCallbackWithLottieDelegate:delegates
                                                                   forKeyPath:keypath];
  if (!valueCallback) {
    return;
  }

  NSInteger propertyType = [LottieValueDelegateAdapter getLayerPropertyTypeForKeyPath:keypath];
  if (propertyType == 0) {
    return;
  }

  NSArray<NSString *> *trimmedKeys =
      [LottieValueDelegateAdapter trimmedKeysForKeyPath:keypath propertyType:propertyType];
  AnimaXKeyPath *animaXKeyPath = [[AnimaXKeyPath alloc] initWithKeys:trimmedKeys];

  __weak typeof(self) weakSelf = self;
  [self enqueueOperation:^{
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf->_animaXView
          addLayerPropertyCallback:(LayerPropertyType)propertyType
                           keyPath:animaXKeyPath
                     valueCallback:valueCallback
                          callback:[LottiePropertyCallbackAdapter sharedInstance]];
    }
  }];
}

- (nullable NSArray *)keysForKeyPath:(nonnull LottieKeyPath *)keyPath {
  if (!keyPath) {
    return nil;
  }

  NSArray<NSString *> *trimmedKeys = [LottieValueDelegateAdapter trimmedKeysForKeyPath:keyPath
                                                                          propertyType:0];
  AnimaXKeyPath *animaXKeyPath = [[AnimaXKeyPath alloc] initWithKeys:trimmedKeys];
  __block NSMutableArray<LottieKeyPath *> *result = [NSMutableArray array];
  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

  id<AnimaXKeyPathListCallback> callbackAdapter = [[LottieKeyPathListCallbackAdapter alloc]
      initWithBlock:^(NSArray<AnimaXKeyPath *> *keyPaths) {
        for (AnimaXKeyPath *path in keyPaths) {
          NSString *keypathString = [path.keys componentsJoinedByString:@"."];
          LottieKeyPath *lottieKeyPath = [[LottieKeyPath alloc] initWithKeypath:keypathString];
          [result addObject:lottieKeyPath];
        }
        dispatch_semaphore_signal(semaphore);
      }];

  [_animaXView getKeysForKeyPath:animaXKeyPath callback:callbackAdapter];

  // Wait for callback with 100ms timeout
  dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, 0.1 * NSEC_PER_SEC);
  if (dispatch_semaphore_wait(semaphore, timeout) != 0) {
    // Timeout occurred, return empty array
    return @[];
  }

  return [result copy];
}

@end

@implementation LottieAdapterAnimaXAnimationListener

- (void)onReady:(NSDictionary *)params {
  [self parseParamsOnReady:params];
  if (self.adapter) {
    [self.adapter flushOperationQueue];
  }
}

/**
 * Called when the animation completes successfully.
 * Invokes the animation stop callback with TRUE to indicate normal completion.
 *
 * @param params Dictionary containing animation completion parameters
 */
- (void)onCompletion:(NSDictionary *)params {
  if (self.animationStopCallback) {
    self.animationStopCallback(YES);
  }
}

/**
 * Called when the animation is cancelled before completion.
 * Invokes the animation stop callback with FALSE to indicate cancellation.
 *
 * @param params Dictionary containing animation cancellation parameters
 */
- (void)onCancel:(NSDictionary *)params {
  if (self.animationStopCallback) {
    self.animationStopCallback(NO);
  }
}

- (void)parseParamsOnReady:(NSDictionary *)params {
  double currentValue = 0, totalValue = 0;
  NSObject *current = [params objectForKey:@"current"];
  if ([current isKindOfClass:[NSNumber class]]) {
    currentValue = MAX([(NSNumber *)current doubleValue], 0);
  }
  NSObject *total = [params objectForKey:@"total"];
  if ([total isKindOfClass:[NSNumber class]]) {
    totalValue = MAX([(NSNumber *)total doubleValue], 0);
  }
  [self.adapter setModelStartFrame:currentValue andEndFrame:currentValue + totalValue];
}

@end
