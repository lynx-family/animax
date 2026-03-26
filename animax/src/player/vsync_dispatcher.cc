// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/vsync_dispatcher.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {
uint64_t VSyncDispatcher::SubscribeOnce(VSyncMonitor::Callback cb) {
  bool need_request = false;
  uint64_t token = 0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    token = ++next_token_;
    pending_.emplace(token, std::move(cb));
    if (!pumping_) {
      pumping_ = true;
      need_request = true;
    }
  }
  if (need_request) {
    RequestVSync();
  }
  return token;
}

void VSyncDispatcher::Cancel(uint64_t token) {
  std::lock_guard<std::mutex> lock(mutex_);
  pending_.erase(token);  // no-op if not found
}

void VSyncDispatcher::OnVSync(int64_t timestamp_ns) {
  std::unordered_map<uint64_t, VSyncMonitor::Callback> callbacks;
  bool do_log = false;
  size_t batch_size_snapshot = 0;
  uint64_t next_token_snapshot = 0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pumping_ = false;
    if (pending_.empty()) {
      return;
    }
    callbacks.swap(pending_);
    batch_size_snapshot = callbacks.size();
    static constexpr int64_t kLogIntervalNs = 60LL * 1000 * 1000 * 1000;
    if (last_log_ns_ == 0) {
      last_log_ns_ = timestamp_ns;
    } else if (timestamp_ns - last_log_ns_ >= kLogIntervalNs) {
      do_log = true;
      last_log_ns_ = timestamp_ns;
      next_token_snapshot = next_token_;
    }
  }  // unlock
  for (auto& [token, cb] : callbacks) {
    if (cb) {
      cb(timestamp_ns);
    }
  }
  if (do_log) {
    ANIMAX_LOGI("RAF tick alive."
                << " batch_size=" << batch_size_snapshot
                << " next_token=" << next_token_snapshot);
  }
}

}  // namespace animax
}  // namespace lynx
