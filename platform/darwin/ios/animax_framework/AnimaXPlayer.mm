// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXPlayer.h>

#import <AnimaX/AnimaXKeyPathListCallback.h>
#import <AnimaX/AnimaXPropertyCallback.h>
#import <AnimaX/AnimaXResourceFactoryService.h>
#import <AnimaX/AnimaXSurfaceDrawable.h>
#import "AnimaXComposition+Internal.h"
#import "AnimaXContext+Internal.h"
#import "AnimaXPropertyUpdater.h"
#import "AnimaXResourceLoaderHolder.h"
#import "AnimaXSurfaceDrawable+Internal.h"

#include <map>
#include <string>

#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "src/base/thread/task_runner.h"
#include "src/base/util/ios/lifecycle_manager.h"
#include "src/player/ios/animax_ability_ios.h"
#include "src/player/ios/animax_surface_ios.h"

namespace lynx {
namespace animax {
static id EventParamValueToNSObject(const EventParamValue &value) {
  switch (value.type) {
    case EventParamValue::Type::kInt32:
      if (value.int_val) {
        return @(*value.int_val);
      }
      break;
    case EventParamValue::Type::kDouble:
      if (value.double_val) {
        return @(*value.double_val);
      }
      break;
    case EventParamValue::Type::kString:
      if (value.string_val) {
        return [NSString stringWithUTF8String:value.string_val->c_str()];
      }
      break;
    case EventParamValue::Type::kStringVector:
      if (value.string_vector_val) {
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:value.string_vector_val->size()];
        for (const auto &str : *value.string_vector_val) {
          [array addObject:[NSString stringWithUTF8String:str.c_str()]];
        }
        return array;
      }
      break;
    default:
      break;
  }
  return nil;
}

static NSDictionary<NSString *, id> *MapToNSDictionary(const lynx::animax::EventParamMap &map) {
  NSMutableDictionary<NSString *, id> *dict =
      [NSMutableDictionary dictionaryWithCapacity:map.size()];
  for (const auto &pair : map) {
    NSString *key = [NSString stringWithUTF8String:pair.first.c_str()];
    id value = EventParamValueToNSObject(pair.second);
    if (key && value) {
      dict[key] = value;
    }
  }
  return [dict copy];
}

}  // namespace animax
}  // namespace lynx

@interface AnimaXPlayer ()
@property(nonatomic) AnimaXResourceLoaderHolder *resourceLoader;
@property(nonatomic, strong, readwrite) AnimaXContext *context;
@end

@implementation AnimaXPlayer {
  std::shared_ptr<lynx::animax::AnimaXPlayer> _player;
  AnimaXPropertyUpdater *_propertyUpdater;
  CGFloat _width;
  CGFloat _height;
  int _visibilityFlag;
}

- (instancetype)initWithContext:(AnimaXContext *)context {
  return [self initWithContext:context scale:[UIScreen mainScreen].scale];
}

- (instancetype)initWithContext:(AnimaXContext *)context scale:(CGFloat)scale {
  if (self = [super init]) {
    _context = context;
    [self createPlayerWithScale:scale];
  }
  return self;
}

- (void)dealloc {
  [[self.context monitorDelegate] onRelease];
  if (_player) {
    _player->Destroy();
    _player = nullptr;
  }
}

- (void *)getPlayerRaw {
  return _player.get();
}

- (std::shared_ptr<lynx::animax::AnimaXPlayer>)getPlayerShared {
  return _player;
}

#pragma mark - IAnimaXPlayer

- (void)setImageFolder:(NSString *)imageFolder {
  if (![imageFolder length]) {
    return;
  }
  _player->SetImageFolder([imageFolder UTF8String]);
}

- (void)setSrc:(NSString *)src {
  [self setSrc:src inBundle:nil];
}

- (void)setSrc:(NSString *)src inBundle:(NSBundle *)bundle {
  if (![src length]) {
    return;
  }
  [_resourceLoader setAssetBundle:bundle];

  _player->SetSrc([src UTF8String]);
  [[self.context monitorDelegate] updateUrl:src];
}

