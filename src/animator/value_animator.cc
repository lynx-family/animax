// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/animator/value_animator.h"

#include <cmath>
#include <limits>

#include "include/player/vsync_monitor.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/base/util/event_warning_checker.h"

namespace lynx {
namespace animax {

namespace {
constexpr double kVSyncTimestampToleranceNs = 1.0;
}  // namespace

std::shared_ptr<ValueAnimator> ValueAnimator::Create(
    std::shared_ptr<VSyncMonitor> vsync_monitor,
    std::weak_ptr<ValueAnimatorListener> listener) {
  auto value_animator = std::shared_ptr<ValueAnimator>(
      new ValueAnimator(std::move(vsync_monitor), std::move(listener)));
  return value_animator;
}

ValueAnimator::ValueAnimator(std::shared_ptr<VSyncMonitor> vsync_monitor,
                             std::weak_ptr<ValueAnimatorListener> listener)
    : vsync_monitor_(std::move(vsync_monitor)),
      listener_(std::move(listener)) {}

void ValueAnimator::SetupOnFrame() {
  vsync_monitor_->AsyncRequestVSync(
      [weak_value_animator = weak_from_this()](int64_t frame_start) {
        auto value_animator = weak_value_animator.lock();
        if (!value_animator || !value_animator->IsAnimating()) {
          return;
        }
        value_animator->SetupOnFrame();
        value_animator->OnFrame(frame_start);
      });
}

void ValueAnimator::OnFrame(int64_t current_time_ns) {
  const double current_time_ms = static_cast<double>(current_time_ns) / 1e6;
  bool start = false;
  bool end = false;
  bool new_loop = false;
  if (last_time_ms_ == -1.0) {
    frame_time_anchor_ns_ = current_time_ns;
    next_frame_ns_ = fps_interval_ns_;
    if (!has_on_start_emit_) {
      has_on_start_emit_ = true;
      start = true;
    }
  } else {
    if (!UpdateNextFrameNs(current_time_ns)) {
      // Update failed, means the next frame haven't reach yet, return directly
      return;
    }

    double time_elapse_ms = current_time_ms - last_time_ms_;
    current_frame_ += speed_ * time_elapse_ms * frame_rate_ / 1000.0;
  }
  last_time_ms_ = current_time_ms;

  if (current_frame_ > end_frame_) {
    if (!IsLoopForever() && current_loop_count_ >= loop_count_ - 1) {
      current_frame_ = end_frame_;
      end = true;
    } else {
      ++current_loop_count_;
      new_loop = true;
      current_frame_ = start_frame_ + std::fmod(current_frame_ - start_frame_,
                                                end_frame_ - start_frame_);
    }
  } else if (current_frame_ < start_frame_) {
    if (!IsLoopForever() && current_loop_count_ <= 0) {
      current_frame_ = start_frame_;
      end = true;
    } else {
      --current_loop_count_;
      new_loop = true;
      current_frame_ = end_frame_ + std::fmod(current_frame_ - start_frame_,
                                              end_frame_ - start_frame_);
    }
  }

  auto listener = listener_.lock();
  bool has_listener = listener.get();
  if (start && has_listener) {
    listener->OnStart();
  }
  if (new_loop && has_listener) {
    listener->OnNewLoop(current_loop_count_);
  }
  if (has_listener) {
    double progress = GetProgress();
    listener->OnProgress(progress, current_frame_);
  }
  if (end) {
    ResetState();
    if (has_listener) {
      listener->OnEnd();
    }
  }
}

bool ValueAnimator::UpdateNextFrameNs(int64_t current_time_ns) {
  if (next_frame_ns_ == 0.0) {
    // SetMaxFrameRate invalidates the phase; rebuild it without rendering.
    frame_time_anchor_ns_ = current_time_ns;
    next_frame_ns_ = fps_interval_ns_;
    return false;
  }

  // Subtract as integers first so device uptime does not reduce precision.
  const double elapsed_ns =
      static_cast<double>(current_time_ns - frame_time_anchor_ns_);
  // A rounded integer VSync can land just below a fractional deadline. The
  // fixed tolerance and one ULP per operand keep that from dropping a frame.
  const double elapsed_ulp_ns =
      std::nextafter(elapsed_ns, std::numeric_limits<double>::infinity()) -
      elapsed_ns;
  const double deadline_ulp_ns =
      std::nextafter(next_frame_ns_, std::numeric_limits<double>::infinity()) -
      next_frame_ns_;
  const double adjusted_elapsed_ns = elapsed_ns + kVSyncTimestampToleranceNs +
                                     elapsed_ulp_ns + deadline_ulp_ns;
  if (adjusted_elapsed_ns >= next_frame_ns_) {
    // Skip every expired slot in one step while preserving the cadence phase.
    const double elapsed_intervals =
        std::floor(adjusted_elapsed_ns / fps_interval_ns_);
    next_frame_ns_ = (elapsed_intervals + 1.0) * fps_interval_ns_;
    // Division may round back to the current slot; always advance past it.
    if (next_frame_ns_ <= adjusted_elapsed_ns) {
      next_frame_ns_ += fps_interval_ns_;
    }
    return true;
  }
  return false;
}

void ValueAnimator::Rerender() {
  if (auto listener = listener_.lock()) {
    listener->OnRerender();
  }
}

double ValueAnimator::GetProgress() const {
  return (auto_reverse_ && (current_loop_count_ & 1))
             ? (end_frame_ - (current_frame_ - start_frame_)) /
                   (origin_end_frame_ - origin_start_frame_)
             : (current_frame_ - origin_start_frame_) /
                   (origin_end_frame_ - origin_start_frame_);
}

bool ValueAnimator::IsLoopForever() { return loop_count_ == 0.0; }

void ValueAnimator::ResetState() {
  state_ = State::kUnknown;
  last_time_ms_ = -1.0;
  current_loop_count_ = 0;
  current_frame_ = 0.0;
  has_on_start_emit_ = false;
}

bool ValueAnimator::IsAnimating() { return State::kPlaying == state_; }

bool ValueAnimator::CanPlay() {
  return origin_end_frame_ > origin_start_frame_ && frame_rate_ > 0;
}

void ValueAnimator::SetPlaySegments(double start_frame, double end_frame) {
  if (!CanPlay()) {
    ANIMAX_LOGE(
        "ValueAnimator: Can't SetPlaySegments before SetOriginFrameProperty");
    return;
  }

  if (start_frame < origin_start_frame_) {
    start_frame = origin_start_frame_;
  } else if (start_frame > origin_end_frame_) {
    start_frame = origin_end_frame_;
  }

  if (end_frame == -1.0 || end_frame > origin_end_frame_) {
    end_frame = origin_end_frame_;
  } else if (end_frame < origin_start_frame_) {
    end_frame = origin_start_frame_;
  }

  ANIMAX_LOGI("SetPlaySegments start_frame:" << start_frame
                                             << ", end_frame:" << end_frame);

  start_frame_ = start_frame;
  end_frame_ = end_frame;
}

void ValueAnimator::Start() {
  Resume();
  if (IsAnimating()) {
    has_on_start_emit_ = false;
  }
}

void ValueAnimator::Resume() {
  if (!CanPlay()) {
    return;
  }
  auto listener = listener_.lock();
  EventWarningChecker::CheckStartAndEndFrame(
      start_frame_, end_frame_, current_frame_,
      [listener](EventWarning warning, const std::string& warning_msg) {
        if (listener) {
          listener->OnWarning(warning, warning_msg);
        }
      });
  if (start_frame_ >= end_frame_) {
    return;
  }
  MakeCurrentFrameValid();

  state_ = State::kPlaying;
  if (listener) {
    listener->OnResume();
  }
  last_time_ms_ = -1.0;
  SetupOnFrame();
}

void ValueAnimator::Pause() {
  if (State::kPlaying == state_) {
    state_ = State::kPaused;
  }
  if (auto listener = listener_.lock()) {
    listener->OnPause();
  }
}

void ValueAnimator::Stop() {
  const bool need_cancel_event = IsAnimating();
  ResetState();
  if (need_cancel_event) {
    if (auto listener = listener_.lock()) {
      listener->OnCancel();
    }
  }
}

void ValueAnimator::SeekToProgress(double progress,
                                   int32_t current_loop_count) {
  SeekToFrame(progress * (origin_end_frame_ - origin_start_frame_),
              current_loop_count);
}

void ValueAnimator::SeekToFrame(double frame, int32_t current_loop_count) {
  if (!CanPlay()) {
    ANIMAX_LOGE("ValueAnimator: Seek before CanPlay");
    return;
  }
  auto listener = listener_.lock();
  EventWarningChecker::CheckStartAndEndFrame(
      start_frame_, end_frame_, current_frame_,
      [listener](EventWarning warning, const std::string& warning_msg) {
        if (listener) {
          listener->OnWarning(warning, warning_msg);
        }
      });
  if (start_frame_ >= end_frame_) {
    ANIMAX_LOGE("ValueAnimator: Invalid start_frame_ and end_frame_ in Seek");
    return;
  }
  current_loop_count_ = current_loop_count;
  current_frame_ = origin_start_frame_ + frame;
  MakeCurrentFrameValid();

  if (listener) {
    listener->OnProgress(GetProgress(), current_frame_);
  }
}

void ValueAnimator::MakeCurrentFrameValid() {
  DCHECK(start_frame_ < end_frame_);
  if (current_frame_ < start_frame_) {
    current_frame_ = start_frame_;
  } else if (current_frame_ > end_frame_) {
    current_frame_ = end_frame_;
  }
}

// TEST: 0, 1, 2
void ValueAnimator::SetLoopCount(const int32_t loop_count) {
  if (loop_count < 0) {
    ANIMAX_LOGW("ValueAnimator::SetLoopCount less than 0: " << loop_count);
    return;
  }
  loop_count_ = loop_count;
}

// TEST: true+loopCont=1, true+loopCont=2
void ValueAnimator::SetAutoReverse(const bool auto_reverse) {
  auto_reverse_ = auto_reverse;
}

// TEST: 0.5, 2
void ValueAnimator::SetSpeed(const double speed) { speed_ = speed; }

// progress
// TEST: 0.5, 1, 0.5+speed=0, 0.5+speed=-1

// start-frame, end-frame
// TEST: 0,-1
// TEST: 15,45
// TEST: 15,45,loop
// TEST: 15,45,loop,progress=0.5
// TEST: 15,45,loop,progress=0.5,auto-reverse

void ValueAnimator::SetOriginFrameProperty(const double origin_start_frame,
                                           const double origin_end_frame,
                                           const double frame_rate) {
  origin_start_frame_ = origin_start_frame;
  origin_end_frame_ = origin_end_frame;
  frame_rate_ = frame_rate;

  SetPlaySegments(origin_start_frame, origin_end_frame);
}

void ValueAnimator::SetMaxFrameRate(const double max_frame_rate) {
  fps_interval_ns_ = kSecondNs / max_frame_rate;
  next_frame_ns_ = 0.f;
}

}  // namespace animax
}  // namespace lynx
