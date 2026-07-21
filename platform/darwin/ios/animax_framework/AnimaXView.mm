// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXView.h>

#import <AnimaX/AnimaX.h>
#import <AnimaX/AnimaXPlayer.h>
#import <AnimaX/AnimaXResourceFactoryService.h>
#import <AnimaX/AnimaXSurfaceDrawable.h>
#import <Metal/Metal.h>
#import <UIKit/UIKit.h>
#import "AnimaXComposition+Internal.h"
#import "AnimaXContext+Internal.h"
#import "AnimaXLayer.h"
#import "AnimaXResourceLoaderHolder.h"
#include "include/base/player_event_tracker.h"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"
#include "src/base/util/ios/ca_util.h"
#include "src/base/util/ios/lifecycle_manager.h"
#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/model/text/font_character_model.h"

@interface AnimaXView () <AnimaXLayerProtocol, AnimaXApplicationLifecycleListener>

@property(strong, nonatomic, nullable) CAMetalLayer *gpuLayer;
@property(nonatomic, nonnull, readwrite) id<AnimaXPlayerProtocol> player;
@property(nonatomic, strong) AnimaXSurfaceDrawable *drawable;
@end

@implementation AnimaXView

+ (Class)layerClass {
  return [AnimaXLayer class];
}

- (instancetype)initWithContext:(AnimaXContext *)context {
  // Register global log function once before init AnimaXView
  [[AnimaX shareInstance] registerLoggerOnce];
  if (self = [super initWithFrame:CGRectZero]) {
    self.layer.opaque = NO;
    self.layer.contentsScale = [UIScreen mainScreen].scale;
    // create gpu layer later to allow front-end user change backend type
    self.gpuLayer = nil;
    self.enableNativeTapLayerEvent = NO;
    self.ignoreAttachStatus = NO;
    AnimaXPlayer *player = [[AnimaXPlayer alloc] initWithContext:context];
    player.compositionListener = self;
    _player = player;
    lynx::animax::LifecycleManager::Instance().AddApplicationLifecycleListener(self);
    [self notifyFrameChanged];
  }
  return self;
}

- (instancetype)initWithPlayer:(AnimaXPlayer *)player {
  // Register global log function once before init AnimaXView
  [[AnimaX shareInstance] registerLoggerOnce];
  if (self = [super initWithFrame:CGRectZero]) {
    self.layer.opaque = NO;
    self.layer.contentsScale = [UIScreen mainScreen].scale;
    // create gpu layer later to allow front-end user change backend type
    self.enableNativeTapLayerEvent = NO;
    self.ignoreAttachStatus = NO;
    player.compositionListener = self;
    _player = player;
    lynx::animax::LifecycleManager::Instance().AddApplicationLifecycleListener(self);
    [self notifyFrameChanged];
  }
  return self;
}

- (void)dealloc {
  // compatible with legacy logic
  _player = nil;
}

- (void)onAnimaXApplicationDidBecomeActive {
  [self notifyFrameChanged];
}

- (BOOL)isAnimaXSurfaceCreationAllowed {
  return lynx::animax::LifecycleManager::Instance().IsApplicationActive();
}

- (void)initGPULayer {
  // do not recreate gpu layer even if backend type is changed
  if (self.gpuLayer) {
    return;
  }

  self.gpuLayer = lynx::animax::CreateCAMetalLayer(self.frame);
  [self.layer addSublayer:self.gpuLayer];
  self.drawable = [[AnimaXSurfaceDrawable alloc] initWithMetalLayer:self.gpuLayer
                                                              scale:self.layer.contentsScale];
}

#pragma mark - FrameChanged

- (void)setFrame:(CGRect)frame {
  [super setFrame:frame];
  [self notifyFrameChanged];
}

- (void)setBounds:(CGRect)bounds {
  [super setBounds:bounds];
  [self notifyFrameChanged];
}