- (void)setComposition:(AnimaXComposition *)composition {
  if (!composition) {
    return;
  }

  auto model = reinterpret_cast<std::shared_ptr<lynx::animax::CompositionModel> *>(
      [composition internalModel]);
  if (model) {
    _player->SetComposition(*model);
  }
  [[self.context monitorDelegate] updateUrl:@"composition"];
}

- (void)setPolyfill:(NSDictionary *)polyfill {
  __block std::unordered_map<std::string, std::string> stdPolyfill;
  [polyfill
      enumerateKeysAndObjectsUsingBlock:^(id _Nonnull key, id _Nonnull obj, BOOL *_Nonnull stop) {
        if ([key isKindOfClass:[NSString class]] && [key isKindOfClass:[NSString class]]) {
          stdPolyfill[std::string([key UTF8String])] = std::string([obj UTF8String]);
        }
      }];
  _player->SetSrcPolyfill(stdPolyfill);
}

- (void)setJson:(NSString *)json {
  [self setJson:json inBundle:nil];
}

- (void)setJson:(NSString *)json inBundle:(NSBundle *)bundle {
  if (![json length]) {
    return;
  }
  [_resourceLoader setAssetBundle:bundle];

  _player->SetJson([json UTF8String]);
  [[self.context monitorDelegate] updateUrl:@"json"];
}

- (void)setLoop:(BOOL)enable {
  _player->SetLoop(enable);
}

- (void)setStartFrame:(double)startFrame {
  _player->SetStartFrame(startFrame);
}

- (void)setEndFrame:(double)endFrame {
  _player->SetEndFrame(endFrame);
}

- (void)setAutoReverse:(BOOL)enable {
  _player->SetAutoReverse(enable);
}

- (void)setProgress:(double)progress {
  _player->SetProgress(progress);
}

- (void)setLoopCount:(int)loopCount {
  _player->SetLoopCount(loopCount);
}

- (void)setObjectfit:(NSString *)objectfit {
  if (![objectfit length]) {
    return;
  }
  lynx::animax::ObjectFit objectFit = lynx::animax::ObjectFit::kContain;
  if ([@"cover" isEqualToString:objectfit]) {
    objectFit = lynx::animax::ObjectFit::kCover;
  } else if ([@"center" isEqualToString:objectfit]) {
    objectFit = lynx::animax::ObjectFit::kCenter;
  } else if ([@"fill" isEqualToString:objectfit]) {
    objectFit = lynx::animax::ObjectFit::kFill;
  } else if ([@"scale-down" isEqualToString:objectfit]) {
    objectFit = lynx::animax::ObjectFit::kScaleDown;
  }
  _player->SetObjectFit(objectFit);
}

