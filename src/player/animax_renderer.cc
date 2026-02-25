// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_renderer.h"

#include <algorithm>
#include <utility>

#include "include/player/animax_player.h"
#include "src/base/log/log.h"
#include "src/base/monitor/animax_metric_names.h"
#include "src/base/monitor/animax_metrics_manager.h"
#include "src/base/monitor/trace_event.h"
#include "src/base/thread/thread_assert.h"
#include "src/base/util/event_warning_checker.h"
#include "src/layer/composition_layer.h"
#include "src/parser/layer_parser.h"
#include "src/player/animax_main_controller.h"
#include "src/player/animax_playback_event_handler.h"
#include "src/property/animax_property_updater.h"
#include "src/render/matrix.h"
#include "src/render/surface.h"

namespace lynx {
namespace animax {

AnimaXRenderer::AnimaXRenderer(
    std::weak_ptr<AnimaXPlaybackEventHandler> weak_playback_handler)
    : weak_playback_handler_(std::move(weak_playback_handler)) {
  property_updater_ = std::make_unique<AnimaXPropertyUpdater>(*this);
  gpu_thread_recorder_.SetFPSListener(weak_playback_handler_);
}

AnimaXRenderer::~AnimaXRenderer() = default;

void AnimaXRenderer::Init(std::shared_ptr<AnimaXPlayerContext> context) {
  DCHECK(context);
  weak_controller_actor_ = context->weak_main_controller;
  weak_context_ = std::move(context);
}

void AnimaXRenderer::UpdateSurfaceInternal(
    std::unique_ptr<AnimaXSurface> surface) {
  auto controller_actor = weak_controller_actor_.lock();
  if (!surface || !surface->Valid()) {
    if (controller_actor) {
      controller_actor->Act(
          [error = EventError::kSurfaceInitError,
           message = "Failed to update AnimaXSurface."](auto& controller) {
            controller->NotifyError(error, message);
          });
    }
    return;
  }

  width_ = surface->Width();
  height_ = surface->Height();
  surface_ = std::move(surface);
  ANIMAX_LOGI("AnimaXRenderer UpdateSurfaceInternal"
              << ", surface: " << surface_);

  if (!has_surface_before_) {
    // If this is the first surface assigned to the renderer, initiate
    // animation.
    has_surface_before_ = true;
    StartAnimationIfNeeded();
  } else {
    // When the surface changes, seek to the current frame, ensure it is
    // rendered on the new surface.
    if (controller_actor) {
      controller_actor->Act(
          [](auto& controller) { controller->SeekToCurrentFrame(); });
    }
  }
}

std::unique_ptr<AnimaXSurface> AnimaXRenderer::ReleaseSurface() {
  ANIMAX_LOGI("ReleaseSurface"
              << ", surface: " << surface_);
  return std::move(surface_);
}

void AnimaXRenderer::CreateSurface(SurfaceCreationFactory creation_factory) {
  ReleaseSurface();
  ANIMAX_TRACE_EVENT_BEGIN(kCreateSurface);
  auto surface = creation_factory();
  ANIMAX_TRACE_EVENT_END();
  if (surface) {
    UpdateSurfaceInternal(std::move(surface));
  } else {
    ANIMAX_LOGI("CreateSurface failed, surface is null");
  }
}

void AnimaXRenderer::UpdateSurface(SurfaceUpdateFactory update_factory) {
  auto old_surface = ReleaseSurface();
  auto old_surface_valid = old_surface && old_surface->Valid();
  std::unique_ptr<AnimaXSurface> new_surface;
  if (old_surface_valid) {
    new_surface = update_factory(std::move(old_surface));
  } else {
    // If the old surface is not valid, means the renderer has not been
    // initialized yet, we need to trace it as the creation of a new surface.
    ANIMAX_TRACE_EVENT_BEGIN(kCreateSurface);
    new_surface = update_factory(nullptr);
    ANIMAX_TRACE_EVENT_END();
  }
  if (new_surface) {
    UpdateSurfaceInternal(std::move(new_surface));
  } else {
    ANIMAX_LOGI("UpdateSurface failed, new_surface is null");
  }
}

void AnimaXRenderer::UpdateComposition(
    int32_t src_index, std::shared_ptr<CompositionModel> model) {
  model_ = std::move(model);
  StartAnimationIfNeeded();
}

void AnimaXRenderer::StartAnimationIfNeeded() {
  if (!surface_ || !model_) {
    return;
  }
  if (surface_->Type() == AnimaXBackend::kSoftware &&
      !model_->GetVideos().empty()) {
    // Need to clear it because software didn't ensure the gl environment for
    // alpha video, which causes null pointer error.
    // TODO(lixianruo.cyrus): Support to render video layer when backend is
    // software.
    auto controller_actor = weak_controller_actor_.lock();
    if (controller_actor) {
      controller_actor->Act([](auto& controller) {
        controller->NotifyError(
            EventError::kVideoPlayerSoftwareRenderingNotSupported,
            "Video layers can't be rendered in software.");
      });
    }
    model_->GetVideos().clear();
    return;
  }
  StartAnimation();
}

void AnimaXRenderer::StartAnimation() {
  audio_controllers_.clear();
  has_rendered_first_frame_ = false;
  auto& bounds = model_->GetBounds();
  model_width_ = bounds.GetWidth();
  model_height_ = bounds.GetHeight();

  layer_model_ = LayerParser::Parse(*model_);
  layer_ = std::unique_ptr<CompositionLayer>(
      new CompositionLayer(*layer_model_, *model_));
  layer_->SetLayerModels(model_->GetLayers());
  auto playback_handler = weak_playback_handler_.lock();
  if (playback_handler) {
    layer_->SetEventListener(playback_handler.get());
  }
  layer_->SetPlayerContext(weak_context_);
  layer_->Init();

  auto controller_actor = weak_controller_actor_.lock();
  if (!controller_actor) {
    return;
  }
  auto meta = CompositionModelMeta{
      .start_frame = model_->GetStartFrame(),
      .end_frame = model_->GetEndFrame(),
      .duration = model_->GetDuration(),
      .frame_rate = model_->GetFrameRate(),
      .has_video_layer = !model_->GetVideos().empty(),
      .image_count = model_->GetImages().size(),
      .video_count = model_->GetVideos().size(),
      .font_count = model_->GetFonts().size(),
  };
  controller_actor->Act([meta = std::move(meta)](auto& controller) {
    controller->UpdateProperties(meta);
  });
}

void AnimaXRenderer::Render(double progress) {
  if (is_destroyed_ || !surface_ || is_invalid_platform_surface_) {
    return;
  }

  ANIMAX_TRACE_EVENT(kRenderFrame);

  ANIMAX_TRACE_EVENT_BEGIN(kInterpolateFrame);
  Trace(TraceEventType::kRenderFrameStart);
  layer_->SetProgress(progress);
  ANIMAX_TRACE_EVENT_END();

  ANIMAX_TRACE_EVENT_BEGIN(kDrawFrame);
  Canvas* canvas = surface_->Canvas();
  canvas->Save();
  ResizeCanvas(*canvas);
  canvas_matrix_ = canvas->GetMatrix();
  std::unique_ptr<Matrix> matrix = std::make_unique<Matrix>();
  layer_->Draw(*canvas, *matrix, 255);
  canvas->Restore();

  ANIMAX_TRACE_EVENT_END();
  surface_->Flush();
  NotifyFirstFrameIfNeeded();

  Trace(TraceEventType::kRenderFrameEnd);
}

void AnimaXRenderer::OnTap(float x, float y) {
  if (!layer_ || !canvas_matrix_) {
    return;
  }

  // Map x,y click location to scaled location.
  float invert_x = x, invert_y = y;
  auto invert_matrix = std::make_unique<Matrix>();
  if (canvas_matrix_->Invert(*invert_matrix)) {
    float points[2] = {x, y};
    invert_matrix->MapPoints(points, 1);
    invert_x = points[0];
    invert_y = points[1];
  }

  ANIMAX_LOGI("OnTap position x: " << x << ", y:" << y << ", invert_x:"
                                   << invert_x << ", invert_y:" << invert_y);

  std::unordered_set<std::string> hit_layers;
  layer_->HitTest(invert_x, invert_y, hit_layers);
  if (hit_layers.empty()) {
    return;
  }
  auto controller_actor = weak_controller_actor_.lock();
  if (!controller_actor) {
    return;
  }
  controller_actor->Act([hit_layers = std::move(hit_layers)](auto& controller) {
    controller->NotifyTapEvent(std::move(hit_layers));
  });
}

void AnimaXRenderer::SetObjectFit(const ObjectFit object_fit) {
  object_fit_ = object_fit;
}

void AnimaXRenderer::SetObjectPosition(const ObjectPosition object_position) {
  object_position_ = object_position;
}

void AnimaXRenderer::EnsureSubscribeValidOrWarn() {
  auto composition_model = GetComposition();
  if (!composition_model) {
    return;
  }

  auto controller_actor = weak_controller_actor_.lock();
  if (!controller_actor) {
    return;
  }

  auto start_frame = composition_model->GetStartFrame();
  EventWarningChecker::CheckIllegalSubscribedStartFrame(
      start_frame, [controller_actor](EventWarning warning,
                                      const std::string& warning_message) {
        controller_actor->Act([warning, warning_message](auto& controller) {
          controller->NotifyWarning(warning, warning_message);
        });
      });
}

void AnimaXRenderer::Destroy() {
  ANIMAX_LOGI("AnimaXRenderer Destroy");
  surface_ = nullptr;
  layer_ = nullptr;
  model_ = nullptr;
}

void AnimaXRenderer::MarkDestroyed() { is_destroyed_ = true; }

void AnimaXRenderer::MarkPlatformSurfaceAsInvalid(bool is_invalid) {
  is_invalid_platform_surface_ = is_invalid;
}

void AnimaXRenderer::ResizeCanvas(Canvas& canvas) {
  if (model_width_ <= 0 || model_height_ <= 0) {
    return;
  }

  float scale_factor_x = 1.f;
  float scale_factor_y = 1.f;

  // Determine scale factors based on object fit mode
  switch (object_fit_) {
    case ObjectFit::kCover:
      scale_factor_x = std::max(width_ / model_width_, height_ / model_height_);
      scale_factor_y = scale_factor_x;
      break;
    case ObjectFit::kContain:
      scale_factor_x = std::min(width_ / model_width_, height_ / model_height_);
      scale_factor_y = scale_factor_x;
      break;
    case ObjectFit::kFill:
      scale_factor_x = width_ / model_width_;
      scale_factor_y = height_ / model_height_;
      break;
    case ObjectFit::kScaleDown:
      scale_factor_x = std::min(
          1.f, std::min(width_ / model_width_, height_ / model_height_));
      scale_factor_y = scale_factor_x;
    default:
      break;
  }

  // Default to center alignment
  float dx = (width_ - scale_factor_x * model_width_) / 2.f;
  float dy = (height_ - scale_factor_y * model_height_) / 2.f;

  // Adjust translation based on object position
  switch (object_position_) {
    case ObjectPosition::kLeft:
      dx = 0.f;
      break;
    case ObjectPosition::kRight:
      dx = width_ - scale_factor_x * model_width_;
      break;
    case ObjectPosition::kTop:
      dy = 0.f;
      break;
    case ObjectPosition::kBottom:
      dy = height_ - scale_factor_y * model_height_;
      break;
    case ObjectPosition::kTopLeft:
      dx = dy = 0.f;
      break;
    case ObjectPosition::kTopRight:
      dx = width_ - scale_factor_x * model_width_;
      dy = 0.f;
      break;
    case ObjectPosition::kBottomLeft:
      dx = 0.f;
      dy = height_ - scale_factor_y * model_height_;
      break;
    case ObjectPosition::kBottomRight:
      dx = width_ - scale_factor_x * model_width_;
      dy = height_ - scale_factor_y * model_height_;
      break;
    case ObjectPosition::kCenter:
    default:
      break;
  }

  canvas.ResetMatrix();
  canvas.Translate(dx, dy);
  canvas.Scale(scale_factor_x, scale_factor_y);
}

void AnimaXRenderer::NotifyFirstFrameIfNeeded() {
  if (has_rendered_first_frame_) {
    return;
  }

  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([](auto& controller) {
      controller->NotifyCurrentFrameEvent(Event::kFirstFrame);
    });
  }