- (void)notifyFrameChanged {
  const CGFloat scale = self.layer.contentsScale;
  const CGFloat width = scale * self.bounds.size.width;
  const CGFloat height = scale * self.bounds.size.height;

  // The invalid size will cause the update fail directly
  auto sizeIsValid = (width > 0) && (height > 0);
  [self.player updateVisibilityState:sizeIsValid forType:kSize];

  if (!sizeIsValid) {
    ANIMAX_LOGI("Skip invalid size: width: " << width << " height: " << height);
    return;
  }
  if (![self isAnimaXSurfaceCreationAllowed]) {
    return;
  }
  if (!self.drawable) {
    [self initGPULayer];
  } else if ([self.drawable updateSize:CGSizeMake(width, height)]) {
    lynx::animax::ResizeCALayer(self.gpuLayer, self.bounds.size);
  } else {
    return;
  }
  [self.player adoptAnimaXSurfaceDrawable:self.drawable];
}

- (void)ensureAnimaXSurface {
  if (self.drawable || !self.ignoreAttachStatus) {
    return;
  }

  if (CGRectGetWidth(self.bounds) < 1 || CGRectGetHeight(self.bounds) < 1) {
    self.bounds = CGRectMake(self.bounds.origin.x, self.bounds.origin.y, 1, 1);
  }
}

#pragma mark - Window
- (void)didMoveToWindow {
  [super didMoveToWindow];
  if (self.ignoreAttachStatus) {
    return;
  }
  BOOL attachToWindow = self.window != nullptr;
  [_player updateVisibilityState:attachToWindow forType:VisibilityState::kAttach];
}

#pragma mark - Hidden
- (void)setHidden:(BOOL)hidden {
  [super setHidden:hidden];
  [_player updateVisibilityState:!hidden forType:kVisible];
}

#pragma mark - Alpha
- (void)handleAlphaChange:(CGFloat)oldAlpha newAlpha:(CGFloat)newAlpha {
  BOOL wasVisible = oldAlpha > 0.0;
  BOOL isVisible = newAlpha > 0.0;

  if (wasVisible != isVisible) {
    [_player updateVisibilityState:isVisible forType:kOpacity];
  }
}

#pragma mark - Event

- (void)onCompositionReady {
  [self ensureAnimaXSurface];
}

#pragma mark - Tap
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  [super touchesEnded:touches withEvent:event];
  if (self.enableNativeTapLayerEvent) {
    [self handleTouch:touches withEvent:event];
  }
}

- (void)handleTouch:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  UITouch *touch = [touches anyObject];
  CGPoint location = [touch locationInView:self];

  if (_player && self.isUserInteractionEnabled) {
    [_player handleTapAtPoint:CGPointMake(location.x * self.layer.contentsScale,
                                          location.y * self.layer.contentsScale)];
  }
}

#pragma mark Player Delegate

- (void)enterForeground {
  [_player enterForeground];
}

- (void)enterBackground {
  [_player enterBackground];
}

- (void)setImageFolder:(NSString *)imageFolder {
  [_player setImageFolder:imageFolder];
}

- (void)setSrc:(NSString *)src {
  [_player setSrc:src];
}

- (void)setPolyfill:(NSDictionary *)polyfill {
  [_player setPolyfill:polyfill];
}

- (void)setJson:(NSString *)json {
  [_player setJson:json];
}

- (void)setComposition:(AnimaXComposition *)composition {
  [_player setComposition:composition];
}

- (void)setLoop:(BOOL)enable {
  [_player setLoop:enable];
}

- (void)setStartFrame:(double)startFrame {
  [_player setStartFrame:startFrame];
}

- (void)setEndFrame:(double)endFrame {
  [_player setEndFrame:endFrame];
}

- (void)setAutoReverse:(BOOL)enable {
  [_player setAutoReverse:enable];
}

- (void)setProgress:(double)progress {
  [_player setProgress:progress];
}

- (void)setLoopCount:(int)loopCount {
  [_player setLoopCount:loopCount];
}

- (void)setObjectfit:(NSString *)objectfit {
  [_player setObjectfit:objectfit];
}

- (void)setObjectPosition:(NSString *)objectPosition {
  [_player setObjectPosition:objectPosition];
}

- (void)setAutoplay:(BOOL)enable {
  [_player setAutoplay:enable];
}

- (void)setSpeed:(double)speed {
  [_player setSpeed:speed];
}

- (void)setFPSEventInterval:(long)interval {
  [_player setFPSEventInterval:interval];
}

