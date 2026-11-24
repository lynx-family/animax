// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/player/animax_player.h"

#include <cmath>

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
#include "src/player/composition_binding_state.h"
#include "src/player/layer_query.h"
#include "src/resource/log_util.h"

namespace lynx {
namespace animax {
namespace {
std::atomic<uint64_t> next_instance_token{1};

}  // namespace

CompositionBindingRequest::CompositionBindingRequest(
    std::shared_ptr<CompositionBindingState> state, uint64_t id,
    Completion completion)
    : state_(std::move(state)), id_(id), completion_(std::move(completion)) {}

bool CompositionBindingRequest::IsPending() const {
  return phase_.load() == Phase::kPending;
}

void CompositionBindingRequest::Fail(std::string code, std::string message) {
  Completion completion;
  {
    std::lock_guard<std::mutex> lock(state_->mutex);
    if (phase_ != Phase::kPending) {
      return;
    }
    phase_ = Phase::kFailed;
    completion = std::move(completion_);
  }
  if (completion) {
    completion(false, std::move(code), std::move(message));
  }
}

bool AnimaXPlayer::SetCompositionMode(bool enabled) {
  std::lock_guard<std::mutex> lock(binding_state_->mutex);
  if (binding_state_->mode_locked) {
    return binding_state_->composition_mode == enabled;
  }
  binding_state_->mode_locked = true;
  binding_state_->composition_mode = enabled;
  return true;
}

bool AnimaXPlayer::IsCompositionMode() const {
  std::lock_guard<std::mutex> lock(binding_state_->mutex);
  return binding_state_->composition_mode;
}

void AnimaXPlayer::SetCompositionHost(std::weak_ptr<PreloadHost> host,
                                      CompositionHostGuard guard) {
  std::lock_guard<std::mutex> lock(binding_state_->mutex);
  // A component belongs to the page that created it for its entire lifetime.
  if (!binding_state_->valid || binding_state_->host_guard) {
    return;
  }
  binding_state_->host = std::move(host);
  binding_state_->host_guard = std::move(guard);
}

std::string AnimaXPlayer::GetPlayerID() const {
  // The ID is assigned synchronously at creation and never changes.
  return controller_actor_->Impl()->GetPlayerID();
}

CompositionBindingError AnimaXPlayer::ValidatePlayerReceiver(
    uint64_t token, const std::shared_ptr<PreloadHost>& owner) const {
  CompositionHostGuard guard;
  {
    std::lock_guard<std::mutex> lock(binding_state_->mutex);
    if (token != instance_token_) {
      return {"InstanceMismatch", "The component instance has changed"};
    }
    if (!binding_state_->valid) {
      return {"Destroyed", "The component has been destroyed"};
    }
    const auto& host = binding_state_->host;
    if (!owner || host.expired() || !binding_state_->host_guard) {
      return {"UnsupportedCapability", "The component has no preload host"};
    }
    if (host.owner_before(owner) || owner.owner_before(host)) {
      return {"HostMismatch", "The component belongs to another preload host"};
    }
    guard = binding_state_->host_guard;
  }
  // Do not acquire host validity while holding the Player lock.
  if (!guard([] {})) {
    return {"EnvironmentUnavailable", "Preload host is unavailable"};
  }
  return {};
}

CompositionBindingError AnimaXPlayer::ValidateCompositionReceiver(
    uint64_t token, const std::shared_ptr<PreloadHost>& owner) const {
  auto error = ValidatePlayerReceiver(token, owner);
  if (!error.IsSuccess()) return error;
  std::lock_guard<std::mutex> lock(binding_state_->mutex);
  if (!binding_state_->composition_mode) {
    return {"SourceConflict", "The component uses src/json loading"};
  }
  return {};
}

std::shared_ptr<CompositionBindingRequest>
AnimaXPlayer::BeginCompositionBinding(
    uint64_t request_id, CompositionBindingRequest::Completion completion) {
  auto request =
      std::shared_ptr<CompositionBindingRequest>(new CompositionBindingRequest(
          binding_state_, request_id, std::move(completion)));
  CompositionBindingRequest::Completion previous_completion;
  CompositionBindingError error;
  {
    std::lock_guard<std::mutex> lock(binding_state_->mutex);
    if (!binding_state_->valid) {
      error = {"Destroyed", "The component has been destroyed"};
    } else if (request_id == 0 ||
               request_id <= binding_state_->latest_request_id) {
      error = {"Superseded", "A newer binding request has already arrived"};
    } else {
      if (auto previous = binding_state_->pending.lock()) {
        if (previous->phase_ == CompositionBindingRequest::Phase::kPending) {
          previous->phase_ = CompositionBindingRequest::Phase::kFailed;
          previous_completion = std::move(previous->completion_);
        }
      }
      binding_state_->latest_request_id = request_id;
      binding_state_->pending = request;
    }
  }
  if (previous_completion) {
    previous_completion(false, "Superseded", "A newer binding request arrived");
  }
  if (!error.IsSuccess()) {
    request->Fail(std::move(error.code), std::move(error.message));
  }
  return request;
}

void AnimaXPlayer::CancelPendingCompositionBinding() {
  std::shared_ptr<CompositionBindingRequest> pending;
  {
    std::lock_guard<std::mutex> lock(binding_state_->mutex);
    if (!binding_state_->composition_mode) {
      return;
    }
    pending = binding_state_->pending.lock();
  }
  if (pending) {
    // Fail is a no-op after commitment and dispatches completion outside the
    // state lock. Do not wait for preparation or permanently destroy a node
    // that may later be inserted into the tree again.
    pending->Fail("Destroyed",
                  "The component was removed before binding completed");
  }
}

void AnimaXPlayer::CancelCompositionBinding(uint64_t request_id) {
  std::shared_ptr<CompositionBindingRequest> pending;
  {
    std::lock_guard<std::mutex> lock(binding_state_->mutex);
    if (!binding_state_->composition_mode) {
      return;
    }
    pending = binding_state_->pending.lock();
    if (!pending || pending->Id() != request_id) {
      return;
    }
  }
  // Fail takes the same state lock as Renderer commitment, so call it after
  // releasing the lock above. If commitment won, cancellation is a no-op;
  // otherwise the original completion reports failure without consuming data.
  pending->Fail("Superseded", "The binding request was cancelled");
}

CompositionBindingError AnimaXPlayer::ValidateCompositionBinding(
    const std::shared_ptr<CompositionBindingRequest>& request,
    const std::shared_ptr<PreloadHost>& owner, float scale) const {
  auto error = ValidateCompositionReceiver(instance_token_, owner);
  if (!error.IsSuccess()) {
    return error;
  }
  std::lock_guard<std::mutex> lock(binding_state_->mutex);
  if (!request || request->state_ != binding_state_) {
    return {"InstanceMismatch",
            "The binding request belongs to another player"};
  }
  if (!binding_state_->valid) {
    return {"Destroyed", "The component has been destroyed"};
  }
  if (request->phase_ != CompositionBindingRequest::Phase::kPending ||
      request->id_ != binding_state_->latest_request_id) {
    return {"Superseded", "The binding request is no longer pending"};
  }
  if (!std::isfinite(scale) || scale <= 0 || scale != scale_) {
    return {"ScaleMismatch", "Composition scale " + std::to_string(scale) +
                                 " differs from player scale " +
                                 std::to_string(scale_)};
  }
  return {};
}

bool AnimaXPlayer::IsCurrentInstallation(uint64_t installation_id) const {
  std::lock_guard<std::mutex> lock(binding_state_->mutex);
  return binding_state_->valid &&
         binding_state_->installed_id == installation_id;
}

AnimaXPlayer::AnimaXPlayer(AnimaXPlayerBuilder& builder)
    : instance_token_(next_instance_token.fetch_add(1)),
      binding_state_(std::make_shared<CompositionBindingState>()),
      scale_(builder.scale_),
      allow_extensionless_json_(builder.allow_extensionless_json_),
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
  loader_actor_->Act(
      [allow_extensionless_json = allow_extensionless_json_](auto& loader) {
        loader->SetAllowExtensionlessJson(allow_extensionless_json);
      });
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
  std::shared_ptr<CompositionBindingRequest> pending;
  {
    std::lock_guard<std::mutex> lock(binding_state_->mutex);
    binding_state_->valid = false;
    pending = binding_state_->pending.lock();
  }
  if (pending) {
    pending->Fail("Destroyed", "The component has been destroyed");
  }

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

void AnimaXPlayer::RunAfterPendingRenderTasks(base::closure callback) {
  renderer_actor_->Act(
      [callback = std::move(callback)](auto&) mutable { callback(); });
}

void AnimaXPlayer::SetJson(const char* json) {
  std::string json_string = std::string(json);
  SetJson(std::move(json_string));
}

void AnimaXPlayer::SetJson(std::string json) {
  if (!SetCompositionMode(false)) return;
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
  if (!SetCompositionMode(false)) return;
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
      [src,
       allow_extensionless_json = allow_extensionless_json_](auto& controller) {
        controller->SetCurrentSrc(src, allow_extensionless_json);
      });
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
  if (IsCompositionMode()) return;
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

