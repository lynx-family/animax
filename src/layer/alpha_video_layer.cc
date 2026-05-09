// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/alpha_video_layer.h"

#include <cmath>

#include "src/base/thread/thread_assert.h"
#include "src/base/util/event_warning_checker.h"
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
  ANIMAX_LOGI("AttachAssetOnce on video: " << video_asset_->Model().id);
  auto provider = VideoPlayerService::GetInstance().GetProvider(ability.get(),
                                                                video_asset_);
  if (!provider) {
    return;
  }
  video_shader_ = provider->CreateVideoShader(ability.get());

  DCHECK(video_shader_);
  int32_t video_width = video_asset_->GetVideoWidth();
  int32_t video_height = video_asset_->GetVideoHeight();
  EventWarningChecker::CheckIllegalAlphaVideoSize(
      video_width, video_height,
      [this](EventWarning warning, const std::string& warning_msg) {
        OnVideoPlayerWarning(warning, warning_msg);
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
  video_shader_->Init(composite_texture_width, composite_texture_height,
                      rgb_frame, a_frame);
  if (!video_shader_->Valid()) {
    VideoPlayerListener::OnVideoPlayerError("video shader init error");
  }

  video_player_ = provider->CreateVideoPlayer(ability.get());
  DCHECK(video_player_);
  video_player_->SetListener(this);
  video_player_->AttachAsset(video_asset_);

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

void AlphaVideoLayer::DrawLayer(Canvas& canvas, Matrix& parent_matrix,
                                int32_t parent_alpha) {
  auto image = UpdateCompositeImage(canvas.GetRealContext());
  if (image == nullptr) {
    return;
  }

  paint_->SetAlpha(parent_alpha);

  canvas.Save();
  canvas.Concat(parent_matrix);
  src_.Set(0, 0, static_cast<int>(image->GetWidth()),
           static_cast<int>(image->GetHeight()));
  dst_.Set(0, 0, static_cast<int>(video_asset_->GetRgbWidth() * scale_),
           static_cast<int>(video_asset_->GetRgbHeight() * scale_));

  canvas.DrawImageRect(*image, src_, dst_, *paint_);
  canvas.Restore();
}

Image* AlphaVideoLayer::UpdateCompositeImage(RealContext* real_context) {
  AttachAssetOnce();
  auto composite_image = GetCompositeImage(real_context);
  if (!composite_image || !video_shader_ || !video_player_) {
    return nullptr;
  }

  const int32_t frame = GetCurrentFrame();
  // update video texture
  auto video_texture = video_player_->UpdateTexture(frame);
  // compose video texture into composite texture
  if (video_texture) {
    video_shader_->Draw(std::move(video_texture),
                        video_player_->GetTransform());
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

void AlphaVideoLayer::OnVideoPlayerError(EventError error,
                                         const std::string& err_msg) {
  ANIMAX_LOGE("OnVideoPlayerError, code:" << static_cast<int32_t>(error)
                                          << ", message: " << err_msg);
  if (auto listener = weak_listener_.lock()) {
    listener->OnLayerError(error, err_msg);
  }
}

void AlphaVideoLayer::OnVideoPlayerWarning(lynx::animax::EventWarning warning,
                                           const std::string& warning_msg) {
  if (auto listener = weak_listener_.lock()) {
    listener->OnLayerWarning(warning, warning_msg);
  }
}

}  // namespace animax
}  // namespace lynx
