// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_VSYNC_MONITOR_ANDROID_H_
#define ANIMAX_SRC_PLAYER_ANDROID_VSYNC_MONITOR_ANDROID_H_

#include <jni.h>

#include <memory>

#include "include/player/vsync_monitor.h"

namespace lynx {
namespace animax {

class VSyncMonitorAndroid final : public VSyncMonitor {
 public:
  ~VSyncMonitorAndroid() final = default;
  void RequestVSync(VSyncMonitor::Callback callback) final;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_VSYNC_MONITOR_ANDROID_H_
