// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/web/vsync_monitor_web.h"

#include <utility>

namespace lynx {
namespace animax {

void VSyncMonitorWeb::SetMaxFrameRate(double max_frame_rate) {
  frame_rate_limiter_.SetMaxFrameRate(max_frame_rate);
}

void VSyncMonitorWeb::RequestVSync(Callback callback) {
  RequestNextVSync(std::make_shared<Callback>(std::move(callback)));
}

void VSyncMonitorWeb::RequestNextVSync(CallbackHolder callback) {
  VSyncMonitor::RequestVSync(
      [weak_self = weak_from_this(),
       callback = std::move(callback)](int64_t timestamp_ns) mutable {
        auto self = std::static_pointer_cast<VSyncMonitorWeb>(weak_self.lock());
        if (!self) {
          return;
        }
        if (!self->frame_rate_limiter_.ShouldDispatch(timestamp_ns)) {
          self->RequestNextVSync(std::move(callback));
          return;
        }
        auto pending_callback = std::move(*callback);
        if (pending_callback) {
          pending_callback(timestamp_ns);
        }
      });
}

}  // namespace animax
}  // namespace lynx
