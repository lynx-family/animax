// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <emscripten/html5_webgl.h>

#include "base/include/no_destructor.h"
#include "src/player/vsync_dispatcher.h"

namespace lynx {
namespace animax {

class VsyncDispatcherWeb final : public VSyncDispatcher {
 public:
  void RequestVSync() override {
    emscripten_request_animation_frame(&VsyncDispatcherWeb::RAFCallback, this);
  }
  static EM_BOOL RAFCallback(double timestamp_ms, void* user_data) {
    static_cast<VSyncDispatcher*>(user_data)->OnVSync(timestamp_ms * 1e+6);
    return EM_FALSE;
  }
};

VSyncDispatcher& VSyncDispatcher::Instance() {
  static base::NoDestructor<VsyncDispatcherWeb> inst;
  return *inst;
}

}  // namespace animax
}  // namespace lynx
