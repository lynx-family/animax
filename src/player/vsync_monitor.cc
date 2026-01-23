// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/player/vsync_monitor.h"

#include "base/include/fml/message_loop.h"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"
#include "src/player/vsync_dispatcher.h"

namespace lynx {
namespace animax {

VSyncMonitor::~VSyncMonitor() {
  // Atomically take the token and clear it to 0.
  uint64_t t = token_.exchange(0, std::memory_order_acq_rel);
  if (t != 0) {
    VSyncDispatcher::Instance().Cancel(t);
  }
}

void VSyncMonitor::RequestVSync(Callback callback) {
  auto &dispatcher = VSyncDispatcher::Instance();
  uint64_t t = dispatcher.SubscribeOnce(std::move(callback));
  // If multiple RequestVSync calls are possible, cancel the old one.
  uint64_t old = token_.exchange(t, std::memory_order_acq_rel);
  if (old != 0) {
    dispatcher.Cancel(old);
  }
}

void VSyncMonitor::AsyncRequestVSync(Callback callback) {
  if (callback_) {
    // If there is a pending callback_ that has not been invoked yet,
    // we will keep the pending callback_ and discard the callback
    // for this specific AsyncRequestVSync operation.
    ANIMAX_LOGV(
        "AsyncRequestVSync detected a pending callback_ that has not "
        "been invoked yet. The callback for this specific "
        "AsyncRequestVSync operation will be discarded.");
    return;
  }
  callback_ = std::move(callback);
  // Call from AnimaX_Main thread
  RequestVSync([weak_self = weak_from_this()](int64_t timestamp) {
    auto self = weak_self.lock();
    if (!self) {
      return;
    }

    // Call from UI thread to AnimaX_Main thread to invoke the callback.
    GetAnimaXMainThread()->PostTask(
        [weak_self = self->weak_from_this(), timestamp]() {
          auto self = weak_self.lock();
          if (!self) {
            return;
          }
          // Clean up the callback_ once it is invoked.
          auto callback = std::move(self->callback_);
          callback(timestamp);
        });
  });
}

}  // namespace animax
}  // namespace lynx
