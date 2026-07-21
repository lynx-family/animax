// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_
#define ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_

#import <Foundation/Foundation.h>

#include <memory>
#include <vector>

#include "include/player/animax_player.h"

@protocol AnimaXApplicationLifecycleListener <NSObject>
- (void)onAnimaXApplicationDidBecomeActive;
@end

namespace lynx {
namespace animax {
class LifecycleManager {
 public:
  static LifecycleManager &Instance();

  LifecycleManager();
  ~LifecycleManager();

  void AddListener(std::weak_ptr<AnimaXPlayer> weak_player);
  void AddApplicationLifecycleListener(id<AnimaXApplicationLifecycleListener> listener);
  bool IsApplicationActive() const;

 private:
  void NotifyAppEnterForeground();
  void NotifyAppEnterBackground();
  void NotifyApplicationDidBecomeActive();
  id<NSObject> foreground_observer_ = nil;
  id<NSObject> background_observer_ = nil;
  id<NSObject> application_active_observer_ = nil;
  NSHashTable<id<AnimaXApplicationLifecycleListener>> *application_lifecycle_listeners_ = nil;
  std::vector<std::weak_ptr<AnimaXPlayer>> listeners_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_IOS_LIFECYCLE_MANAGER_H_
