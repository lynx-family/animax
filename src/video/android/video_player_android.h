// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_ANDROID_VIDEO_PLAYER_ANDROID_H_
#define ANIMAX_SRC_VIDEO_ANDROID_VIDEO_PLAYER_ANDROID_H_

#include "base/include/platform/android/scoped_java_ref.h"
#include "src/player/animax_ability.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {
class VideoAssetAndroid;
class VideoPlayerAndroid : public VideoPlayer {
 public:
  explicit VideoPlayerAndroid(std::shared_ptr<AnimaXAbility> ability);
  ~VideoPlayerAndroid() override;

  std::unique_ptr<TextureInfo> UpdateTexture(const int32_t frame) override;
  const std::array<float, 16> &GetTransform() override;
  void AttachAsset(std::shared_ptr<VideoAsset> asset) override;
  void UpdateOutputFrameSize(const int32_t width,
                             const int32_t height) override;

  void NotifyErrorEvent(const std::string &err_msg);

 private:
  void CreateVideoTexture();

  lynx::base::android::ScopedGlobalJavaRef<jobject> player_;
  std::shared_ptr<VideoAssetAndroid> asset_;

  uint32_t video_texture_ = 0;
  std::array<float, 16> transform_{};
  int32_t current_frame_ = -1;
  int32_t output_width_ = 0;
  int32_t output_height_ = 0;
  bool enable_downsample_ = false;
  ContextBackend backend_ = ContextBackend::kOpenGL;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_ANDROID_VIDEO_PLAYER_ANDROID_H_
