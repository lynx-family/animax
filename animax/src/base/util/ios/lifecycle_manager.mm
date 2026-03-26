// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/ios/lifecycle_manager.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

LifecycleManager &LifecycleManager::Instance() {
  static base::NoDestructor<LifecycleManager> instance;
  return *instance;
}

LifecycleManager::LifecycleManager() {
  // Ensure the callback runs on main thread to avoid threading issues.
  foreground_observer_ = [[NSNotificationCenter defaultCenter]
      addObserverForName:UIApplicationWillEnterForegroundNotification
                  object:nil
                   queue:[NSOperationQueue mainQueue]
              usingBlock:^(NSNotification *_Nonnull note) {
                this->NotifyAppEnterForeground();
              }];
  background_observer_ = [[NSNotificationCenter defaultCenter]
      addObserverForName:UIApplicationDidEnterBackgroundNotification
                  object:nil
                   queue:[NSOperationQueue mainQueue]
              usingBlock:^(NSNotification *_Nonnull note) {
                this->NotifyAppEnterBackground();
              }];
}

LifecycleManager::~LifecycleManager() {
  [[NSNotificationCenter defaultCenter] removeObserver:foreground_observer_];
  [[NSNotificationCenter defaultCenter] removeObserver:background_observer_];
}

void LifecycleManager::AddListener(std::weak_ptr<AnimaXPlayer> weak_player) {
  // Ensure listener addition runs on main thread to avoid threading issues.
  if (![NSThread isMainThread]) {
    dispatch_async(dispatch_get_main_queue(), ^{
      this->AddListener(weak_player);
    });
    return;
  }
  listeners_.push_back(std::move(weak_player));
}

void LifecycleManager::NotifyAppEnterForeground() {
  decltype(listeners_) listeners;
  for (const auto &weak_listener : listeners_) {
    if (auto listener = weak_listener.lock()) {
      listener->OnAppEnterForeground();
      listeners.push_back(listener);
    }
  }
  if (listeners.size() < listeners_.size()) {
    listeners_.swap(listeners);
  }
}

void LifecycleManager::NotifyAppEnterBackground() {
  for (const auto &weak_listener : listeners_) {
    if (auto listener = weak_listener.lock()) {
      listener->OnAppEnterBackground();
    }
  }
}

}  // namespace animax
}  // namespace lynx