  has_rendered_first_frame_ = true;
}

void AnimaXRenderer::Reload() {
  auto controller_actor = weak_controller_actor_.lock();
  if (!controller_actor) {
    return;
  }
  if (!model_) {
    return;
  }
  controller_actor->Act([](auto& controller) {
    controller->NotifyCurrentFrameEvent(Event::kCompositionReady);
  });
  if (!surface_) {
    return;
  }
  controller_actor->Act([](auto& controller) {
    controller->NotifyCurrentFrameEvent(Event::kReady);
  });
}

bool AnimaXRenderer::IsCompositionAssetsLoaded() {
  return model_ != nullptr && model_->IsAssetsLoaded();
}

void AnimaXRenderer::GetKeysForKeyPath(std::unique_ptr<AnimaXKeyPath> key_path,
                                       AnimaXKeyPathCallback callback) {
  property_updater_->GetKeysForKeyPath(std::move(key_path),
                                       std::move(callback));
}

void AnimaXRenderer::UpdateLayerProperty(
    std::unique_ptr<LayerStaticRequest> request) {
  property_updater_->UpdateLayerProperty(std::move(request));
}

void AnimaXRenderer::SetResourceProperty(
    std::unique_ptr<ResourceUpdateRequest> request) {
  property_updater_->SetResourceProperty(std::move(request));
}

void AnimaXRenderer::AddLayerPropertyCallback(
    std::unique_ptr<LayerCallbackRequest> request) {
  property_updater_->AddLayerPropertyCallback(std::move(request));
}

void AnimaXRenderer::SetFpsEventInterval(long interval) {
  gpu_thread_recorder_.SetFPSReportThreshold(interval);
}

void AnimaXRenderer::Trace(TraceEventType type) {
  gpu_thread_recorder_.Trace(type);
}

TimestampArray AnimaXRenderer::ExportTimestamps() const {
  return gpu_thread_recorder_.Export();
}

MetricsMap AnimaXRenderer::ExportMetricsMap() const {
  MetricsMap metrics_to_export;
  auto avg_frame_time = gpu_thread_recorder_.GetAverageFrameTime();
  if (avg_frame_time > 0) {
    metrics_to_export[AnimaXMetricNames::kMeanFrameTime] = avg_frame_time;
  }
  auto max_frame_time = gpu_thread_recorder_.GetMaxFrameTime();
  if (max_frame_time > 0) {
    metrics_to_export[AnimaXMetricNames::kMaxFrameTime] = max_frame_time;
  }
  auto fps = gpu_thread_recorder_.GetFPS();
  if (fps > 0) {
    metrics_to_export[AnimaXMetricNames::kFps] = fps;
  }
  return metrics_to_export;
}

void AnimaXRenderer::AddAudioController(
    std::weak_ptr<AudioController> controller) {
  audio_controllers_.push_back(controller);
}

void AnimaXRenderer::OnResume() {
  for (auto weak_controller : audio_controllers_) {
    if (auto controller = weak_controller.lock()) {
      controller->OnResume();
    }
  }
}

void AnimaXRenderer::OnPause() {
  for (auto weak_controller : audio_controllers_) {
    if (auto controller = weak_controller.lock()) {
      controller->OnPause();
    }
  }
}

void AnimaXRenderer::OnEnd() { OnPause(); }

void AnimaXRenderer::OnCancel() { OnPause(); }

void AnimaXRenderer::SetVolume(double volume) {
  if (volume < 0) {
    volume_ = 0;
  } else if (volume > 1) {
    volume_ = 1;
  } else {
    volume_ = volume;
  }
  if (!mute_) {
    for (auto weak_controller : audio_controllers_) {
      if (auto controller = weak_controller.lock()) {
        controller->SetVolume(volume_);
      }
    }
  }
}

void AnimaXRenderer::SetMute(bool mute) {
  if (mute_ == mute) {
    return;
  }
  mute_ = mute;
  if (mute_) {
    SetVolume(0);
  } else {
    SetVolume(volume_);
  }
}

}  // namespace animax
}  // namespace lynx
