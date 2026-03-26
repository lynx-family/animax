// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "LottieAdapterView.h"
#import "LottieAdapterFactory.h"

@implementation LottieAdapterView {
  id<LottieAdapterProtocol> _adapterImpl;
}

- (instancetype)init {
  return [self initWithOptions:nil];
}

- (instancetype)initWithOptions:(nullable LottieAdapterOption *)options {
  self = [super initWithFrame:CGRectZero];
  if (self) {
    LottieAdapterRequest *request = [LottieAdapterRequest animationWithOptions:options];
    id<LottieAdapterProtocol> impl = [LottieAdapterFactory createAdapterWithRequest:request];
    [self setupWithImpl:impl];
  }
  return self;
}

- (instancetype)initWithName:(NSString *)name bundle:(NSBundle *)bundle {
  return [self initWithName:name bundle:bundle options:nil];
}

- (instancetype)initWithName:(NSString *)name
                      bundle:(NSBundle *_Nullable)bundle
                     options:(nullable LottieAdapterOption *)options {
  self = [super initWithFrame:CGRectZero];
  if (self) {
    LottieAdapterRequest *request = [LottieAdapterRequest animationNamed:name
                                                                inBundle:bundle
                                                                 options:options];
    id<LottieAdapterProtocol> impl = [LottieAdapterFactory createAdapterWithRequest:request];
    [self setupWithImpl:impl];
  }
  return self;
}

#pragma mark - Factory Methods

+ (instancetype)animationNamed:(NSString *)animationName {
  return [LottieAdapterView animationNamed:animationName inBundle:nil options:nil];
}

+ (instancetype)animationNamed:(NSString *)animationName inBundle:(NSBundle *)bundle {
  return [LottieAdapterView animationNamed:animationName inBundle:bundle options:nil];
}

+ (instancetype)animationFromJSON:(NSDictionary *)animationJSON {
  return [LottieAdapterView animationFromJSON:animationJSON inBundle:nil options:nil];
}

+ (instancetype)animationFromJSON:(NSDictionary *)animationJSON inBundle:(NSBundle *)bundle {
  return [LottieAdapterView animationFromJSON:animationJSON inBundle:bundle options:nil];
}

+ (instancetype)animationWithFilePath:(NSString *)filePath {
  return [LottieAdapterView animationWithFilePath:filePath options:nil];
}

- (instancetype)initWithContentsOfURL:(NSURL *)url {
  return [self initWithContentsOfURL:url options:nil];
}

+ (instancetype)animationNamed:(NSString *)animationName
                       options:(nullable LottieAdapterOption *)option {
  return [LottieAdapterView animationNamed:animationName inBundle:nil options:nil];
}

+ (instancetype)animationNamed:(NSString *)animationName
                      inBundle:(NSBundle *)bundle
                       options:(nullable LottieAdapterOption *)option {
  LottieAdapterView *view = [[LottieAdapterView alloc] initWithName:animationName
                                                             bundle:bundle
                                                            options:option];
  return view;
}

+ (instancetype)animationFromJSON:(NSDictionary *)animationJSON
                          options:(nullable LottieAdapterOption *)option {
  LottieAdapterView *view = [[LottieAdapterView alloc] init];
  LottieAdapterRequest *request = [LottieAdapterRequest animationFromJSON:animationJSON
                                                                 inBundle:nil
                                                                  options:option];
  id<LottieAdapterProtocol> impl = [LottieAdapterFactory createAdapterWithRequest:request];
  [view setupWithImpl:impl];
  return view;
}

+ (instancetype)animationFromJSON:(NSDictionary *)animationJSON
                         inBundle:(NSBundle *_Nullable)bundle
                          options:(nullable LottieAdapterOption *)option {
  LottieAdapterView *view = [[LottieAdapterView alloc] init];
  LottieAdapterRequest *request = [LottieAdapterRequest animationFromJSON:animationJSON
                                                                 inBundle:bundle
                                                                  options:option];
  id<LottieAdapterProtocol> impl = [LottieAdapterFactory createAdapterWithRequest:request];
  [view setupWithImpl:impl];
  return view;
}

+ (instancetype)animationWithFilePath:(NSString *)filePath
                              options:(nullable LottieAdapterOption *)option {
  LottieAdapterView *view = [[LottieAdapterView alloc] init];
  LottieAdapterRequest *request = [LottieAdapterRequest animationWithFilePath:filePath
                                                                      options:option];
  id<LottieAdapterProtocol> impl = [LottieAdapterFactory createAdapterWithRequest:request];
  [view setupWithImpl:impl];
  return view;
}

- (instancetype)initWithContentsOfURL:(NSURL *)url options:(nullable LottieAdapterOption *)option {
  self = [super initWithFrame:CGRectZero];
  if (self) {
    LottieAdapterRequest *request = [LottieAdapterRequest animationWithURL:url options:option];
    id<LottieAdapterProtocol> impl = [LottieAdapterFactory createAdapterWithRequest:request];
    [self setupWithImpl:impl];
  }
  return self;
}

#pragma mark - Private Methods

- (void)setupWithImpl:(id<LottieAdapterProtocol>)impl {
  _adapterImpl = impl;
  UIView *implView = impl.contentView;
  implView.frame = self.bounds;
  implView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
  [self addSubview:implView];
}

#pragma mark - LottieAdapterProtocol Implementation

