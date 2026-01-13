// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_MAIN_CONTROLLER_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_MAIN_CONTROLLER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "base/include/lynx_actor.h"
#include "include/base/player_event_tracker.h"
#include "include/base/visibility_state.h"
#include "include/player/animax_event.h"
#include "include/player/vsync_monitor.h"
#include "src/player/animax_event_dispatcher.h"

namespace lynx {
namespace animax {

class AnimaXPlayer;
class AnimaXValueAnimator;
class VSyncMonitor;
class AnimaXPlaybackEventHandler;

struct CompositionModelMeta {
  float start_frame = 0.f;
  float end_frame = 0.f;
  long duration = 0;
  float frame_rate = 0.f;
  bool has_video_layer = false;
  size_t image_count = 0;
  size_t video_count = 0;
  size_t font_count = 0;
};

class AnimaXMainController {
 public:
  AnimaXMainController(
      std::weak_ptr<AnimaXPlayer> weak_player,
      std::shared_ptr<VSyncMonitor> vsync_monitor,
      std::weak_ptr<AnimaXPlaybackEventHandler> playback_handler);
  ~AnimaXMainController();

  // Event dispatcher operations
  void AddEventListener(EventListener listener);
  void ClearEventListeners();
  void NotifyError(const EventError error, const std::string& message);
  void NotifyWarning(const EventWarning warning, const std::string& message);
  void NotifyCurrentFrameEvent(const Event event);
  void NotifyFpsEvent(double fps, int32_t max_drop);
  void NotifyTapEvent(const std::unordered_set<std::string>& hit_layers);

  // Value animator operations
  void SeekToFrame(double frame);
  void SeekToCurrentFrame();
  void SetLoop(bool loop);
  void SetLoopCount(const int32_t loop_count);
  void SetSpeed(double speed);
  void SetAutoReverse(bool auto_reverse);
  void Pause();
  void Resume();
  void Stop();
  void Seek(double frame);
  void Play();
  void PlaySegment(double start_frame, double end_frame);

  // Playback event handlers
  void OnNewLoop(int32_t current_loop);
  void OnEnd();
  void OnProgress(double progress, double current_frame);

  // Visiblity handlers
  void OnShow(VisibilityState state);
  void OnHide(VisibilityState state);

  void UpdateProperties(const CompositionModelMeta& meta);

  // Utility operations
  double GetDurationMs();
  bool IsAnimating();
  double GetCurrentFrame();
  std::string GetPlayerID();
  std::string GetAnimationID();
  float GetTotalFrame();
  int32_t GetLoopIndex();
  std::string GetCurrentSrc();
  void SetCurrentSrc(const std::string& src);

  void SetStartFrame(const double start_frame);
  void SetEndFrame(const double end_frame);
  void SetKeepLastFrame(const bool keep_last_frame);
  void SetFpsEventInterval(long interval);
  void SetMaxFrameRate(const double max_frame_rate);
  void SetProgress(float progress);
  void SetAutoplay(bool autoplay);
  bool EnableDynamicResourceFeature();
  void SetDynamicResource(bool dynamic);
  void Reload();

  // Event subscription operations
  void SubscribeUpdateEvent(int32_t frame);
  void UnsubscribeUpdateEvent(int32_t frame);
  void SubscribeUpdateEvents(const std::unordered_set<int32_t>& frames,
                             bool subscribe);

  // Event tracking
  PlayerEventTracker::EventArray GetEventTrackingArray();
  const PlayerEventTracker::EventNameArray& GetEventNames();
  void MarkEvent(Event event);

 private:
  void EnsureReadyOrWarn(const std::string& operation_name);
  void UpdateAnimationID();
  void PlayFromStart();

  std::weak_ptr<AnimaXPlayer> weak_player_;
  std::shared_ptr<AnimaXEventDispatcher> event_dispatcher_;
  std::shared_ptr<AnimaXValueAnimator> value_animator_;
  std::shared_ptr<PlayerEventTracker> event_tracker_ =
      std::make_shared<PlayerEventTracker>();

  bool keep_last_frame_ = true;
  bool autoplay_ = true;
  bool dynamic_resource_ = false;
  double user_progress_ = 0.0;

  int32_t animation_count_ = 0;
  std::string player_id_;
  std::string animation_id_;

  CompositionModelMeta model_meta_;
  double current_frame_ = 0.0;
  int32_t current_loop_ = 0;

  std::string current_src_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_MAIN_CONTROLLER_H_
