// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/web/vsync_monitor_web.h"

#include <emscripten/html5_webgl.h>

#include "src/base/log/log.h"

namespace lynx {
namespace animax {
extern "C" {
EM_BOOL OnVsyncFrame(double timestamp, void* user_data) {
  auto* self = static_cast<VSyncMonitorWeb*>(user_data);
  self->OnFrame(timestamp);
  return false;
}
}

VSyncMonitorWeb::VSyncMonitorWeb() : callback_(nullptr) {}

VSyncMonitorWeb::~VSyncMonitorWeb() { OnDestroy(); }

void VSyncMonitorWeb::RequestVSync(Callback callback) {
  callback_ = std::move(callback);
  emscripten_request_animation_frame(&OnVsyncFrame, this);
}

void VSyncMonitorWeb::OnDestroy() {
  is_destroyed_ = true;
  callback_ = nullptr;
}

EM_BOOL VSyncMonitorWeb::OnFrame(double timestamp) {
  // ANIMAX_LOGI("OnFrame:" << timestamp);
  auto callback = std::move(callback_);
  callback_ = nullptr;
  if (!is_destroyed_ && callback) {
    double timestamp_us = timestamp * 1000000.0;
    callback(timestamp_us);
  }
  return EM_FALSE;  // retur false to stop the loop
}

}  // namespace animax
}  // namespace lynx