- (void)setAnimationNamed:(NSString *)animationName {
  [_adapterImpl setAnimationNamed:animationName];
}

- (void)setAnimationNamed:(NSString *)animationName inBundle:(NSBundle *)bundle {
  [_adapterImpl setAnimationNamed:animationName inBundle:bundle];
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON {
  [_adapterImpl setAnimationFromJSON:animationJSON];
}

- (void)setAnimationFromJSON:(NSDictionary *)animationJSON inBundle:(NSBundle *)bundle {
  [_adapterImpl setAnimationFromJSON:animationJSON inBundle:bundle];
}

- (BOOL)isAnimationPlaying {
  return _adapterImpl.isAnimationPlaying;
}

- (BOOL)loopAnimation {
  return _adapterImpl.loopAnimation;
}

- (void)setLoopAnimation:(BOOL)loopAnimation {
  _adapterImpl.loopAnimation = loopAnimation;
}

- (BOOL)autoReverseAnimation {
  return _adapterImpl.autoReverseAnimation;
}

- (void)setAutoReverseAnimation:(BOOL)autoReverseAnimation {
  _adapterImpl.autoReverseAnimation = autoReverseAnimation;
}

- (CGFloat)animationProgress {
  return _adapterImpl.animationProgress;
}

- (void)setAnimationProgress:(CGFloat)animationProgress {
  _adapterImpl.animationProgress = animationProgress;
}

- (CGFloat)animationSpeed {
  return _adapterImpl.animationSpeed;
}

- (void)setAnimationSpeed:(CGFloat)animationSpeed {
  _adapterImpl.animationSpeed = animationSpeed;
}

- (CGFloat)animationDuration {
  return _adapterImpl.animationDuration;
}

- (BOOL)cacheEnable {
  return _adapterImpl.cacheEnable;
}

- (void)setCacheEnable:(BOOL)cacheEnable {
  _adapterImpl.cacheEnable = cacheEnable;
}

- (void)setCompletionBlock:(LottieAdapterCompletionBlock)completionBlock {
  _adapterImpl.completionBlock = completionBlock;
}

- (LottieAdapterCompletionBlock)completionBlock {
  return _adapterImpl.completionBlock;
}

- (BOOL)shouldRasterizeWhenIdle {
  return _adapterImpl.shouldRasterizeWhenIdle;
}

- (void)setShouldRasterizeWhenIdle:(BOOL)shouldRasterizeWhenIdle {
  _adapterImpl.shouldRasterizeWhenIdle = shouldRasterizeWhenIdle;
}

- (NSUInteger)repeatCount {
  return _adapterImpl.repeatCount;
}

- (void)setRepeatCount:(NSUInteger)repeatCount {
  _adapterImpl.repeatCount = repeatCount;
}

- (void)play {
  [_adapterImpl play];
}

- (void)playWithCompletion:(LottieAdapterCompletionBlock)completion {
  [_adapterImpl playWithCompletion:completion];
}

- (void)playToProgress:(CGFloat)toProgress withCompletion:(LottieAdapterCompletionBlock)completion {
  [_adapterImpl playToProgress:toProgress withCompletion:completion];
}

- (void)playFromProgress:(CGFloat)fromStartProgress
              toProgress:(CGFloat)toEndProgress
          withCompletion:(LottieAdapterCompletionBlock)completion {
  [_adapterImpl playFromProgress:fromStartProgress
                      toProgress:toEndProgress
                  withCompletion:completion];
}

- (void)playToFrame:(NSNumber *)toFrame withCompletion:(LottieAdapterCompletionBlock)completion {
  [_adapterImpl playToFrame:toFrame withCompletion:completion];
}

- (void)playFromFrame:(NSNumber *)fromStartFrame
              toFrame:(NSNumber *)toEndFrame
       withCompletion:(LottieAdapterCompletionBlock)completion {
  [_adapterImpl playFromFrame:fromStartFrame toFrame:toEndFrame withCompletion:completion];
}

- (void)stop {
  [_adapterImpl stop];
}

- (void)pause {
  [_adapterImpl pause];
}

- (void)setProgressWithFrame:(NSNumber *)currentFrame {
  [_adapterImpl setProgressWithFrame:currentFrame];
}

- (void)forceDrawingUpdate {
  [_adapterImpl forceDrawingUpdate];
}

- (UIView *)contentView {
  return [_adapterImpl contentView];
}

- (UIViewContentMode)contentMode {
  return [_adapterImpl contentMode];
}

- (void)setContentMode:(UIViewContentMode)contentMode {
  _adapterImpl.contentMode = contentMode;
}

- (void)setMaxFrameRate:(double)maxFrameRate {
  [_adapterImpl setMaxFrameRate:maxFrameRate];
}

- (void)setValueDelegate:(id<LottieValueDelegate> _Nonnull)delegates
              forKeypath:(LottieKeyPath *_Nonnull)keypath {
  [_adapterImpl setValueDelegate:delegates forKeypath:keypath];
}

- (nullable NSArray *)keysForKeyPath:(nonnull LottieKeyPath *)keyPath {
  return [_adapterImpl keysForKeyPath:keyPath];
}

- (void)setProgressWithFrame:(nonnull NSNumber *)currentFrame
    callCompletionIfNecessary:(BOOL)callCompletion {
  [_adapterImpl setProgressWithFrame:currentFrame callCompletionIfNecessary:callCompletion];
}

@end
