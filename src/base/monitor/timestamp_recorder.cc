// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/monitor/timestamp_recorder.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"

namespace lynx {
namespace animax {

namespace {
Timestamp Current() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
}  // namespace

void TimestampRecorder::Trace(TraceEventType type) {
  DCHECK(type < TraceEventType::kEnumCount);
  timestamps_[static_cast<uint8_t>(type)] = Current();
}

TimestampArray TimestampRecorder::Export() const { return timestamps_; }

void PeriodicalTimestampRecorder::TraceFirstFrame(TraceEventType type) {
  if (type == TraceEventType::kRenderFrameStart &&
      timestamps_[static_cast<uint8_t>(type)] == 0) {
    TimestampRecorder::Trace(type);
    ResetSession();
  } else if (type == TraceEventType::kRenderFrameEnd &&
             timestamps_[static_cast<uint8_t>(type)] == 0) {
    TimestampRecorder::Trace(type);
  }
}

void PeriodicalTimestampRecorder::ResetSession() {
  session_playback_duration_ = 0;
  session_playback_start_ = is_playback_active_ ? Current() : 0;
  num_of_session_frames_ = 0;
  session_max_dropped_frames_ = 0;
}

void PeriodicalTimestampRecorder::AccumulatePlaybackDuration(Timestamp now) {
  if (!is_playback_active_ || playback_start_ <= 0 || now <= playback_start_) {
    return;
  }
  auto duration = now - playback_start_;
  playback_duration_ += duration;
  playback_start_ = now;
  if (session_playback_start_ > 0 && now > session_playback_start_) {
    session_playback_duration_ += now - session_playback_start_;
    session_playback_start_ = now;
  }
}

Timestamp PeriodicalTimestampRecorder::GetActivePlaybackDuration() const {
  auto duration = playback_duration_;
  if (is_playback_active_ && playback_start_ > 0) {
    duration += Current() - playback_start_;
  }
  return duration;
}

Timestamp PeriodicalTimestampRecorder::GetSessionActivePlaybackDuration()
    const {
  auto duration = session_playback_duration_;
  if (is_playback_active_ && session_playback_start_ > 0) {
    duration += Current() - session_playback_start_;
  }
  return duration;
}

void PeriodicalTimestampRecorder::ReportFPSAndMaxDroppedFrames() {
  auto time_since_last_report = GetSessionActivePlaybackDuration();

  if (time_since_last_report > session_report_threshold_) {
    auto fps = GetSessionFPS();

    auto listener_locked = listener_.lock();
    if (listener_locked) {
      listener_locked->OnFps(fps, session_max_dropped_frames_);
    }

    ResetSession();
  }
}

void PeriodicalTimestampRecorder::UpdateInternalStateIfNeeded() {
  auto frame_time = last_render_frame_end_ - last_render_frame_start_;

  // Global-level Update
  max_frame_time_ = std::max(frame_time, max_frame_time_);
  avg_frame_time_ = (avg_frame_time_ * num_of_all_frames_ + frame_time) /
                    (num_of_all_frames_ + 1);
  auto dropped_frames = static_cast<uint32_t>(frame_time / kFrameInterval);
  max_dropped_frames_ = std::max(dropped_frames, max_dropped_frames_);
  num_of_all_frames_++;

  // Session-level Update
  session_max_dropped_frames_ =
      std::max(session_max_dropped_frames_, dropped_frames);
  num_of_session_frames_++;
}

void PeriodicalTimestampRecorder::Trace(TraceEventType type) {
  TraceFirstFrame(type);

  if (type == TraceEventType::kRenderFrameStart) {
    last_render_frame_start_ = Current();
  } else if (type == TraceEventType::kRenderFrameEnd) {
    last_render_frame_end_ = Current();
    UpdateInternalStateIfNeeded();
    if (session_report_threshold_ > 0) {
      ReportFPSAndMaxDroppedFrames();
    }
  }
}

float PeriodicalTimestampRecorder::GetFPS() const {
  auto active_playback_duration = GetActivePlaybackDuration();

  if (active_playback_duration <= 0 || num_of_all_frames_ == 0) {
    return 0.0f;
  }
  return round((num_of_all_frames_ * 1000.0 / active_playback_duration) * 100) /
         100.f;
}

float PeriodicalTimestampRecorder::GetSessionFPS() const {
  auto active_session_duration = GetSessionActivePlaybackDuration();

  if (active_session_duration <= 0 || num_of_session_frames_ == 0) {
    return 0.0f;
  }
  return round((num_of_session_frames_ * 1000.0 / active_session_duration) *
               100) /
         100.f;
}

float PeriodicalTimestampRecorder::GetAverageFrameTime() const {
  return avg_frame_time_;
}
Timestamp PeriodicalTimestampRecorder::GetMaxFrameTime() const {
  return max_frame_time_;
}

void PeriodicalTimestampRecorder::SetFPSListener(
    std::weak_ptr<FPSListener> listener) {
  listener_ = listener;
}

void PeriodicalTimestampRecorder::SetFPSReportThreshold(long threshold) {
  session_report_threshold_ = threshold;
}

void PeriodicalTimestampRecorder::OnPlaybackStart() {
  if (is_playback_active_) {
    return;
  }
  is_playback_active_ = true;
  playback_start_ = Current();
  session_playback_start_ = playback_start_;
}

void PeriodicalTimestampRecorder::OnPlaybackStop() {
  if (!is_playback_active_) {
    return;
  }
  AccumulatePlaybackDuration(Current());
  is_playback_active_ = false;
  playback_start_ = 0;
  session_playback_start_ = 0;
}

}  // namespace animax
}  // namespace lynx
