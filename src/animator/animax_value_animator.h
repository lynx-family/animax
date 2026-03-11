// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_ANIMATOR_ANIMAX_VALUE_ANIMATOR_H_
#define ANIMAX_SRC_ANIMATOR_ANIMAX_VALUE_ANIMATOR_H_

#include <memory>

#include "include/base/visibility_state.h"
#include "src/base/thread/task_runner.h"
#include "src/player/value_animator_listener.h"

namespace lynx {
namespace animax {
class TaskRunner;
class ValueAnimator;
class VSyncMonitor;
/**
 * thread safe
 */
class AnimaXValueAnimator final
    : public std::enable_shared_from_this<AnimaXValueAnimator> {
 public:
  static std::shared_ptr<AnimaXValueAnimator> Create(
      std::shared_ptr<VSyncMonitor> vsync_monitor,
      std::weak_ptr<ValueAnimatorListener> listener);

  ~AnimaXValueAnimator();

  double GetStartFrame() { return start_frame_; }
  void SetStartFrame(const double start_frame);
  void SetEndFrame(const double end_frame);
  void SetLoop(const bool loop);
  void SetLoopCount(const int32_t loop_count);
  void SetAutoReverse(const bool auto_reverse);
  void SetSpeed(const double speed);
  void SetMaxFrameRate(const double max_frame_rate);
  void SetOriginFrameProperty(const double origin_start_frame,
                              const double origin_end_frame,
                              const double frame_rate);
  void SetPlaySegments(double start_frame, double end_frame);
  void Resume();
  void Start();
  void Pause();
  void Stop();
  void SeekToProgress(double progress, int32_t current_loop_count = 0);
  void SeekToFrame(double frame, int32_t current_loop_count = 0);

  void PlaySegment(double start_frame, double end_frame);

  void OnShow(VisibilityState state);
  void OnHide(VisibilityState state);

  void SetAutoplay(const bool autoplay);

  bool IsAnimating();

 private:
  AnimaXValueAnimator();
  void Init(std::shared_ptr<VSyncMonitor> vsync_monitor,
            std::weak_ptr<ValueAnimatorListener> listener);

  bool IsVisible();
  void AddVisibilityState(VisibilityState state);
  void RemoveVisibilityState(VisibilityState state);

  // specific thread only
  std::shared_ptr<ValueAnimator> impl_;
  bool loop_ = false;
  int32_t loop_count_ = 1;
  double start_frame_ = 0.0;
  double end_frame_ = -1.0;

  // The animator is playing when called OnHide
  bool playing_when_hide_ = false;
  // The animation is auto play after resource was loaded
  bool auto_play_ = true;
  // The animator has called Start success or not
  bool has_on_start_play_ = false;
  // Flag that record multiple different state, is visible if equals to 0
  uint8_t visibility_state_flag_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATOR_ANIMAX_VALUE_ANIMATOR_H_
