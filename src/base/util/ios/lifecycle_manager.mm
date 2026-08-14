// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/ios/lifecycle_manager.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "base/include/no_destructor.h"

@interface AnimaXPlayer (LifecycleManager)
- (void)onAppEnterForeground;
- (void)onAppEnterBackground;
@end

namespace lynx {
namespace animax {

namespace {

bool ShouldNotifyPlayer(::AnimaXPlayer *player, UIScene *scene) {
  UIView *view = player.lifecycleView;
  BOOL is_scene_notification = scene != nil;

  // Without a lifecycle view, the player cannot be matched to a UIScene. In this case, only
  // application-level notifications can be used as a fallback.
  if (!view) {
    return !is_scene_notification;
  }

  if (@available(iOS 13.0, *)) {
    UIScene *player_scene = view.window.windowScene;
    if (is_scene_notification) {
      // Scene notifications should only affect players attached to the same window scene.
      return player_scene == scene;
    } else if (player_scene == nil) {
      // Application notifications are a fallback for legacy apps or players not attached to a
      // scene yet. Players with a scene should rely on scene notifications to avoid duplicate
      // foreground/background callbacks.
      return true;
    } else {
      return false;
    }
  } else {
    // UIScene is unavailable before iOS 13, so only application-level notifications are valid.
    return !is_scene_notification;
  }
}

}  // namespace

LifecycleManager &LifecycleManager::Instance() {
  static base::NoDestructor<LifecycleManager> instance;
  return *instance;
}

LifecycleManager::LifecycleManager() {
  // Ensure the callback runs on main thread to avoid threading issues.
  listeners_ = [NSHashTable weakObjectsHashTable];
  if (@available(iOS 13.0, *)) {
    scene_foreground_observer_ = [[NSNotificationCenter defaultCenter]
        addObserverForName:UISceneWillEnterForegroundNotification
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification *_Nonnull note) {
                  UIScene *scene = (UIScene *)note.object;
                  if (![scene isKindOfClass:UIScene.class]) {
                    return;
                  }
                  this->NotifyAppEnterForeground(scene);
                }];
    scene_background_observer_ = [[NSNotificationCenter defaultCenter]
        addObserverForName:UISceneDidEnterBackgroundNotification
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification *_Nonnull note) {
                  UIScene *scene = (UIScene *)note.object;
                  if (![scene isKindOfClass:UIScene.class]) {
                    return;
                  }
                  this->NotifyAppEnterBackground(scene);
                }];
  }
  app_foreground_observer_ = [[NSNotificationCenter defaultCenter]
      addObserverForName:UIApplicationWillEnterForegroundNotification
                  object:nil
                   queue:[NSOperationQueue mainQueue]
              usingBlock:^(NSNotification *_Nonnull note) {
                this->NotifyAppEnterForeground(nil);
              }];
  app_background_observer_ = [[NSNotificationCenter defaultCenter]
      addObserverForName:UIApplicationDidEnterBackgroundNotification
                  object:nil
                   queue:[NSOperationQueue mainQueue]
              usingBlock:^(NSNotification *_Nonnull note) {
                this->NotifyAppEnterBackground(nil);
              }];
}

LifecycleManager::~LifecycleManager() {
  [[NSNotificationCenter defaultCenter] removeObserver:scene_foreground_observer_];
  [[NSNotificationCenter defaultCenter] removeObserver:scene_background_observer_];
  [[NSNotificationCenter defaultCenter] removeObserver:app_foreground_observer_];
  [[NSNotificationCenter defaultCenter] removeObserver:app_background_observer_];
}

void LifecycleManager::AddListener(::AnimaXPlayer *player) {
  // Ensure listener addition runs on main thread to avoid threading issues.
  if (![NSThread isMainThread]) {
    dispatch_async(dispatch_get_main_queue(), ^{
      this->AddListener(player);
    });
    return;
  }
  if (!player) {
    return;
  }
  [listeners_ addObject:player];
}

void LifecycleManager::NotifyAppEnterForeground(UIScene *scene) {
  for (::AnimaXPlayer *player in listeners_) {
    if (!ShouldNotifyPlayer(player, scene)) {
      continue;
    }
    [player onAppEnterForeground];
  }
}

void LifecycleManager::NotifyAppEnterBackground(UIScene *scene) {
  for (::AnimaXPlayer *player in listeners_) {
    if (!ShouldNotifyPlayer(player, scene)) {
      continue;
    }
    [player onAppEnterBackground];
  }
}

}  // namespace animax
}  // namespace lynx
