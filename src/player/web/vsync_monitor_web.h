// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WEB_VSYNC_MONITOR_WEB_H_
#define ANIMAX_SRC_PLAYER_WEB_VSYNC_MONITOR_WEB_H_

#include <memory>

#include "include/player/vsync_monitor.h"
#include "src/player/web/frame_rate_limiter_web.h"

namespace lynx {
namespace animax {

class VSyncMonitorWeb final : public VSyncMonitor {
 public:
  void SetMaxFrameRate(double max_frame_rate);
  void RequestVSync(Callback callback) override;

 private:
  using CallbackHolder = std::shared_ptr<Callback>;

  void RequestNextVSync(CallbackHolder callback);

  FrameRateLimiterWeb frame_rate_limiter_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WEB_VSYNC_MONITOR_WEB_H_
