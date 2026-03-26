// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRICS_LISTENER_H_
#define ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRICS_LISTENER_H_

#include <cstdint>

namespace lynx {
namespace animax {
class FPSListener {
 public:
  virtual ~FPSListener() = default;
  virtual void OnFps(float fps, uint32_t session_max_drop_value) = 0;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRICS_LISTENER_H_