- (void)setObjectPosition:(NSString *)objectPosition {
  if (![objectPosition length]) {
    return;
  }
  lynx::animax::ObjectPosition objectPos = lynx::animax::ObjectPosition::kCenter;
  if ([@"left" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kLeft;
  } else if ([@"right" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kRight;
  } else if ([@"top" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kTop;
  } else if ([@"bottom" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kBottom;
  } else if ([@"top-left" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kTopLeft;
  } else if ([@"top-right" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kTopRight;
  } else if ([@"bottom-left" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kBottomLeft;
  } else if ([@"bottom-right" isEqualToString:objectPosition]) {
    objectPos = lynx::animax::ObjectPosition::kBottomRight;
  }
  _player->SetObjectPosition(objectPos);
}

- (void)setAutoplay:(BOOL)enable {
  _player->SetAutoplay(enable);
}

- (void)setSpeed:(double)speed {
  _player->SetSpeed(speed);
}

- (void)setFPSEventInterval:(long)interval {
  _player->SetFpsEventInterval(interval);
}

- (void)setMaxFrameRate:(double)maxFrameRate {
  _player->SetMaxFrameRate(maxFrameRate);
}

- (void)setDynamicResource:(BOOL)enable {
  _player->SetDynamicResource(enable);
}

- (void)subscribeUpdateEvent:(int)frame {
  _player->SubscribeUpdateEvent(frame);
}

- (void)unsubscribeUpdateEvent:(int)frame {
  _player->UnsubscribeUpdateEvent(frame);
}

- (void)subscribeUpdateEvents:(NSArray *)frames subscribe:(BOOL)subscribe {
  std::unordered_set<int> frames_set;
  for (NSNumber *frame in frames) {
    frames_set.insert(frame.intValue);
  }
  _player->SubscribeUpdateEvents(std::move(frames_set), subscribe);
}

- (void)play {
  [[self.context monitorDelegate] onPlay];
  _player->Play();
}

- (void)playFrom:(double)startFrame to:(double)endFrame {
  [[self.context monitorDelegate] onPlaySegment];
  _player->PlaySegment(startFrame, endFrame);
}

- (void)resume {
  [[self.context monitorDelegate] onResume];
  _player->Resume();
}

- (void)stop {
  _player->Stop();
}

- (void)pause {
  _player->Pause();
}

- (void)seekTo:(double)frame {
  _player->Seek(frame);
}

- (double)durationInMS {
  return _player->GetDurationMs();
}

- (BOOL)isAnimating {
  return _player->IsAnimating();
}

- (double)currentFrame {
  return _player->GetCurrentFrame();
}

#pragma mark - Reload
- (void)reload {
  _player->Reload();
}

#pragma mark - Tap
- (void)handleTapAtPoint:(CGPoint)point {
  if (_player) {
    _player->OnTap(point.x, point.y);
  }
}

#pragma mark - Native AnimaXPlayer
- (void)createPlayerWithScale:(CGFloat)scale {
  using lynx::animax::AnimaXPlayer;
  using lynx::animax::Event;
  using lynx::animax::EventParamMap;

  lynx::animax::AnimaXPlayerBuilder builder;
  builder.SetScale(static_cast<float>(scale))
      .EnableMultiThreadAccelerate(_context.enableMultiThreadAccelerate)
      .DisablePlaybackOnAssetLoadFailure(_context.disablePlaybackOnAssetLoadFailure)
      .SetAbility(std::make_shared<lynx::animax::AnimaXAbilityIOS>(_context));

  __weak typeof(self) weakSelf = self;
  builder.AddEventListener(
      [weakSelf](AnimaXPlayer * /*player*/, const Event event, const EventParamMap &params) {
        if (!weakSelf) {
          return;
        }
        NSDictionary *eventParams = lynx::animax::MapToNSDictionary(params);
        dispatch_async(dispatch_get_main_queue(), ^{
          [weakSelf onEvent:event params:eventParams];
        });
      });

  _resourceLoader = [AnimaXResourceLoaderHolder loaderForPlayer:&builder context:_context];

  _player = builder.Build();
  [[self.context monitorDelegate] setAnimaXPlayer:_player.get()];
  lynx::animax::LifecycleManager::Instance().AddListener(_player);
}

#pragma mark - Visibility
- (void)onHide:(VisibilityState)state {
  _player->OnHide(static_cast<lynx::animax::VisibilityState>(state));
}

- (void)onShow:(VisibilityState)state {
  _player->OnShow(static_cast<lynx::animax::VisibilityState>(state));
}

- (void)enterForeground {
  [self updateVisibilityState:YES forType:VisibilityState::kBackground];
}

- (void)enterBackground {
  [self updateVisibilityState:NO forType:VisibilityState::kBackground];
}

- (BOOL)updateVisibilityFlagIfChanged:(BOOL)isVisible forType:(VisibilityState)state {
  BOOL currentIsVisible = (_visibilityFlag & state) == 0;
  if (isVisible == currentIsVisible) {
    return NO;
  }
  _visibilityFlag ^= state;
  return YES;
}

- (void)updateVisibilityState:(BOOL)visible forType:(VisibilityState)state {
  if (![self updateVisibilityFlagIfChanged:visible forType:state]) {
    return;
  }
  if (visible) {
    [self onShow:state];
  } else {
    [self onHide:state];
  }
}

#pragma mark - Event
- (void)onEvent:(lynx::animax::Event)event params:(NSDictionary *_Nonnull)params {
  // internal listeners
  if (event == lynx::animax::Event::kCompositionReady) {
    [_compositionListener onCompositionReady];
  }

  // external listeners
  if (self.context.ability) {
    void *eventPtr = (void *)(uintptr_t)event;
    [self.context.ability handleEvent:eventPtr params:params];
  }
}

- (void)adoptAnimaXSurfaceDrawable:(AnimaXSurfaceDrawable *)drawable {
  _player->UpdateSurface([drawable](std::unique_ptr<lynx::animax::AnimaXSurface> old_surface) {
    if (old_surface) {
      return lynx::animax::AnimaXSurfaceIOS::Reconfigure(std::move(old_surface),
                                                         {.size = drawable.size});
    } else {
      return lynx::animax::AnimaXSurfaceIOS::Make(drawable);
    }
  });
}

- (void)addAnimationEventListener:(id<AnimaXAnimationListener>)listener {
  [self.context.ability addAnimationListener:listener];
}

- (void)removeAnimationEventListener:(id<AnimaXAnimationListener>)listener {
  [self.context.ability removeAnimationListener:listener];
}

- (AnimaXContext *)getAnimaXContext {
  return _context;
}

#pragma mark - Property Updates
- (void)updateLayerProperty:(AnimaXKeyPath *)keyPath
               propertyType:(LayerPropertyType)propertyType
                      value:(AnimaXValueParam *)value
                   callback:(nullable id<AnimaXPropertyCallback>)callback {
  [self executePropertyOperation:callback
                       operation:^(AnimaXPropertyUpdater *updater) {
                         [updater updateLayerProperty:keyPath
                                         propertyType:propertyType
                                                value:value
                                             callback:callback];
                       }];
}

- (void)updateResourceProperty:(AnimaXKeyPath *)keyPath
                  propertyType:(ResourcePropertyType)propertyType
                         value:(AnimaXValueParam *)value
                      callback:(nullable id<AnimaXPropertyCallback>)callback {
  [self executePropertyOperation:callback
                       operation:^(AnimaXPropertyUpdater *updater) {
                         [updater updateResourceProperty:keyPath
                                            propertyType:propertyType
                                                   value:value
                                                callback:callback];
                       }];
}

- (void)addLayerPropertyCallback:(LayerPropertyType)propertyType
                         keyPath:(AnimaXKeyPath *)keyPath
                   valueCallback:(AnimaXValueCallback *)valueCallback
                        callback:(nullable id<AnimaXPropertyCallback>)callback {
  [self executePropertyOperation:callback
                       operation:^(AnimaXPropertyUpdater *updater) {
                         [updater addLayerPropertyCallback:propertyType
                                                   keyPath:keyPath
                                             valueCallback:valueCallback
                                                  callback:callback];
                       }];
}

- (void)executePropertyOperation:(nullable id<AnimaXPropertyCallback>)callback
                       operation:(void (^)(AnimaXPropertyUpdater *))operation {
  [self ensurePropertyUpdater];
  if (operation) {
    operation(_propertyUpdater);
  }
}

- (void)getKeysForKeyPath:(AnimaXKeyPath *)keyPath
                 callback:(id<AnimaXKeyPathListCallback>)callback {
  [self ensurePropertyUpdater];
  [_propertyUpdater getKeysForKeyPath:keyPath callback:callback];
}

- (void)ensurePropertyUpdater {
  if (!_propertyUpdater) {
    _propertyUpdater = [[AnimaXPropertyUpdater alloc] initWithPlayer:&_player];
  }
}

@end
