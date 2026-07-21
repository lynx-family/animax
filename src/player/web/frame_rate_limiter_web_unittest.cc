// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/web/frame_rate_limiter_web.h"

#include <array>
#include <cstdint>

#include "gtest/gtest.h"

namespace lynx {
namespace animax {
namespace {

constexpr int64_t kNanosecondsPerSecond = 1000000000;

int64_t FrameTimeNs(int64_t frame_index, int32_t refresh_rate) {
  return (frame_index * kNanosecondsPerSecond + refresh_rate / 2) /
         refresh_rate;
}

TEST(FrameRateLimiterWebTest, LimitsSteady120HzVSyncTo30Fps) {
  FrameRateLimiterWeb limiter(30.0);
  size_t dispatch_count = 0;
  for (int32_t index = 0; index < 31; ++index) {
    dispatch_count += limiter.ShouldDispatch(FrameTimeNs(index, 120));
  }
  EXPECT_EQ(8u, dispatch_count);
}

TEST(FrameRateLimiterWebTest, Keeps60FpsAcrossJitteredHighRefreshVSync) {
  FrameRateLimiterWeb limiter(60.0);
  constexpr std::array<int64_t, 10> kJitteredFrameIntervalsNs = {
      8300000,  8300000, 9200000, 7500000, 8300000,
      25000000, 8300000, 8300000, 8300000, 8300000};
  int64_t timestamp_ns = 0;
  size_t dispatch_count = limiter.ShouldDispatch(timestamp_ns);
  for (int repeat = 0; repeat < 100; ++repeat) {
    for (const int64_t interval_ns : kJitteredFrameIntervalsNs) {
      timestamp_ns += interval_ns;
      dispatch_count += limiter.ShouldDispatch(timestamp_ns);
    }
  }
  EXPECT_GE(dispatch_count, 597u);
  EXPECT_LE(dispatch_count, 600u);
}

TEST(FrameRateLimiterWebTest, LimitsDelayedRecoveryToOneExtraFrame) {
  FrameRateLimiterWeb limiter(30.0);
  size_t dispatch_count = 0;
  for (int32_t index = 0; index < 5; ++index) {
    dispatch_count += limiter.ShouldDispatch(FrameTimeNs(index, 120));
  }
  EXPECT_EQ(2u, dispatch_count);

  const int64_t delayed_vsync_ns = FrameTimeNs(4, 120) + kNanosecondsPerSecond;
  dispatch_count += limiter.ShouldDispatch(delayed_vsync_ns);
  EXPECT_EQ(3u, dispatch_count);

  dispatch_count +=
      limiter.ShouldDispatch(delayed_vsync_ns + FrameTimeNs(1, 120));
  EXPECT_EQ(4u, dispatch_count);

  dispatch_count +=
      limiter.ShouldDispatch(delayed_vsync_ns + FrameTimeNs(2, 120));
  EXPECT_EQ(4u, dispatch_count);
}

TEST(FrameRateLimiterWebTest, RateChangeResetsAccumulatedCredit) {
  FrameRateLimiterWeb limiter(30.0);
  EXPECT_TRUE(limiter.ShouldDispatch(0));
  EXPECT_FALSE(limiter.ShouldDispatch(FrameTimeNs(1, 120)));

  limiter.SetMaxFrameRate(60.0);
  EXPECT_FALSE(limiter.ShouldDispatch(FrameTimeNs(2, 120)));
  EXPECT_TRUE(limiter.ShouldDispatch(FrameTimeNs(4, 120)));
}

TEST(FrameRateLimiterWebTest, RateChangesPreserveOnlyInitialCredit) {
  FrameRateLimiterWeb initial_limiter(60.0);
  initial_limiter.SetMaxFrameRate(30.0);
  EXPECT_TRUE(initial_limiter.ShouldDispatch(0));

  FrameRateLimiterWeb running_limiter(30.0);
  EXPECT_TRUE(running_limiter.ShouldDispatch(0));
  running_limiter.SetMaxFrameRate(60.0);
  running_limiter.SetMaxFrameRate(30.0);
  EXPECT_FALSE(running_limiter.ShouldDispatch(FrameTimeNs(1, 120)));
  EXPECT_TRUE(running_limiter.ShouldDispatch(FrameTimeNs(5, 120)));
}

TEST(FrameRateLimiterWebTest, KeepsCadenceAtLongRunningTimestamp) {
  FrameRateLimiterWeb limiter(120.0);
  constexpr int64_t kLongRunningTimestampNs = 10000000000000000LL;
  EXPECT_TRUE(limiter.ShouldDispatch(kLongRunningTimestampNs));
  EXPECT_TRUE(
      limiter.ShouldDispatch(kLongRunningTimestampNs + FrameTimeNs(1, 120)));
}

TEST(FrameRateLimiterWebTest, BackwardTimestampDoesNotRefillBucket) {
  FrameRateLimiterWeb limiter(60.0);
  EXPECT_TRUE(limiter.ShouldDispatch(100));
  EXPECT_FALSE(limiter.ShouldDispatch(99));
}

}  // namespace
}  // namespace animax
}  // namespace lynx
