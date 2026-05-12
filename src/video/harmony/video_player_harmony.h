// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_HARMONY_VIDEO_PLAYER_HARMONY_H_
#define ANIMAX_SRC_VIDEO_HARMONY_VIDEO_PLAYER_HARMONY_H_

#include <native_image/native_image.h>
#include <native_window/external_window.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "src/base/util/harmony/frame_listener_adapter.h"
#include "src/video/harmony/video_asset_harmony.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {

class VideoCodecManagerHarmony;

class VideoPlayerHarmony : public VideoPlayer {
 public:
  explicit VideoPlayerHarmony(const AnimaXAbility *ability_ptr);
  ~VideoPlayerHarmony() override;

  std::unique_ptr<TextureInfo> UpdateTexture(const int32_t frame) override;

  void UpdateSurfaceImage();

  const std::array<float, 16> &GetTransform() override;

  void AttachAsset(std::shared_ptr<VideoAsset> asset) override;

  void NotifyErrorEvent(const std::string &err_msg);

 private:
  // Create the native window by creating texture id and native image.
  void InitNativeWindow();

  // Create the platform codec manager. The manager decides sync or async codec.
  void InitCodecManager();

  // Check if frame listener is enabled (user has set a timeout).
  bool IsFrameListenerEnabled() const { return user_timeout_.count() > 0; }

  uint32_t video_texture_ = 0;
  std::array<float, 16> transform_{};

  std::shared_ptr<VideoAssetHarmony> asset_;
  VideoData *data_ = nullptr;

  OH_NativeImage *native_image_ = nullptr;
  OHNativeWindow *native_window_ = nullptr;
  std::shared_ptr<VideoCodecManagerHarmony> codec_manager_;

  std::shared_ptr<FrameCallbackContext> frame_callback_context_ =
      std::make_shared<FrameCallbackContext>();

  // User-defined timeout. Zero means use default behavior.
  std::chrono::milliseconds user_timeout_{0};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_HARMONY_VIDEO_PLAYER_HARMONY_H_
