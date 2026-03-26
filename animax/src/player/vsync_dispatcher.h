// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_VSYNC_DISPATCHER_H_
#define ANIMAX_SRC_PLAYER_VSYNC_DISPATCHER_H_

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "include/player/vsync_monitor.h"

namespace lynx {
namespace animax {
// Shared requestAnimationFrame dispatcher.
// Batches multiple one-shot VSync requests into a single RAF per frame.
//
// Thread-safety:
// - This implementation is thread-safe for SubscribeOnce/Cancel/OnVSync.
// - User callbacks are always executed WITHOUT holding the internal mutex.
//
// Cancellation semantics (simplified):
// - Cancel(token) guarantees the callback won't run if it's still pending_.
// - If the callback has already been moved out by OnVSync() for the current
//   frame batch, it may still run.
class VSyncDispatcher {
 public:
  // Implemented with platform-specific derived class.
  static VSyncDispatcher& Instance();

  // Registers a one-shot callback to be fired on the next animation frame.
  // Returns a token that can be used to cancel before the frame arrives.
  uint64_t SubscribeOnce(VSyncMonitor::Callback cb);
  // Cancels a previously subscribed callback (no-op if token not found in
  // pending_).
  void Cancel(uint64_t token);
  // Called when a VSync event occurs (by platform-specific derived classes).
  void OnVSync(int64_t timestamp_ns);

 protected:
  VSyncDispatcher() = default;
  virtual ~VSyncDispatcher() = default;

 private:
  // Implemented by platform-specific derived classes.
  // Should arrange that OnVSync(timestamp_ns) is called when the next frame
  // fires.
  virtual void RequestVSync() = 0;

 private:
  mutable std::mutex mutex_;
  std::unordered_map<uint64_t, VSyncMonitor::Callback> pending_;
  uint64_t next_token_ = 0;
  bool pumping_ = false;
  int64_t last_log_ns_ = 0;
};
}  // namespace animax
}  // namespace lynx
#endif  // ANIMAX_SRC_PLAYER_VSYNC_DISPATCHER_H_
