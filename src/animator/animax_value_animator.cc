// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/animator/animax_value_animator.h"

#include "include/player/vsync_monitor.h"
#include "src/animator/value_animator.h"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"
#include "src/base/util/visibility_state.h"

namespace lynx {
namespace animax {

std::shared_ptr<AnimaXValueAnimator> AnimaXValueAnimator::Create(
    std::shared_ptr<VSyncMonitor> vsync_monitor,
    std::weak_ptr<ValueAnimatorListener> listener) {
  auto value_animator =
      std::shared_ptr<AnimaXValueAnimator>(new AnimaXValueAnimator());
  value_animator->Init(std::move(vsync_monitor), std::move(listener));
  return value_animator;
}

AnimaXValueAnimator::AnimaXValueAnimator() = default;

AnimaXValueAnimator::~AnimaXValueAnimator() = default;

void AnimaXValueAnimator::Init(std::shared_ptr<VSyncMonitor> vsync_monitor,
                               std::weak_ptr<ValueAnimatorListener> listener) {
  // impl_ must be created synchronously during initialization.
  impl_ = ValueAnimator::Create(std::move(vsync_monitor), std::move(listener));
}

void AnimaXValueAnimator::SetStartFrame(const double start_frame) {
  start_frame_ = start_frame;
  impl_->SetPlaySegments(start_frame_, end_frame_);
}

void AnimaXValueAnimator::SetEndFrame(const double end_frame) {
  end_frame_ = end_frame;
  impl_->SetPlaySegments(start_frame_, end_frame_);
}

void AnimaXValueAnimator::SetLoop(const bool loop) {
  loop_ = loop;
  impl_->SetLoopCount(loop_ ? 0 : loop_count_);
}

void AnimaXValueAnimator::SetLoopCount(const int32_t loop_count) {
  loop_count_ = loop_count;
  impl_->SetLoopCount(loop_ ? 0 : loop_count_);
}

void AnimaXValueAnimator::SetAutoReverse(const bool auto_reverse) {
  impl_->SetAutoReverse(auto_reverse);
}

void AnimaXValueAnimator::SetSpeed(const double speed) {
  impl_->SetSpeed(speed);
}

void AnimaXValueAnimator::SetMaxFrameRate(const double max_frame_rate) {
  impl_->SetMaxFrameRate(max_frame_rate);
}

void AnimaXValueAnimator::SetOriginFrameProperty(
    const double origin_start_frame, const double origin_end_frame,
    const double frame_rate) {
  impl_->SetOriginFrameProperty(origin_start_frame, origin_end_frame,
                                frame_rate);
  impl_->SetPlaySegments(start_frame_, end_frame_);
}

void AnimaXValueAnimator::SetPlaySegments(double start_frame,
                                          double end_frame) {
  impl_->SetPlaySegments(start_frame, end_frame);
}

void AnimaXValueAnimator::PlaySegment(double start_frame, double end_frame) {
  // Default to stop current animation first
  impl_->Stop();
  // Update animation's playing start and end frame
  impl_->SetPlaySegments(start_frame, end_frame);
  // Play from start frame
  impl_->Resume();
}

void AnimaXValueAnimator::Resume() {
  if (IsVisible()) {
    impl_->Resume();
  } else {
    playing_when_hide_ = true;
    ANIMAX_LOGI("Resume fail, not visible");
  }
}

void AnimaXValueAnimator::Start() {
  if (IsVisible()) {
    has_on_start_play_ = true;
    impl_->Start();
  } else {
    playing_when_hide_ = true;
    ANIMAX_LOGI("Start fail, not visible");
  }
}

void AnimaXValueAnimator::Pause() {
  playing_when_hide_ = false;
  impl_->Pause();
}

void AnimaXValueAnimator::Stop() {
  playing_when_hide_ = false;
  impl_->Stop();
}

void AnimaXValueAnimator::SeekToProgress(double progress,
                                         int32_t current_loop_count) {
  impl_->SeekToProgress(progress, current_loop_count);
}

void AnimaXValueAnimator::SeekToFrame(double frame,
                                      int32_t current_loop_count) {
  impl_->SeekToFrame(frame, current_loop_count);
}

bool AnimaXValueAnimator::IsVisible() { return visibility_state_flag_ == 0; }

void AnimaXValueAnimator::AddVisibilityState(VisibilityState state) {
  visibility_state_flag_ |= static_cast<uint8_t>(state);
  ANIMAX_LOGI("AddVisibilityState state: "
              << StringifyVisibilityState(state) << ", CurrentFlag:"
              << StringifyVisibilityStateFlag(visibility_state_flag_));
}

void AnimaXValueAnimator::RemoveVisibilityState(VisibilityState state) {
  visibility_state_flag_ &= ~static_cast<uint8_t>(state);
  ANIMAX_LOGI("RemoveVisibilityState state: "
              << StringifyVisibilityState(state) << ", CurrentFlag: "
              << StringifyVisibilityStateFlag(visibility_state_flag_));
}

void AnimaXValueAnimator::OnShow(VisibilityState state) {
  // Visible, keep play status
  if (IsVisible()) {
    return;
  }

  RemoveVisibilityState(state);
  // Remove state and still not visible, keep play status
  if (!IsVisible()) {
    return;
  }

  if (auto_play_ && !has_on_start_play_) {
    // Need auto play and haven't do onStart play
    has_on_start_play_ = true;
    impl_->Start();
  } else if (playing_when_hide_) {
    // Need resume playing status on hide
    impl_->Resume();
  }

  // Reset the flag anyway
  playing_when_hide_ = false;
}

void AnimaXValueAnimator::OnHide(VisibilityState state) {
  auto was_visible = IsVisible();
  AddVisibilityState(state);
  if (was_visible) {
    if (impl_->IsAnimating()) {
      playing_when_hide_ = true;
    }
    impl_->Pause();
  }
}

void AnimaXValueAnimator::SetAutoplay(const bool autoplay) {
  auto_play_ = autoplay;
}

bool AnimaXValueAnimator::IsAnimating() { return impl_->IsAnimating(); }

}  // namespace animax
}  // namespace lynx
