// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PLAYER_VSYNC_MONITOR_H_
#define ANIMAX_INCLUDE_PLAYER_VSYNC_MONITOR_H_

#include <memory>

#include "base/include/closure.h"
#include "include/base/macros.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT VSyncMonitor
    : public std::enable_shared_from_this<VSyncMonitor> {
 public:
  virtual ~VSyncMonitor() = default;

  // During a single VSync cycle, only the first callback passed to
  // AsyncRequestVSync will be invoked upon the VSync event. Any subsequent
  // callbacks passed to AsyncRequestVSync within the same VSync cycle will be
  // ignored until the first callback has been executed.
  using Callback = base::MoveOnlyClosure<void, int64_t>;
  virtual void AsyncRequestVSync(Callback callback);

  // Pure virtual function defined here, leaving the implementation to be
  // provided by platform-specific derived classes.
  virtual void RequestVSync(Callback callback) = 0;

 private:
  Callback callback_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PLAYER_VSYNC_MONITOR_H_
