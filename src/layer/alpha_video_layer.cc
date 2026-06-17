// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/alpha_video_layer.h"

#include <cmath>

#include "src/base/thread/thread_assert.h"
#include "src/base/util/event_warning_checker.h"
#include "src/player/animax_ability.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/video_player.h"
#include "src/video/video_player_service.h"

namespace lynx {
namespace animax {

AlphaVideoLayer::AlphaVideoLayer(LayerModel& layer_model,
                                 CompositionModel& composition)
    : BaseLayer(layer_model, composition),
      paint_(std::make_unique<Paint>()),
      scale_(composition.GetScale()) {
  paint_->SetAntiAlias(true);

  auto id = layer_model_.GetRefId();
  std::shared_ptr<VideoAsset> video = composition.GetVideos()[id];
  if (video) {
    video_asset_ = video;
  } else {
    ANIMAX_LOGE("video " << id << " is null");
  }
}

void AlphaVideoLayer::Init() {
  BaseLayer::Init();
  AttachAssetOnce();
}

void AlphaVideoLayer::AttachAssetOnce() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  auto context = weak_context_.lock();
  if (!context) {
    return;
  }
  auto ability = context->weak_ability.lock();
  auto player = context->weak_player.lock();
  if (!player || !video_asset_ || has_attach_ || !video_asset_->IsValid()) {
    return;
  }

  if (ability) {
    enable_downsample_ = ability->IsDownsampleVideoEnabled();
  }

  ANIMAX_LOGI("AttachAssetOnce on video: " << video_asset_->Model().id);
  auto provider =
      VideoPlayerService::GetInstance().GetProvider(ability, video_asset_);
  if (!provider) {
    return;
  }
  video_shader_ = provider->CreateVideoShader(ability);

  DCHECK(video_shader_);
  int32_t video_width = video_asset_->GetVideoWidth();
  int32_t video_height = video_asset_->GetVideoHeight();
  EventWarningChecker::CheckIllegalAlphaVideoSize(
      video_width, video_height,
      [this](EventWarning warning, const std::string& warning_msg) {
        if (auto listener = weak_listener_.lock()) {
          listener->OnLayerWarning(warning, warning_msg);
        }
      });

  std::array<float, 4> rgb_frame{
      float(video_asset_->GetRgbX()) / video_width,
      float(video_asset_->GetRgbY()) / video_height,
      float(video_asset_->GetRgbWidth()) / video_width,
      float(video_asset_->GetRgbHeight()) / video_height};
  std::array<float, 4> a_frame{
      float(video_asset_->GetAlphaX()) / video_width,
      float(video_asset_->GetAlphaY()) / video_height,
      float(video_asset_->GetAlphaWidth()) / video_width,
      float(video_asset_->GetAlphaHeight()) / video_height};
  int32_t composite_texture_width = video_asset_->GetRgbWidth();
  int32_t composite_texture_height = video_asset_->GetRgbHeight();

  bool is_downsample_size_valid =
      enable_downsample_ && downsample_width_ > 0 && downsample_height_ > 0;
  if (is_downsample_size_valid) {
    composite_texture_width = downsample_width_;
    composite_texture_height = downsample_height_;
  }

  // VideoShader / VideoPlayer ensure any backend-specific rendering context
  // (e.g. the Vulkan path's offscreen EGL context) themselves around their GL
  // resource creation, so no scope is needed here.
  video_shader_->Init(composite_texture_width, composite_texture_height,
                      rgb_frame, a_frame);
  if (!video_shader_->Valid()) {
    ANIMAX_LOGE("OnLayerError, code: "
                << static_cast<int32_t>(EventError::kVideoPlayerError)
                << ", message: video shader init error");
    if (auto listener = weak_listener_.lock()) {
      listener->OnLayerError(EventError::kVideoPlayerError,
                             "video shader init error");
    }
  }

  video_player_ = provider->CreateVideoPlayer(std::move(ability));
  DCHECK(video_player_);
  video_player_->SetListener(weak_listener_);
  video_player_->AttachAsset(video_asset_);

  if (is_downsample_size_valid) {
    video_player_->UpdateOutputFrameSize(downsample_width_, downsample_height_);
  }

  has_attach_ = true;
}

void AlphaVideoLayer::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                                bool apply_parent) {
  BaseLayer::GetBounds(out_bounds, parent_matrix, apply_parent);
  if (video_asset_) {
    out_bounds.Set(0, 0, video_asset_->GetRgbWidth() * scale_,
                   video_asset_->GetRgbHeight() * scale_);
    bounds_matrix_->MapRect(out_bounds);
  }
}

