// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_main_controller.h"

#include <cmath>

#include "include/player/animax_player.h"
#include "include/property/animax_player_global.h"
#include "src/animator/animax_value_animator.h"
#include "src/base/log/log.h"
#include "src/base/util/composition_frame_util.h"
#include "src/base/util/event_warning_checker.h"
#include "src/player/animax_playback_event_handler.h"

namespace lynx {
namespace animax {

AnimaXMainController::AnimaXMainController(
    std::weak_ptr<AnimaXPlayer> weak_player,
    std::shared_ptr<VSyncMonitor> vsync_monitor,
    std::weak_ptr<AnimaXPlaybackEventHandler> playback_handler)
    : weak_player_(std::move(weak_player)),
      value_animator_(AnimaXValueAnimator::Create(
          std::move(vsync_monitor), std::move(playback_handler))) {
  event_dispatcher_ =
      std::make_shared<AnimaXEventDispatcher>(weak_player_, *this);
  player_id_ =
      std::string("ANIMAX_") + std::to_string(reinterpret_cast<intptr_t>(this));
  animation_id_ = player_id_ + std::string("_ANIMATION_") +
                  std::to_string(animation_count_);
  AnimaXPlayerGlobal::Register(player_id_, weak_player_);
  ANIMAX_LOGI("AnimaXMainController Create"
              << ", this: " << this);
}

AnimaXMainController::~AnimaXMainController() {
  AnimaXPlayerGlobal::Unregister(player_id_);
  ANIMAX_LOGI("AnimaXMainController Destroy"
              << ", this: " << this);
}

// Event dispatcher operations
void AnimaXMainController::AddEventListener(EventListener listener) {
  event_dispatcher_->AddEventListener(std::move(listener));
}

void AnimaXMainController::ClearEventListeners() {
  event_dispatcher_->ClearEventListeners();
}

void AnimaXMainController::NotifyError(const EventError error,
                                       const std::string& message) {
  event_dispatcher_->NotifyError(error, message);
}

void AnimaXMainController::NotifyWarning(const EventWarning warning,
                                         const std::string& message) {
  event_dispatcher_->NotifyWarning(warning, message);
}

void AnimaXMainController::NotifyCurrentFrameEvent(const Event event) {
  event_dispatcher_->NotifyFrameEvent(event, GetCurrentFrame());
}

void AnimaXMainController::NotifyFpsEvent(double fps, int32_t max_drop) {
  event_dispatcher_->NotifyFps(GetCurrentFrame(), max_drop, fps);
}

void AnimaXMainController::NotifyTapEvent(
    const std::unordered_set<std::string>& hit_layers) {
  event_dispatcher_->NotifyTap(hit_layers);
}

// Value animator operations
void AnimaXMainController::PlayFromStart() {
  event_dispatcher_->ClearPrevNotifiedFrame();
  current_loop_ = 0;
  value_animator_->SeekToProgress(user_progress_, current_loop_);
  value_animator_->Start();
  user_progress_ = 0.0;
}

void AnimaXMainController::SeekToFrame(double frame) {
  value_animator_->SeekToFrame(frame);
}

void AnimaXMainController::SeekToCurrentFrame() {
  value_animator_->SeekToFrame(current_frame_, current_loop_);
}

void AnimaXMainController::SetLoop(bool loop) {
  value_animator_->SetLoop(loop);
}

void AnimaXMainController::SetLoopCount(int32_t loop_count) {
  value_animator_->SetLoopCount(loop_count);
}

void AnimaXMainController::SetSpeed(double speed) {
  value_animator_->SetSpeed(speed);
  event_dispatcher_->SetSpeed(speed);
}

void AnimaXMainController::SetAutoReverse(bool auto_reverse) {
  value_animator_->SetAutoReverse(auto_reverse);
}

void AnimaXMainController::Pause() {
  EnsureReadyOrWarn("Pause");
  value_animator_->Pause();
}

void AnimaXMainController::Resume() {
  EnsureReadyOrWarn("Resume");
  event_dispatcher_->ClearPrevNotifiedFrame();
  value_animator_->Resume();
}

void AnimaXMainController::Stop() {
  EnsureReadyOrWarn("Stop");
  value_animator_->Stop();
}

void AnimaXMainController::Seek(double frame) {
  EnsureReadyOrWarn("Seek");
  event_dispatcher_->ClearPrevNotifiedFrame();
  value_animator_->SeekToFrame(frame);
}

void AnimaXMainController::UpdateProperties(const CompositionModelMeta& meta) {
  model_meta_ = meta;
  // stop last animation
  value_animator_->Stop();
  current_frame_ = meta.start_frame;
  current_progress_ = 0.0;
  current_loop_ = 0;
  value_animator_->SetOriginFrameProperty(meta.start_frame, meta.end_frame,
                                          meta.frame_rate);

  UpdateAnimationID();
  NotifyCurrentFrameEvent(Event::kReady);

  // Check dynamic-resource property set is llegal
  EventWarningChecker::CheckDynamicResourceLoadable(
      autoplay_, dynamic_resource_,
      [this](EventWarning warning, const std::string& warning_message) {
        NotifyWarning(warning, warning_message);
      });

  // Check resource count
  EventWarningChecker::CheckAssetCountOverLimit(
      meta.image_count, meta.video_count, meta.font_count,
      [this](EventWarning warning, const std::string& warning_message) {
        NotifyWarning(warning, warning_message);
      });

  if (autoplay_) {
    ANIMAX_LOGI("AutoPlay true");
    PlayFromStart();
  } else if (!EnableDynamicResourceFeature()) {
    ANIMAX_LOGI("AutoPlay false, Seek user progress: " << user_progress_);
    value_animator_->SeekToProgress(user_progress_);
  }
}

std::string AnimaXMainController::GetPlayerID() { return player_id_; }

std::string AnimaXMainController::GetAnimationID() { return animation_id_; }

float AnimaXMainController::GetTotalFrame() {
  if (!IsAnimationReady()) {
    return 0.0f;
  }
  return CompositionFrameUtil::ToTimelineDurationFrames(model_meta_.start_frame,
                                                        model_meta_.end_frame);
}

int32_t AnimaXMainController::GetLoopIndex() { return current_loop_; }

std::string AnimaXMainController::GetCurrentSrc() { return current_src_; }

void AnimaXMainController::SetCurrentSrc(const std::string& src,
                                         bool allow_extensionless_json) {
  current_src_ = src;
  event_tracker_->Reset();
  if (!src.empty()) {
    EventWarningChecker::CheckLottieFormat(
        src,
        [this](EventWarning warning, const std::string& warning_message) {
          NotifyWarning(warning, warning_message);
        },
        allow_extensionless_json);
  }
}

double AnimaXMainController::GetDurationMs() {
  EnsureReadyOrWarn("GetDuration");
  return model_meta_.duration;
}

bool AnimaXMainController::IsAnimating() {
  return value_animator_->IsAnimating();
}

void AnimaXMainController::EnsureReadyOrWarn(
    const std::string& operation_name) {
  EventWarningChecker::CheckExecuteBeforeReady(
      IsAnimationReady(), operation_name,
      [this](EventWarning warning, const std::string& warning_message) {
        NotifyWarning(warning, warning_message);
      });
}

// Utility operations
void AnimaXMainController::SetStartFrame(const double start_frame) {
  value_animator_->SetStartFrame(start_frame);
}

void AnimaXMainController::SetEndFrame(const double end_frame) {
  value_animator_->SetEndFrame(end_frame);
}

void AnimaXMainController::SetKeepLastFrame(const bool keep_last_frame) {
  keep_last_frame_ = keep_last_frame;
}

void AnimaXMainController::SetMaxFrameRate(const double max_frame_rate) {
  value_animator_->SetMaxFrameRate(max_frame_rate);
}

void AnimaXMainController::SetProgress(float progress) {
  user_progress_ = progress;
  value_animator_->SeekToProgress(progress);
}

void AnimaXMainController::SetAutoplay(bool autoplay) {
  if (autoplay_ == autoplay) {
    return;
  }
  autoplay_ = autoplay;
  value_animator_->SetAutoplay(autoplay);
  if (autoplay && IsAnimationReady()) {
    Play();
  }
}

bool AnimaXMainController::EnableDynamicResourceFeature() {
  return !autoplay_ && dynamic_resource_;
}

void AnimaXMainController::SetDynamicResource(bool dynamic) {
  dynamic_resource_ = dynamic;
}

void AnimaXMainController::Reload() {
  if (autoplay_) {
    PlayFromStart();
  } else {
    value_animator_->Stop();
    value_animator_->SeekToFrame(value_animator_->GetStartFrame());
  }
}

void AnimaXMainController::Play() {
  if (!IsAnimationReady()) {
    if (!EnableDynamicResourceFeature()) {
      SetAutoplay(true);
    }
    return;
  }

  auto player = weak_player_.lock();
  if (!player) {
    return;
  }

  EnsureReadyOrWarn("Play");

  auto play_action = [this, weak_player = weak_player_]() {
    if (auto player = weak_player.lock()) {
      PlayFromStart();
    }
  };
  if (EnableDynamicResourceFeature()) {
    player->LoadAssetsWithCallback(std::move(play_action));
  } else {
    play_action();
  }
}

void AnimaXMainController::PlaySegment(double start_frame, double end_frame) {
  auto player = weak_player_.lock();
  if (!player) {
    return;
  }

  EnsureReadyOrWarn("PlaySegment");
  auto play_segment_action = [start_frame, end_frame, this,
                              weak_player = weak_player_]() {
    if (auto player = weak_player.lock()) {
      value_animator_->PlaySegment(start_frame, end_frame);
    }
  };
  if (EnableDynamicResourceFeature()) {
    player->LoadAssetsWithCallback(std::move(play_segment_action));
  } else {
    play_segment_action();
  }
}

// Event subscription operations
void AnimaXMainController::SubscribeUpdateEvent(int32_t frame) {
  const auto start_frame = value_animator_->GetStartFrame();
  EventWarningChecker::CheckIllegalSubscribedStartFrame(
      start_frame,
      [this](EventWarning warning, const std::string& warning_message) {
        NotifyWarning(warning, warning_message);
      });
  event_dispatcher_->SubscribeUpdateEvent(frame);
}

void AnimaXMainController::UnsubscribeUpdateEvent(int32_t frame) {
  event_dispatcher_->UnsubscribeUpdateEvent(frame);
}

void AnimaXMainController::SubscribeUpdateEvents(
    const std::unordered_set<int32_t>& frames, bool subscribe) {
  if (subscribe) {
    const auto start_frame = value_animator_->GetStartFrame();
    EventWarningChecker::CheckIllegalSubscribedStartFrame(
        start_frame,
        [this](EventWarning warning, const std::string& warning_message) {
          event_dispatcher_->NotifyWarning(warning, warning_message);
        });
  }

  event_dispatcher_->SubscribeUpdateEvents(frames, subscribe);
}

void AnimaXMainController::UpdateAnimationID() {
  animation_id_ = player_id_ + std::string("_ANIMATION_") +
                  std::to_string(++animation_count_);
}

void AnimaXMainController::OnNewLoop(int32_t current_loop) {
  current_loop_ = current_loop;
  event_dispatcher_->ClearPrevNotifiedFrame();
  NotifyCurrentFrameEvent(Event::kRepeat);
}

void AnimaXMainController::OnEnd() {
  if (!keep_last_frame_) {
    value_animator_->SeekToFrame(0);
  }
  event_dispatcher_->ClearPrevNotifiedFrame();
  NotifyCurrentFrameEvent(Event::kCompletion);
}

void AnimaXMainController::OnProgress(double progress, double current_frame) {
  // Round frame to nearest integer and recalculate progress if there is video
  // layer in the composition
  if (model_meta_.has_video_layer &&
      model_meta_.end_frame > model_meta_.start_frame) {
    current_frame = std::round(current_frame);
    progress = (current_frame - model_meta_.start_frame) /
               std::round(model_meta_.end_frame - model_meta_.start_frame);
  }

  current_frame_ = current_frame;
  current_progress_ = progress;
  event_dispatcher_->NotifyUpdateEvent(current_frame_);

  auto player = weak_player_.lock();
  if (player) {
    player->OnProgress(progress, current_frame);
  }
}

void AnimaXMainController::OnShow(VisibilityState state) {
  value_animator_->OnShow(state);
}

void AnimaXMainController::OnHide(VisibilityState state) {
  value_animator_->OnHide(state);
}

PlayerEventTracker::EventArray AnimaXMainController::GetEventTrackingArray() {
  return event_tracker_->GetEvents();
}

const PlayerEventTracker::EventNameArray&
AnimaXMainController::GetEventNames() {
  return event_tracker_->GetEventNames();
}

void AnimaXMainController::MarkEvent(Event event) {
  switch (event) {
    case Event::kCompletion:
      event_tracker_->MarkEvent(
          PlayerEventTracker::AnimationEventType::kComplete);
      break;
    case Event::kCancel:
      event_tracker_->MarkEvent(
          PlayerEventTracker::AnimationEventType::kCancel);
      break;
    case Event::kRepeat:
      event_tracker_->MarkEvent(
          PlayerEventTracker::AnimationEventType::kRepeat);
      break;
    case Event::kUpdate:
      event_tracker_->MarkEvent(
          PlayerEventTracker::AnimationEventType::kUpdate);
      break;
    case Event::kStart:
      event_tracker_->MarkEvent(PlayerEventTracker::AnimationEventType::kStart);
      break;
    case Event::kReady:
      event_tracker_->MarkEvent(PlayerEventTracker::AnimationEventType::kReady);
      break;
    case Event::kError:
      event_tracker_->MarkEvent(PlayerEventTracker::AnimationEventType::kError);
      break;
    case Event::kWarning:
      event_tracker_->MarkEvent(
          PlayerEventTracker::AnimationEventType::kWarning);
      break;
    default:;  // no change upon the event_tracker_
  }
}

double AnimaXMainController::GetCurrentFrame() { return current_frame_; }

double AnimaXMainController::GetProgress() { return current_progress_; }

bool AnimaXMainController::IsAnimationReady() {
  return event_tracker_->GetEventValue(
      PlayerEventTracker::AnimationEventType::kReady);
}

}  // namespace animax
}  // namespace lynx
