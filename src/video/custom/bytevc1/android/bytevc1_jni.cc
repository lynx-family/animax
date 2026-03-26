// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <jni.h>

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/video/custom/bytevc1/android/video_player_provider_bytevc1.h"
#include "src/video/video_player_service.h"

static constexpr const char *kAnimaXByteVc1ProviderKey = "bytevc1";

extern "C" __attribute__((visibility("default"))) int32_t JNI_OnLoad(JavaVM *vm,
                                                                     void *) {
  auto provider = std::make_shared<lynx::animax::VideoPlayerProviderByteVc1>();
  auto ret = lynx::animax::RegisterVideoPlayerProvider(
      kAnimaXByteVc1ProviderKey, provider);

  if (ret) {
    ANIMAX_LOGI("animax bytevc1 loaded, register bytevc1 success.");
  } else {
    ANIMAX_LOGE("animax bytevc1 failed to register provider.");
  }

  return JNI_VERSION_1_6;
}