void AlphaVideoLayer::UpdateDownsampleSize(int32_t canvas_width,
                                           int32_t canvas_height) {
  if (canvas_width <= 0 || canvas_height <= 0 || !video_asset_) {
    return;
  }

  int32_t video_width = video_asset_->GetRgbWidth();
  int32_t video_height = video_asset_->GetRgbHeight();
  int32_t downsample_width = 0;
  int32_t downsample_height = 0;
  if (canvas_width < video_width && canvas_height < video_height) {
    downsample_width = canvas_width;
    downsample_height = canvas_height;
  }

  if (downsample_width != downsample_width_ ||
      downsample_height != downsample_height_) {
    downsample_width_ = downsample_width;
    downsample_height_ = downsample_height;
    has_attach_ = false;
    image_.reset();
    video_shader_.reset();
    video_player_.reset();

    ANIMAX_LOGI("UpdateDownsampleSize: " << downsample_width_ << "*"
                                         << downsample_height_);
  }
}

void AlphaVideoLayer::DrawLayer(Canvas& canvas, Matrix& parent_matrix,
                                int32_t parent_alpha) {
  auto image = UpdateCompositeImage(canvas, canvas.GetRealContext());
  if (image == nullptr) {
    return;
  }

  paint_->SetAlpha(parent_alpha);

  canvas.Save();
  canvas.Concat(parent_matrix);

  auto dst_width = static_cast<int>(video_asset_->GetRgbWidth() * scale_);
  auto dst_height = static_cast<int>(video_asset_->GetRgbHeight() * scale_);
  if (enable_downsample_) {
    if (canvas_width_ != canvas.GetWidth() ||
        canvas_height_ != canvas.GetHeight()) {
      canvas_width_ = canvas.GetWidth();
      canvas_height_ = canvas.GetHeight();

      auto current_matrix = canvas.GetMatrix();
      RectF video_canvas_rect(0, 0, dst_width, dst_height);
      current_matrix->MapRect(video_canvas_rect);

      UpdateDownsampleSize(video_canvas_rect.GetWidth(),
                           video_canvas_rect.GetHeight());
      image = UpdateCompositeImage(canvas, canvas.GetRealContext());
      if (image == nullptr) {
        return;
      }
    }
  }

  src_.Set(0, 0, static_cast<int>(image->GetWidth()),
           static_cast<int>(image->GetHeight()));
  dst_.Set(0, 0, dst_width, dst_height);

  canvas.DrawImageRect(*image, src_, dst_, *paint_);
  canvas.Restore();
}

Image* AlphaVideoLayer::UpdateCompositeImage(Canvas& canvas,
                                             RealContext* real_context) {
  AttachAssetOnce();
  auto composite_image = GetCompositeImage(real_context);
  if (!composite_image || !video_shader_ || !video_player_) {
    return nullptr;
  }

  const int32_t frame = GetCurrentFrame();
  // BeginFrame returns a scope that holds any backend rendering context current
  // for the GL work below and commits the frame on destruction. The scope is
  // destroyed as this function returns, after the texture has been composed, so
  // GL completion is bridged to the active backend surface before the caller
  // draws the composite image.
  auto frame_scope = video_shader_->BeginFrame(&canvas, real_context);
  if (frame_scope && frame_scope->Ready()) {
    // update video texture
    auto video_texture = video_player_->UpdateTexture(frame);
    // compose video texture into composite texture
    if (video_texture) {
      video_shader_->Draw(std::move(video_texture),
                          video_player_->GetTransform());
    }
  }

  return composite_image;
}

int32_t AlphaVideoLayer::GetCurrentFrame() {
  auto& keyframe = in_out_animation_->GetCurrentKeyframe();
  float progress_into_frame =
      in_out_animation_->GetProgress() - keyframe.GetStartProgress();
  int32_t frame = std::round(progress_into_frame * keyframe.GetDurationMs() *
                             video_asset_->GetFrameRate() / 1000);
  const int32_t frame_count = video_asset_->GetFrameCount();
  if (frame >= frame_count) {
    frame = frame_count - 1;
  } else if (frame < 0) {
    frame = 0;
  }
  return frame;
}

Image* AlphaVideoLayer::GetCompositeImage(RealContext* real_context) {
  if (!video_asset_ || !video_asset_->IsValid() || !video_shader_) {
    return nullptr;
  }
  if (!image_) {
    image_ = video_shader_->GetOutputImage(real_context);
  }
  return image_.get();
}

}  // namespace animax
}  // namespace lynx