- (void)setMaxFrameRate:(double)maxFrameRate {
  [_player setMaxFrameRate:maxFrameRate];
}

- (void)setDynamicResource:(BOOL)enable {
  [_player setDynamicResource:enable];
}

- (void)setSrc:(NSString *)src inBundle:(NSBundle *)bundle {
  [_player setSrc:src inBundle:bundle];
}

- (void)setJson:(NSString *)json inBundle:(NSBundle *)bundle {
  [_player setJson:json inBundle:bundle];
}

- (void)subscribeUpdateEvent:(int)frame {
  [_player subscribeUpdateEvent:frame];
}

- (void)unsubscribeUpdateEvent:(int)frame {
  [_player unsubscribeUpdateEvent:frame];
}

- (void)subscribeUpdateEvents:(NSArray *)frames subscribe:(BOOL)subscribe {
  [_player subscribeUpdateEvents:frames subscribe:subscribe];
}

- (void)play {
  [_player play];
}

- (void)playFrom:(double)startFrame to:(double)endFrame {
  [_player playFrom:startFrame to:endFrame];
}

- (void)resume {
  [_player resume];
}

- (void)stop {
  [_player stop];
}

- (void)pause {
  [_player pause];
}

- (void)seekTo:(double)frame {
  [_player seekTo:frame];
}

- (double)durationInMS {
  return [_player durationInMS];
}

- (BOOL)isAnimating {
  return [_player isAnimating];
}

- (double)progress {
  return [_player progress];
}

- (double)currentFrame {
  return [_player currentFrame];
}

- (int64_t)memoryUsageBytes {
  return [_player memoryUsageBytes];
}

- (void)reload {
  [_player reload];
}

- (void)addAnimationEventListener:(id<AnimaXAnimationListener>)listener {
  [_player addAnimationEventListener:listener];
}

- (void)removeAnimationEventListener:(id<AnimaXAnimationListener>)listener {
  [_player removeAnimationEventListener:listener];
}

- (void)adoptAnimaXSurfaceDrawable:(AnimaXSurfaceDrawable *)drawable {
  [_player adoptAnimaXSurfaceDrawable:drawable];
}

- (void)handleTapAtPoint:(CGPoint)point {
  [_player handleTapAtPoint:point];
}

- (AnimaXContext *)getAnimaXContext {
  return [_player getAnimaXContext];
}

- (void)updateLayerProperty:(AnimaXKeyPath *)keyPath
               propertyType:(LayerPropertyType)propertyType
                      value:(AnimaXValueParam *)value
                   callback:(nullable id<AnimaXPropertyCallback>)callback {
  [_player updateLayerProperty:keyPath propertyType:propertyType value:value callback:callback];
}

- (void)updateResourceProperty:(AnimaXKeyPath *)keyPath
                  propertyType:(ResourcePropertyType)propertyType
                         value:(AnimaXValueParam *)value
                      callback:(nullable id<AnimaXPropertyCallback>)callback {
  [_player updateResourceProperty:keyPath propertyType:propertyType value:value callback:callback];
}

- (void)addLayerPropertyCallback:(LayerPropertyType)propertyType
                         keyPath:(AnimaXKeyPath *)keyPath
                   valueCallback:(AnimaXValueCallback *)valueCallback
                        callback:(nullable id<AnimaXPropertyCallback>)callback {
  [_player addLayerPropertyCallback:propertyType
                            keyPath:keyPath
                      valueCallback:valueCallback
                           callback:callback];
}

- (void)getKeysForKeyPath:(AnimaXKeyPath *)keyPath
                 callback:(id<AnimaXKeyPathListCallback>)callback {
  [_player getKeysForKeyPath:keyPath callback:callback];
}

- (void)updateVisibilityState:(BOOL)visible forType:(VisibilityState)state {
  [_player updateVisibilityState:visible forType:state];
}

- (void)onHide:(VisibilityState)state {
  [_player onHide:state];
}

- (void)onShow:(VisibilityState)state {
  [_player onShow:state];
}

- (void)setMuted:(BOOL)mute {
  [_player setMuted:mute];
}

- (void)setEnableAudio:(BOOL)enable {
  [_player setEnableAudio:enable];
}

@end
