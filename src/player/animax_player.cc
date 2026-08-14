// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/player/animax_player.h"

#include "include/player/animax_player_builder.h"
#include "include/player/animax_surface.h"
#include "src/base/log/log.h"
#include "src/base/monitor/animax_metrics_manager.h"
#include "src/base/thread/task_runner.h"
#include "src/base/util/visibility_state.h"
#include "src/layer/composition_layer.h"
#include "src/player/animax_composition_loader.h"
#include "src/player/animax_event_dispatcher.h"
#include "src/player/animax_main_controller.h"
#include "src/player/animax_playback_event_handler.h"
#include "src/player/animax_renderer.h"
#include "src/resource/log_util.h"

namespace lynx {
namespace animax {

AnimaXPlayer::AnimaXPlayer(AnimaXPlayerBuilder& builder)
    : scale_(builder.scale_),
      ability_(builder.ability_),
      disable_playback_on_asset_load_failure_(
          builder.disable_playback_on_asset_load_failure_),
      gpu_thread_holder_(
          GetAnimaXGPUThreadHolder(builder.multi_thread_accelerate_)) {
  ANIMAX_LOGI("AnimaXPlayer constructor"
              << ", this: " << this);
}

void AnimaXPlayer::Init(AnimaXPlayerBuilder& builder) {
  // Create playback handler.
  playback_handler_ = std::make_shared<AnimaXPlaybackEventHandler>();

  // Create VSync monitor if not provided.
  auto vsync_monitor = builder.vsync_monitor_;
  if (!vsync_monitor) {
    vsync_monitor = std::make_shared<VSyncMonitor>();
  }

  // Create main controller actor and initialize controller capabilities.
  controller_actor_ = std::make_shared<shell::LynxActor<AnimaXMainController>>(
      std::unique_ptr<AnimaXMainController>(new AnimaXMainController(
          weak_from_this(), std::move(vsync_monitor), playback_handler_)),
      GetAnimaXMainThread());

  // Register event listeners provided by builder.
  controller_actor_->Act([listeners = std::move(builder.event_listeners_)](
                             auto& controller) mutable {
    for (auto& listener : listeners) {
      controller->AddEventListener(std::move(listener));
    }
  });

  // Create loader actor and initialize resource/unzip loaders.
  loader_actor_ = AnimaXCompositionLoader::Create();
  auto res_loader = builder.resource_loader_;
  auto unzip_loader = builder.unzip_loader_;
  if (res_loader || unzip_loader) {
    loader_actor_->Act([res_loader = std::move(res_loader),
                        unzip_loader = std::move(unzip_loader)](auto& loader) {
      loader->Init(res_loader, unzip_loader);
    });
  }

  // Create renderer actor for GPU thread operations.
  renderer_actor_ = std::make_shared<shell::LynxActor<AnimaXRenderer>>(
      std::unique_ptr<AnimaXRenderer>(new AnimaXRenderer(playback_handler_)),
      gpu_thread_holder_->Get());

  player_context_ = std::make_shared<AnimaXPlayerContext>();
  player_context_->weak_renderer_actor = renderer_actor_;
  player_context_->weak_main_controller = controller_actor_;
  player_context_->weak_ability = ability_;
  player_context_->weak_player = weak_from_this();
  player_context_->disable_render_in_background =
      builder.disable_render_in_background_;

  renderer_actor_->Act([player_context = player_context_](auto& renderer) {
    renderer->Init(player_context);
  });
  playback_handler_->Init(player_context_);

  // Create metrics manager.
  metrics_manager_ = std::make_shared<AnimaXMetricsManager>(
      loader_actor_, renderer_actor_, controller_actor_);
}

AnimaXPlayer::~AnimaXPlayer() {
  ANIMAX_LOGI("AnimaXPlayer destructor"
              << ", this: " << this);
}

PlayerEventTracker::EventArray AnimaXPlayer::GetEventTrackingArray() {
  return controller_actor_->ActSync(
      [](auto& controller) { return controller->GetEventTrackingArray(); });
}

const PlayerEventTracker::EventNameArray& AnimaXPlayer::GetEventNames() {
  // Since ActSync cannot return a reference safely,
  // and GetEventNames is thread-safe, we can call it on any thread.
  return controller_actor_->Impl()->GetEventNames();
}

void AnimaXPlayer::Destroy() {
  ANIMAX_LOGI("AnimaXPlayer Destroy"
              << ", this: " << this);
  controller_actor_->Act(
      [](auto& controller) { controller->ClearEventListeners(); });
  renderer_actor_->Impl()->MarkDestroyed();
  renderer_actor_->Act([](auto& renderer) { renderer->Destroy(); });
}

void AnimaXPlayer::Reload() {
  ANIMAX_LOGI("AnimaXPlayer Reload"
              << ", this: " << this)
  renderer_actor_->Act([](auto& renderer) { renderer->Reload(); });
  controller_actor_->Act([](auto& controller) { controller->Reload(); });
}

void AnimaXPlayer::CreateSurface(SurfaceCreationFactory creation_factory) {
  renderer_actor_->Act(
      [creation_factory = std::move(creation_factory)](auto& renderer) mutable {
        renderer->CreateSurface(std::move(creation_factory));
      });
}

void AnimaXPlayer::UpdateSurface(SurfaceUpdateFactory update_factory) {
  renderer_actor_->Act(
      [update_factory = std::move(update_factory)](auto& renderer) mutable {
        renderer->UpdateSurface(std::move(update_factory));
      });
}

void AnimaXPlayer::SetJson(const char* json) {
  std::string json_string = std::string(json);
  SetJson(std::move(json_string));
}

void AnimaXPlayer::SetJson(std::string json) {
  if (json.empty()) {
    ANIMAX_RESOURCE_LOGE("SetJson failed, json is empty, this: " << this);
    return;
  }

  if (!source_state_.SetJson(json)) {
    ANIMAX_RESOURCE_LOGI("SetJson failed, json is the same, this: " << this);
    return;
  }
  auto src_index = source_state_.GetIndex();

  controller_actor_->Act(
      [](auto& controller) { controller->SetCurrentSrc(""); });

  loader_actor_->Act([json = std::move(json), src_index, scale = scale_,
                      weak_player = weak_from_this()](auto& loader) {
    loader->LoadCompositionModelFromJSONString(
        std::move(json), scale,
        [weak_player, src_index](CompositionAssetResponse res,
                                 LoaderError error) {
          auto player = weak_player.lock();
          if (!player) {
            return;
          }
          player->OnCompositionLoaded(src_index, res, error);
        });
  });
}

void AnimaXPlayer::SetComposition(std::shared_ptr<CompositionModel> model) {
  if (!model) {
    ANIMAX_RESOURCE_LOGE("SetComposition failed, model is null");
    return;
  }
  source_state_.Reset();
  auto src_index = source_state_.GetIndex();
  ANIMAX_RESOURCE_LOGI("SetComposition index: " << src_index);
  controller_actor_->Act(
      [](auto& controller) { controller->SetCurrentSrc(""); });

  UpdateComposition(src_index, std::move(model));
}

void AnimaXPlayer::SetSrc(const std::string& src) {
  if (src.empty()) {
    ANIMAX_RESOURCE_LOGE("SetSrc failed, src is empty, this: " << this);
    return;
  }

  auto old_src = source_state_.src;
  if (!source_state_.SetSrc(src)) {
    ANIMAX_RESOURCE_LOGI("SetSrc failed, src is the same, this: " << this);
    return;
  }
  auto src_index = source_state_.GetIndex();
  ANIMAX_RESOURCE_LOGI("SetSrc index: "
                       << src_index << ", new src: " << src
                       << (old_src.empty() ? "" : ", old src: " + old_src));

  controller_actor_->Act(
      [src](auto& controller) { controller->SetCurrentSrc(src); });
  loader_actor_->Act([src_index, src, scale = scale_,
                      weak_player = weak_from_this()](auto& loader) {
    loader->LoadCompositionModelFromURI(
        src, scale,
        [weak_player, src_index](CompositionAssetResponse res,
                                 LoaderError error) {
          auto player = weak_player.lock();
          if (!player) {
            return;
          }

          player->OnCompositionLoaded(src_index, res, error);
        });
  });
}

void AnimaXPlayer::SetImageFolder(std::string image_folder) {
  loader_actor_->Act([image_folder = std::move(image_folder)](auto& loader) {
    loader->SetImageFolder(std::move(image_folder));
  });
}

void AnimaXPlayer::UpdateComposition(int32_t src_index,
                                     std::shared_ptr<CompositionModel> model) {
  if (!model) {
    return;
  }

  if (src_index != source_state_.GetIndex()) {
    return;
  }

  ANIMAX_LOGI("Update composition for index: " << src_index
                                               << ", this: " << this);

  controller_actor_->Act([](auto& controller) {
    controller->NotifyCurrentFrameEvent(Event::kCompositionReady);
  });

  renderer_actor_->Act(
      [src_index, model = std::move(model)](auto& renderer) mutable {
        renderer->UpdateComposition(src_index, std::move(model));
      });
}

int64_t AnimaXPlayer::GetEstimatedMemoryUsage() {
  return estimated_memory_usage_.load();
}

void AnimaXPlayer::SetEstimatedMemoryUsage(int64_t usage) {
  estimated_memory_usage_.store(usage);
}

void AnimaXPlayer::SetSrcPolyfill(
    std::unordered_map<std::string, std::string> polyfill) {
  loader_actor_->Act([polyfill = std::move(polyfill)](auto& loader) {
    loader->SetSrcPolyfill(std::move(polyfill));
  });
}

void AnimaXPlayer::LoadAssetsWithCallback(
    base::MoveOnlyClosure<void> completion) {
  auto handle_renderer = [weak_player = weak_from_this(),
                          completion =
                              std::move(completion)](auto& renderer) mutable {
    auto player = weak_player.lock();
    if (!player) {
      return;
    }

    auto composition = renderer->GetComposition();
    if (!composition) {
      ANIMAX_LOGI("LoadAssetsWithCallback failed, composition is null");
      return;
    }

    if (renderer->IsCompositionAssetsLoaded()) {
      player->controller_actor_->Act([completion = std::move(completion)](
                                         auto& controller) { completion(); });
      return;
    }
    player->LoadCompositionAssets(std::move(composition),
                                  std::move(completion));
  };
  renderer_actor_->Act(std::move(handle_renderer));
}

void AnimaXPlayer::LoadCompositionAssets(
    std::shared_ptr<CompositionModel> composition,
    base::MoveOnlyClosure<void> completion) {
  auto handle_asset_loading = [weak_player = weak_from_this(),
                               completion = std::move(completion)](
                                  CompositionAssetResponse res,
                                  LoaderError error) mutable {
    auto player = weak_player.lock();
    if (!player) {
      return;
    }

    if (error) {
      ANIMAX_RESOURCE_LOGE("LoadAssetsWithCallback failed, error: " << error);
      return;
    }

    player->controller_actor_->Act([completion = std::move(completion)](
                                       auto& controller) { completion(); });
  };

  loader_actor_->Act([composition = std::move(composition),
                      handle_asset_loading = std::move(handle_asset_loading)](
                         auto& loader) mutable {
    loader->LoadCompositionModelAsset(std::move(composition),
                                      std::move(handle_asset_loading));
  });
}

void AnimaXPlayer::OnCompositionLoaded(int32_t src_index,
                                       CompositionAssetResponse& res,
                                       LoaderError& error) {
  if (!res.model || error) {
    ANIMAX_RESOURCE_LOGE("Received index: " << src_index
                                            << ", error: " << error);
    std::ostringstream oss;
    oss << error;
    controller_actor_->Act([oss = std::move(oss)](auto& controller) {
      controller->NotifyError(EventError::kResourceNotFound, oss.str());
    });
    return;
  }

  if (!res.asset_responses.empty()) {
    ANIMAX_RESOURCE_LOGI(
        "Finished loading SRC composition model for index: " << src_index);

    bool has_failed_asset = false;
    for (const auto& asset : res.asset_responses) {
      if (asset.error.code != kSuccess) {
        has_failed_asset = true;
        break;
      }
    }
    if (has_failed_asset) {
      std::ostringstream oss;
      oss << res;
      const std::string msg = oss.str();

      if (disable_playback_on_asset_load_failure_) {
        controller_actor_->Act([msg](auto& controller) {
          controller->NotifyError(EventError::kAssetLoadFailed, msg);
        });
        return;
      } else {
        controller_actor_->Act([msg](auto& controller) {
          controller->NotifyWarning(EventWarning::kAssetLoadFailed, msg);
        });
      }
    }
  } else {
    ANIMAX_RESOURCE_LOGI(
        "Finished loading SRC composition model (no asset) for index: "
        << src_index);
  }

  UpdateComposition(src_index, std::move(res.model));
}

void AnimaXPlayer::SetLoop(const bool loop) {
  controller_actor_->Act(
      [loop](auto& controller) { controller->SetLoop(loop); });
}

void AnimaXPlayer::SetLoopCount(const int32_t loop_count) {
  controller_actor_->Act(
      [loop_count](auto& controller) { controller->SetLoopCount(loop_count); });
}

void AnimaXPlayer::SetAutoReverse(const bool auto_reverse) {
  controller_actor_->Act([auto_reverse](auto& controller) {
    controller->SetAutoReverse(auto_reverse);
  });
}

void AnimaXPlayer::SetSpeed(const double speed) {
  controller_actor_->Act(
      [speed](auto& controller) { controller->SetSpeed(speed); });
}

void AnimaXPlayer::SetFpsEventInterval(const long interval) {
  ANIMAX_LOGI("SetFpsEventInterval: " << interval);
  renderer_actor_->Act(
      [interval](auto& renderer) { renderer->SetFpsEventInterval(interval); });
}

void AnimaXPlayer::SetProgress(const double progress) {
  ANIMAX_LOGI("SetProgress: " << progress);
  controller_actor_->Act(
      [progress](auto& controller) { controller->SetProgress(progress); });
}

void AnimaXPlayer::SetAutoplay(const bool autoplay) {
  controller_actor_->Act(
      [autoplay](auto& controller) { controller->SetAutoplay(autoplay); });
}

void AnimaXPlayer::SetDynamicResource(bool dynamic) {
  ANIMAX_LOGI("SetDynamicResource: " << std::to_string(dynamic));
  controller_actor_->Act(
      [dynamic](auto& controller) { controller->SetDynamicResource(dynamic); });

  loader_actor_->Act(
      [dynamic](auto& loader) { loader->SetHasDynamicResource(dynamic); });
}

bool AnimaXPlayer::EnableDynamicResourceFeature() {
  return controller_actor_->ActSync([](auto& controller) {
    return controller->EnableDynamicResourceFeature();
  });
}

void AnimaXPlayer::SetStartFrame(const double start_frame) {
  controller_actor_->Act([start_frame](auto& controller) {
    controller->SetStartFrame(start_frame);
  });
}

void AnimaXPlayer::SetEndFrame(const double end_frame) {
  controller_actor_->Act(
      [end_frame](auto& controller) { controller->SetEndFrame(end_frame); });
}

void AnimaXPlayer::SetKeepLastFrame(const bool keep_last_frame) {
  controller_actor_->Act([keep_last_frame](auto& controller) {
    controller->SetKeepLastFrame(keep_last_frame);
  });
}

void AnimaXPlayer::SetObjectFit(const ObjectFit object_fit) {
  renderer_actor_->Act(
      [object_fit](auto& renderer) { renderer->SetObjectFit(object_fit); });
}

void AnimaXPlayer::SetObjectPosition(const ObjectPosition object_position) {
  renderer_actor_->Act([object_position](auto& renderer) {
    renderer->SetObjectPosition(object_position);
  });
}

void AnimaXPlayer::SetMuted(bool mute) {
  renderer_actor_->Act([mute](auto& renderer) { renderer->SetMuted(mute); });
}

void AnimaXPlayer::SetEnableAudio(bool enable) {
  loader_actor_->Act(
      [enable](auto& loader) { loader->SetEnableAudio(enable); });
}

void AnimaXPlayer::SetMaxFrameRate(const double max_frame_rate) {
  controller_actor_->Act([max_frame_rate](auto& controller) {
    controller->SetMaxFrameRate(max_frame_rate);
  });
}

void AnimaXPlayer::PlaySegment(double start_frame, double end_frame) {
  ANIMAX_LOGI("USER PlaySegment: " << start_frame << " to " << end_frame
                                   << ", this: " << this);
  controller_actor_->Act([start_frame, end_frame](auto& controller) {
    controller->PlaySegment(start_frame, end_frame);
  });
}

double AnimaXPlayer::GetWidth() {
  return renderer_actor_->ActSync(
      [](auto& renderer) { return renderer->GetWidth(); });
}

double AnimaXPlayer::GetHeight() {
  return renderer_actor_->ActSync(
      [](auto& renderer) { return renderer->GetHeight(); });
}

double AnimaXPlayer::GetDurationMs() {
  return controller_actor_->ActSync(
      [](auto& controller) { return controller->GetDurationMs(); });
}

void AnimaXPlayer::Play() {
  ANIMAX_LOGI("USER Play, this: " << this);
  controller_actor_->Act([](auto& controller) { controller->Play(); });
}

void AnimaXPlayer::Pause() {
  ANIMAX_LOGI("USER Pause, this: " << this);
  controller_actor_->Act([](auto& controller) { controller->Pause(); });
}

void AnimaXPlayer::Resume() {
  ANIMAX_LOGI("USER Resume, this: " << this);
  controller_actor_->Act([](auto& controller) { controller->Resume(); });
}

void AnimaXPlayer::Stop() {
  ANIMAX_LOGI("USER Stop, this: " << this);
  controller_actor_->Act([](auto& controller) { controller->Stop(); });
}

void AnimaXPlayer::Seek(double frame) {
  ANIMAX_LOGI("USER Seek: " << frame << ", this: " << this);
  controller_actor_->Act(
      [frame](auto& controller) { controller->Seek(frame); });
}

bool AnimaXPlayer::IsAnimating() {
  return controller_actor_->ActSync(
      [](auto& controller) { return controller->IsAnimating(); });
}

void AnimaXPlayer::SubscribeUpdateEvent(int32_t frame) {
  renderer_actor_->Act(
      [](auto& renderer) { renderer->EnsureSubscribeValidOrWarn(); });
  controller_actor_->Act(
      [frame](auto& controller) { controller->SubscribeUpdateEvent(frame); });
}

void AnimaXPlayer::UnsubscribeUpdateEvent(int32_t frame) {
  controller_actor_->Act(
      [frame](auto& controller) { controller->UnsubscribeUpdateEvent(frame); });
}

void AnimaXPlayer::SubscribeUpdateEvents(std::unordered_set<int32_t> frames,
                                         bool subscribe) {
  if (subscribe) {
    renderer_actor_->Act(
        [](auto& renderer) { renderer->EnsureSubscribeValidOrWarn(); });
  }

  controller_actor_->Act([frames, subscribe](auto& controller) {
    controller->SubscribeUpdateEvents(frames, subscribe);
  });
}

double AnimaXPlayer::GetCurrentFrame() {
  return controller_actor_->ActSync(
      [](auto& controller) { return controller->GetCurrentFrame(); });
}

void AnimaXPlayer::OnShow(VisibilityState state) {
  ANIMAX_LOGI("OnShow with state: " << StringifyVisibilityState(state)
                                    << ", this: " << this);
  controller_actor_->Act(
      [state](auto& controller) { controller->OnShow(state); });
}

void AnimaXPlayer::OnHide(VisibilityState state) {
  ANIMAX_LOGI("OnHide with state: " << StringifyVisibilityState(state)
                                    << ", this: " << this);
  controller_actor_->Act(
      [state](auto& controller) { controller->OnHide(state); });
}

void AnimaXPlayer::OnProgress(double progress, double current_frame) {
  renderer_actor_->Act(
      [progress](auto& renderer) { renderer->Render(progress); });
}

void AnimaXPlayer::OnTap(float x, float y) {
  renderer_actor_->Act([x, y](auto& renderer) { renderer->OnTap(x, y); });
}

void AnimaXPlayer::GetLayerBounds(const std::string& layer_name,
                                  LayerBoundsSpace bounds_space,
                                  LayerBoundsCallback callback) {
  auto invoke_callback = [callback = std::move(callback)](
                             bool success, float x = 0.f, float y = 0.f,
                             float width = 0.f, float height = 0.f) mutable {
    if (callback != nullptr) {
      callback(success, x, y, width, height);
    }
  };

  if (layer_name.empty()) {
    invoke_callback(false);
    return;
  }

  renderer_actor_->Act([layer_name, bounds_space,
                        invoke_callback = std::move(invoke_callback)](
                           auto& renderer) mutable {
    RectF bounds;
    bool success = renderer->GetLayerBounds(layer_name, bounds_space, bounds);
    invoke_callback(success, bounds.GetLeft(), bounds.GetTop(),
                    bounds.GetWidth(), bounds.GetHeight());
  });
}

void AnimaXPlayer::MarkPlatformSurfaceAsInvalid(bool isInvalid) {
  renderer_actor_->Act([isInvalid](auto& renderer) {
    renderer->MarkPlatformSurfaceAsInvalid(isInvalid);
  });
}

void AnimaXPlayer::ExportDataFromMetricsManager(
    ExternalMetricsReadyCallback callback) {
  metrics_manager_->Collect(std::move(callback));
}

void AnimaXPlayer::GetKeysForKeyPath(std::unique_ptr<AnimaXKeyPath> key_path,
                                     AnimaXKeyPathCallback callback) {
  renderer_actor_->Act(
      [key_path = std::move(key_path),
       callback = std::move(callback)](auto& renderer) mutable {
        renderer->GetKeysForKeyPath(std::move(key_path), std::move(callback));
      });
}

void AnimaXPlayer::UpdateLayerProperty(
    std::unique_ptr<LayerStaticRequest> request) {
  renderer_actor_->Act([request = std::move(request)](auto& renderer) mutable {
    renderer->UpdateLayerProperty(std::move(request));
  });
}

void AnimaXPlayer::SetResourceProperty(
    std::unique_ptr<ResourceUpdateRequest> request) {
  renderer_actor_->Act([request = std::move(request)](auto& renderer) mutable {
    renderer->SetResourceProperty(std::move(request));
  });
}

void AnimaXPlayer::AddLayerPropertyCallback(
    std::unique_ptr<LayerCallbackRequest> request) {
  renderer_actor_->Act([request = std::move(request)](auto& renderer) mutable {
    renderer->AddLayerPropertyCallback(std::move(request));
  });
}

#ifdef OS_IOS
void AnimaXPlayer::OnAppEnterForeground() {
  ANIMAX_LOGI("OnAppEnterForeground");
  renderer_actor_->Act(
      [](auto& renderer) { renderer->SetInBackground(false); });
}
void AnimaXPlayer::OnAppEnterBackground() {
  ANIMAX_LOGI("OnAppEnterBackground");
  renderer_actor_->Act([](auto& renderer) { renderer->SetInBackground(true); });
}
#endif

}  // namespace animax
}  // namespace lynx
