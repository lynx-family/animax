// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_IOS_VSYNC_MONITOR_IOS_H_
#define ANIMAX_SRC_PLAYER_IOS_VSYNC_MONITOR_IOS_H_

#include <memory>

#include "include/player/vsync_monitor.h"

@class AnimaXVSyncMonitor;
namespace lynx {
namespace animax {
class VSyncMonitorIOS final : public VSyncMonitor {
 public:
  VSyncMonitorIOS();
  ~VSyncMonitorIOS() final;
  void RequestVSync(Callback callback) final;

 private:
  template <typename Block>
  void ExecuteOnMainThread(Block block);

  AnimaXVSyncMonitor* monitor_ = nil;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_IOS_VSYNC_MONITOR_IOS_H_
