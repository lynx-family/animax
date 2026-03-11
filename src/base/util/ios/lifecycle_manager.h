// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_
#define ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_

#include <memory>
#include <vector>

#include "include/player/animax_player.h"

namespace lynx {
namespace animax {
class LifecycleManager {
 public:
  static LifecycleManager &Instance();

  LifecycleManager();
  ~LifecycleManager();

  void AddListener(std::weak_ptr<AnimaXPlayer> weak_player);

 private:
  void NotifyAppEnterForeground();
  void NotifyAppEnterBackground();
  id<NSObject> foreground_observer_ = nil;
  id<NSObject> background_observer_ = nil;
  std::vector<std::weak_ptr<AnimaXPlayer>> listeners_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_
