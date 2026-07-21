// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WEB_FRAME_RATE_LIMITER_WEB_H_
#define ANIMAX_SRC_PLAYER_WEB_FRAME_RATE_LIMITER_WEB_H_

#include <algorithm>
#include <cstdint>

namespace lynx {
namespace animax {

// A Web-only token bucket that absorbs one missed RAF without changing its
// timestamp. Keeping two frame intervals of credit allows one bounded recovery
// callback while preventing an unbounded catch-up burst.
class FrameRateLimiterWeb {
 public:
  explicit FrameRateLimiterWeb(double max_frame_rate = 60.0) {
    SetMaxFrameRate(max_frame_rate);
  }

  void SetMaxFrameRate(double max_frame_rate) {
    if (max_frame_rate <= 0.0) {
      return;
    }
    frame_interval_ns_ = kNanosecondsPerSecond / max_frame_rate;
    available_time_ns_ = has_seen_timestamp_ ? 0.0 : frame_interval_ns_;
    last_timestamp_ns_ = 0;
    initialized_ = false;
  }

  bool ShouldDispatch(int64_t timestamp_ns) {
    if (!initialized_) {
      initialized_ = true;
      has_seen_timestamp_ = true;
      last_timestamp_ns_ = timestamp_ns;
    } else if (timestamp_ns > last_timestamp_ns_) {
      // Subtract before converting to double to preserve long-uptime precision.
      const uint64_t elapsed_ns = static_cast<uint64_t>(timestamp_ns) -
                                  static_cast<uint64_t>(last_timestamp_ns_);
      available_time_ns_ =
          std::min(kBucketCapacity * frame_interval_ns_,
                   available_time_ns_ + static_cast<double>(elapsed_ns));
      last_timestamp_ns_ = timestamp_ns;
    }

    if (available_time_ns_ + kTimestampToleranceNs < frame_interval_ns_) {
      return false;
    }
    available_time_ns_ = std::max(0.0, available_time_ns_ - frame_interval_ns_);
    return true;
  }

 private:
  static constexpr double kNanosecondsPerSecond = 1000000000.0;
  static constexpr double kBucketCapacity = 2.0;
  static constexpr double kTimestampToleranceNs = 1.0;

  double frame_interval_ns_ = kNanosecondsPerSecond / 60.0;
  double available_time_ns_ = frame_interval_ns_;
  int64_t last_timestamp_ns_ = 0;
  bool initialized_ = false;
  bool has_seen_timestamp_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WEB_FRAME_RATE_LIMITER_WEB_H_
