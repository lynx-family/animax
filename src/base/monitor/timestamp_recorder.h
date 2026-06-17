// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_MONITOR_TIMESTAMP_RECORDER_H_
#define ANIMAX_SRC_BASE_MONITOR_TIMESTAMP_RECORDER_H_

#include <array>
#include <memory>

#include "src/base/monitor/animax_metrics_listener.h"
#include "src/base/monitor/trace_event.h"

namespace lynx {
namespace animax {

using Timestamp = int64_t;
using TimestampArray =
    std::array<Timestamp, static_cast<size_t>(TraceEventType::kEnumCount)>;
static constexpr double kFrameInterval = 1000.f / 60.f;

class TimestampRecorder {
 public:
  TimestampRecorder() = default;
  virtual ~TimestampRecorder() = default;

  virtual void Trace(TraceEventType type);
  TimestampArray Export() const;
  TimestampArray& GetRefToTimestamps() { return timestamps_; }
  const TimestampArray& GetRefToTimestamps() const { return timestamps_; }

 protected:
  TimestampArray timestamps_{};
};

class PeriodicalTimestampRecorder : public TimestampRecorder {
 public:
  PeriodicalTimestampRecorder() = default;

  void Trace(TraceEventType type) override;

  float GetAverageFrameTime() const;
  Timestamp GetMaxFrameTime() const;
  float GetFPS() const;
  void SetFPSListener(std::weak_ptr<FPSListener> listener);
  void SetFPSReportThreshold(long threshold);
  void OnPlaybackStart();
  void OnPlaybackStop();

 private:
  void TraceFirstFrame(TraceEventType type);
  void ReportFPSAndMaxDroppedFrames();
  void UpdateInternalStateIfNeeded();
  float GetSessionFPS() const;
  void ResetSession();
  Timestamp GetActivePlaybackDuration() const;
  Timestamp GetSessionActivePlaybackDuration() const;
  void AccumulatePlaybackDuration(Timestamp now);

  Timestamp last_render_frame_start_{0};
  Timestamp last_render_frame_end_{0};

  long session_report_threshold_{0};
  Timestamp session_playback_duration_{0};
  Timestamp session_playback_start_{0};
  uint32_t session_max_dropped_frames_{0};
  uint32_t num_of_session_frames_{0};

  uint32_t num_of_all_frames_{0};
  Timestamp playback_duration_{0};
  Timestamp playback_start_{0};
  bool is_playback_active_{false};

  float avg_frame_time_{0.f};
  Timestamp max_frame_time_{0};

  uint32_t max_dropped_frames_{0};

  std::weak_ptr<FPSListener> listener_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_MONITOR_TIMESTAMP_RECORDER_H_
