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
  session_render_frame_start_ = Current();
  num_of_session_frames_ = 0;
  session_max_dropped_frames_ = 0;
}

void PeriodicalTimestampRecorder::ReportFPSAndMaxDroppedFrames() {
  auto time_since_last_report =
      last_render_frame_end_ - session_render_frame_start_;

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
  auto first_render_frame_start =
      timestamps_[static_cast<uint8_t>(TraceEventType::kRenderFrameStart)];
  auto time_since_animation_started =
      last_render_frame_end_ - first_render_frame_start;

  if (time_since_animation_started <= 0 || first_render_frame_start == 0) {
    return 0.0f;
  }
  return round((num_of_all_frames_ * 1000.0 / time_since_animation_started) *
               100) /
         100.f;
}

float PeriodicalTimestampRecorder::GetSessionFPS() const {
  auto time_since_last_session_started =
      last_render_frame_end_ - session_render_frame_start_;

  if (time_since_last_session_started <= 0 ||
      session_render_frame_start_ == 0) {
    return 0.0f;
  }
  return round((num_of_session_frames_ * 1000.0 /
                time_since_last_session_started) *
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

}  // namespace animax
}  // namespace lynx
