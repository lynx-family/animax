// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_
#define ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_

#import <AnimaX/AnimaXPlayer.h>

@class AnimaXPlayer;
@class UIScene;

namespace lynx {
namespace animax {
class LifecycleManager {
 public:
  static LifecycleManager &Instance();

  LifecycleManager();
  ~LifecycleManager();

  void AddListener(::AnimaXPlayer *player);

 private:
  void NotifyAppEnterForeground(UIScene *scene);
  void NotifyAppEnterBackground(UIScene *scene);
  id<NSObject> scene_foreground_observer_ = nil;
  id<NSObject> scene_background_observer_ = nil;
  id<NSObject> app_foreground_observer_ = nil;
  id<NSObject> app_background_observer_ = nil;
  NSHashTable<::AnimaXPlayer *> *listeners_ = nil;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_
