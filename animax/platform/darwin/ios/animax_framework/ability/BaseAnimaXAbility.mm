// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/BaseAnimaXAbility.h>
#include "src/player/animax_event.h"

#pragma mark - Private Interface

@interface BaseAnimaXAbility ()
@property(nonatomic, strong, readonly) AnimaXServiceRegistry *serviceRegistry;
@property(nonatomic, strong) NSHashTable<id<AnimaXAnimationListener>> *animationListeners;
@property(nonatomic, strong) NSLock *animationListenersLock;
@property(nonatomic, strong, readonly) AnimaXMonitorAbilityDelegate *monitorDelegate;
@end

#pragma mark - Implementation

@implementation BaseAnimaXAbility

#pragma mark - Lifecycle

- (instancetype)init {
  self = [super init];
  if (self) {
    _serviceRegistry = [[AnimaXServiceRegistry alloc] initWithScope:[self getServiceScope]];
    _animationListeners = [NSHashTable weakObjectsHashTable];
    _animationListenersLock = [[NSLock alloc] init];
    _monitorDelegate =
        [[AnimaXMonitorAbilityDelegate alloc] initWithServiceRegistry:_serviceRegistry];
    [self addAnimationListener:_monitorDelegate];
  }
  return self;
}

#pragma mark - Service Management

- (nullable id<AnimaXService>)getService:(Protocol *)proto {
  return [self.serviceRegistry getService:proto];
}

- (void)registerService:(Protocol *)proto withImpl:(id<AnimaXService>)impl {
  [self.serviceRegistry registerService:proto withImpl:impl];
}

- (void)unregisterService:(Protocol *)proto {
  [self.serviceRegistry unregisterService:proto];
}

#pragma mark - Animation Listener Management

- (void)addAnimationListener:(id<AnimaXAnimationListener>)listener {
  [self.animationListenersLock lock];
  [self.animationListeners addObject:listener];
  [self.animationListenersLock unlock];
}

- (void)removeAnimationListener:(id<AnimaXAnimationListener>)listener {
  [self.animationListenersLock lock];
  [self.animationListeners removeObject:listener];
  [self.animationListenersLock unlock];
}

- (NSHashTable<id<AnimaXAnimationListener>> *)allListeners {
  NSHashTable<id<AnimaXAnimationListener>> *listeners = nil;
  [self.animationListenersLock lock];
  listeners = [self.animationListeners copy];
  [self.animationListenersLock unlock];
  return listeners;
}

#pragma mark - Event Handling

- (void)handleEvent:(void *)eventPtr params:(NSDictionary *)params {
  auto event = static_cast<lynx::animax::Event>(reinterpret_cast<uintptr_t>(eventPtr));

  NSHashTable<id<AnimaXAnimationListener>> *listeners = [self allListeners];
  if (![listeners count]) {
    return;
  }

  // Dispatch events to all registered listeners
  switch (event) {
    case lynx::animax::Event::kCompletion:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onCompletion:)]) {
          [listener onCompletion:params];
        }
      }
      break;

    case lynx::animax::Event::kStart:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onStart:)]) {
          [listener onStart:params];
        }
      }
      break;

    case lynx::animax::Event::kRepeat:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onRepeat:)]) {
          [listener onRepeat:params];
        }
      }
      break;

    case lynx::animax::Event::kCancel:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onCancel:)]) {
          [listener onCancel:params];
        }
      }
      break;

    case lynx::animax::Event::kReady:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onReady:)]) {
          [listener onReady:params];
        }
      }
      break;

    case lynx::animax::Event::kUpdate:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onUpdate:)]) {
          [listener onUpdate:params];
        }
      }
      break;

    case lynx::animax::Event::kError:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onError:)]) {
          [listener onError:params];
        }
      }
      break;

    case lynx::animax::Event::kWarning:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onWarning:)]) {
          [listener onWarning:params];
        }
      }
      break;

    case lynx::animax::Event::kFps:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onFps:)]) {
          [listener onFps:params];
        }
      }
      break;

    case lynx::animax::Event::kTapLayer:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onTapLayers:)]) {
          [listener onTapLayers:params];
        }
      }
      break;

    case lynx::animax::Event::kFirstFrame:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onFirstFrame:)]) {
          [listener onFirstFrame:params];
        }
      }
      break;

    case lynx::animax::Event::kCompositionReady:
      for (id<AnimaXAnimationListener> listener in listeners) {
        if ([listener respondsToSelector:@selector(onCompositionReady:)]) {
          [listener onCompositionReady:params];
        }
      }
      break;

    default:
      break;
  }
}

#pragma mark - Monitor Delegate

- (AnimaXMonitorAbilityDelegate *)getMonitorDelegate {
  return _monitorDelegate;
}

#pragma mark - Service Scope

- (AnimaXServiceScope *)getServiceScope {
  return [AnimaXServiceScope defaultScope];
}

#pragma mark - Dealloc

- (void)dealloc {
  [self.serviceRegistry releaseAll];
}

@end