    if (player->IsCompositionMode()) {
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
  if (IsCompositionMode()) return;
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

  loader_actor_->Act(
      [composition = std::move(composition), weak_player = weak_from_this(),
       handle_asset_loading =
           std::move(handle_asset_loading)](auto& loader) mutable {
        auto player = weak_player.lock();
        if (!player || player->IsCompositionMode()) return;
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
  if (dynamic && IsCompositionMode()) return;
  ANIMAX_LOGI("SetDynamicResource: " << std::to_string(dynamic));
  controller_actor_->Act(
      [dynamic, weak_player = weak_from_this()](auto& controller) {
        auto player = weak_player.lock();
        if (dynamic && player && player->IsCompositionMode()) return;
        controller->SetDynamicResource(dynamic);
      });

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

double AnimaXPlayer::GetProgress() {
  return controller_actor_->ActSync(
      [](auto& controller) { return controller->GetProgress(); });
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
  OnProgress(progress, current_frame, 0);
}

void AnimaXPlayer::OnProgress(double progress, double current_frame,
                              uint64_t installation_id) {
  renderer_actor_->Act([progress, installation_id](auto& renderer) {
    if (renderer->IsCurrentInstallation(installation_id)) {
      renderer->Render(progress);
    }
  });
}

void AnimaXPlayer::OnTap(float x, float y) {
  renderer_actor_->Act([x, y](auto& renderer) { renderer->OnTap(x, y); });
}

void AnimaXPlayer::GetLayerBounds(const std::string& layer_name,
                                  LayerBoundsSpace bounds_space,
                                  LayerBoundsCallback callback) {
  QueryLayer(layer_name, LayerQueryType::kBounds, bounds_space,
             [callback = std::move(callback)](const LayerQueryResult& result) {
               if (callback) {
                 callback(result.success, result.position.x, result.position.y,
                          result.width, result.height);
               }
             });
}

void AnimaXPlayer::QueryLayer(const std::string& layer_name,
                              LayerQueryType query,
                              LayerBoundsSpace bounds_space,
                              LayerQueryCallback callback) {
  if (layer_name.empty()) {
    callback({});
    return;
  }

  renderer_actor_->Act([layer_name, query, bounds_space,
                        callback = std::move(callback)](auto& renderer) {
    callback(renderer->QueryLayer(layer_name, query, bounds_space));
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
