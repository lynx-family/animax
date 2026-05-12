// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_HARMONY_VIDEO_CODEC_MANAGER_FACTORY_HARMONY_H_
#define ANIMAX_SRC_VIDEO_HARMONY_VIDEO_CODEC_MANAGER_FACTORY_HARMONY_H_

#include <native_window/external_window.h>

#include <memory>

#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

struct VideoData;
class VideoCodecManagerHarmony;

std::shared_ptr<VideoCodecManagerHarmony> MakeVideoCodecManagerAsyncHarmony(
    VideoData *data, OHNativeWindow *native_window);
std::shared_ptr<VideoCodecManagerHarmony> MakeVideoCodecManagerSyncHarmony(
    VideoData *data, OHNativeWindow *native_window);

class VideoCodecManagerFactoryHarmony {
 public:
  static VideoCodecManagerFactoryHarmony &GetInstance();

  VideoCodecManagerFactoryHarmony(const VideoCodecManagerFactoryHarmony &) =
      delete;
  VideoCodecManagerFactoryHarmony &operator=(
      const VideoCodecManagerFactoryHarmony &) = delete;

  void SetEnableSyncCodec(bool enable);
  std::shared_ptr<VideoCodecManagerHarmony> Make(VideoData *data,
                                                 OHNativeWindow *native_window);

 private:
  friend class lynx::base::NoDestructor<VideoCodecManagerFactoryHarmony>;
  VideoCodecManagerFactoryHarmony() = default;

  bool enable_sync_codec_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_HARMONY_VIDEO_CODEC_MANAGER_FACTORY_HARMONY_H_
