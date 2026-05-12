// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/harmony/video_codec_manager_factory_harmony.h"

#include "src/base/log/log.h"
#include "src/video/harmony/video_codec_manager_harmony.h"

namespace lynx {
namespace animax {

VideoCodecManagerFactoryHarmony &
VideoCodecManagerFactoryHarmony::GetInstance() {
  static lynx::base::NoDestructor<VideoCodecManagerFactoryHarmony> factory;
  return *factory;
}

void VideoCodecManagerFactoryHarmony::SetEnableSyncCodec(bool enable) {
  enable_sync_codec_ = enable;
}

std::shared_ptr<VideoCodecManagerHarmony> VideoCodecManagerFactoryHarmony::Make(
    VideoData *data, OHNativeWindow *native_window) {
  if (enable_sync_codec_) {
    auto sync_manager = MakeVideoCodecManagerSyncHarmony(data, native_window);
    if (sync_manager && sync_manager->Init()) {
      return sync_manager;
    } else {
      ANIMAX_LOGW("MakeVideoCodecManagerSyncHarmony fail, fallback to async");
    }
  }

  auto async_manager = MakeVideoCodecManagerAsyncHarmony(data, native_window);
  if (async_manager && async_manager->Init()) {
    return async_manager;
  }

  return nullptr;
}

}  // namespace animax
}  // namespace lynx
