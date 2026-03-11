// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WEB_VSYNC_MONITOR_WEB_H_
#define ANIMAX_SRC_PLAYER_WEB_VSYNC_MONITOR_WEB_H_

#include <emscripten/em_types.h>

#include "include/player/vsync_monitor.h"

namespace lynx {
namespace animax {

class VSyncMonitorWeb final : public VSyncMonitor {
 public:
  VSyncMonitorWeb();
  ~VSyncMonitorWeb() override;

  void RequestVSync(Callback callback) override;
  void OnDestroy();

  EM_BOOL OnFrame(double timestamp);

 private:
  Callback callback_;
  bool is_destroyed_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WEB_VSYNC_MONITOR_WEB_H_
