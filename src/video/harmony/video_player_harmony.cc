// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/harmony/video_player_harmony.h"

#include <utility>

#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/player/animax_ability.h"
#include "src/player/harmony/animax_ability_harmony.h"
#include "src/render/texture_info_gl.h"
#include "src/video/harmony/video_codec_manager_factory_harmony.h"
#include "src/video/harmony/video_codec_manager_harmony.h"

namespace lynx {
namespace animax {
namespace {

constexpr const int32_t kSurfaceUpdateRetryCount = 2;

}  // namespace

VideoPlayerHarmony::VideoPlayerHarmony(std::shared_ptr<AnimaXAbility> ability) {
  if (ability) {
    auto harmony_ability =
        std::static_pointer_cast<AnimaXAbilityHarmony>(std::move(ability));
    if (harmony_ability) {
      auto timeout = harmony_ability->GetVideoFrameTimeout();
      if (timeout > 0) {
        ANIMAX_LOGI("Set user video decode timeout: " << timeout);
        user_timeout_ = std::chrono::milliseconds(timeout);
      }
    }
  }

  // Flip Y
  transform_ = {1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f,
                0.f, 0.f, 1.f, 0.f, 0.f, 1.f,  0.f, 1.f};
}

VideoPlayerHarmony::~VideoPlayerHarmony() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);

  codec_manager_.reset();

  // Destroy surface window
  if (native_window_) {
    OH_NativeWindow_DestroyNativeWindow(native_window_);
    native_window_ = nullptr;
  }

  // Detach and destroy native image
  if (native_image_) {
    if (IsFrameListenerEnabled()) {
      FrameListenerAdapter::GetInstance().Unregister(native_image_);
    }
    OH_NativeImage_DetachContext(native_image_);
  }

  // Delete texture after detach from context
  if (video_texture_ > 0) {
    glDeleteTextures(1, &video_texture_);
    video_texture_ = 0;
  }

  // Destroy native image in the end
  if (native_image_) {
    OH_NativeImage_Destroy(&native_image_);
    native_image_ = nullptr;
  }
}

std::unique_ptr<TextureInfo> VideoPlayerHarmony::UpdateTexture(
    const int32_t frame) {
  if (asset_ == nullptr || data_ == nullptr || codec_manager_ == nullptr) {
    ANIMAX_LOGI("asset, data or codec manager is nullptr");
    return nullptr;
  }

  auto texture_info = std::make_unique<TextureInfoGL>(
      video_texture_, asset_->GetVideoWidth(), asset_->GetVideoHeight(),
      GL_TEXTURE_EXTERNAL_OES);

  if (codec_manager_->RenderFrame(frame)) {
    UpdateSurfaceImage();
  }

  return texture_info;
}

void VideoPlayerHarmony::UpdateSurfaceImage() {
  if (IsFrameListenerEnabled() &&
      !FrameListenerAdapter::WaitForFrameAvailable(*frame_callback_context_,
                                                   user_timeout_)) {
    ANIMAX_LOGW("Timeout waiting for frame available callback");
    return;
  }

  // Retry for the case that the surface image is not updated.
  for (int32_t i = 0; i < kSurfaceUpdateRetryCount; i++) {
    auto ret = OH_NativeImage_UpdateSurfaceImage(native_image_);
    if (ret == 0) {
      break;
    }
    ANIMAX_LOGW("OH_NativeImage_UpdateSurfaceImage fail, ret: " << ret);
  }
}

const std::array<float, 16> &VideoPlayerHarmony::GetTransform() {
  return transform_;
}

void VideoPlayerHarmony::InitNativeWindow() {
  if (asset_ == nullptr || native_window_) {
    return;
  }

  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  lynx::animax::ScopedGLResetRestore s(GL_TEXTURE_BINDING_EXTERNAL_OES);
  glGenTextures(1, &video_texture_);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Create the native image, the video texture will not bind yet.
  native_image_ =
      OH_NativeImage_Create(video_texture_, GL_TEXTURE_EXTERNAL_OES);
  // Attach the texture to native image and call glBindTexture inside.
  auto ret = OH_NativeImage_AttachContext(native_image_, video_texture_);
  if (ret != 0) {
    ANIMAX_LOGI("OH_NativeImage_AttachContext fail, ret: " << ret);
  }
  // Hold the native window, used to update the image texture.
  native_window_ = OH_NativeImage_AcquireNativeWindow(native_image_);
  // Initialize the window's size by video's config.
  OH_NativeWindow_NativeWindowHandleOpt(native_window_, SET_BUFFER_GEOMETRY,
                                        asset_->GetVideoWidth(),
                                        asset_->GetVideoHeight());

  if (IsFrameListenerEnabled()) {
    FrameListenerAdapter::GetInstance().Register(native_image_,
                                                 frame_callback_context_);
  }
}

void VideoPlayerHarmony::AttachAsset(std::shared_ptr<VideoAsset> asset) {
  DCHECK(asset);
  asset_ = std::static_pointer_cast<VideoAssetHarmony>(asset);
  if (asset_) {
    data_ = &asset_->GetVideoData();
  }

  if (!data_) {
    ANIMAX_LOGE("AttachAsset fail, raw data is null.")
    return;
  }

  InitNativeWindow();
  InitCodecManager();
  // Clear GL errors from video decoding, as OH_NativeImage_UpdateSurfaceImage
  // checks GL error state and fails if any error exists.
  ANIMAX_LOGE("AttachAsset success, gl_error:" << glGetError());
}

void VideoPlayerHarmony::InitCodecManager() {
  if (data_ == nullptr || native_window_ == nullptr ||
      codec_manager_ != nullptr || !asset_->IsValid()) {
    ANIMAX_LOGE("InitCodecManager fail.")
    return;
  }
  codec_manager_ = VideoCodecManagerFactoryHarmony::GetInstance().Make(
      data_, native_window_);
  if (codec_manager_ == nullptr) {
    ANIMAX_LOGE("Init video codec manager fail.");
  }
}

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    std::shared_ptr<AnimaXAbility> ability) {
  return std::unique_ptr<VideoPlayer>(
      new VideoPlayerHarmony(std::move(ability)));
}

}  // namespace animax
}  // namespace lynx
