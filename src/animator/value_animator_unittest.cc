// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/animator/value_animator.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "include/player/vsync_monitor.h"

namespace lynx {
namespace animax {
namespace {

constexpr int64_t kNanosecondsPerSecond = 1000000000;

int64_t FrameTimeNs(int64_t frame_index, int32_t refresh_rate) {
  return (frame_index * kNanosecondsPerSecond + refresh_rate / 2) /
         refresh_rate;
}

class FakeVSyncMonitor final : public VSyncMonitor {
 public:
  void AsyncRequestVSync(Callback callback) override {
    if (pending_callback_) {
      ADD_FAILURE() << "VSync callback already pending";
      return;
    }
    pending_callback_ = std::move(callback);
  }

  void Fire(int64_t timestamp_ns) {
    if (!pending_callback_) {
      ADD_FAILURE() << "No pending VSync callback";
      return;
    }
    auto callback = std::move(pending_callback_);
    callback(timestamp_ns);
  }

  bool HasPendingCallback() const {
    return static_cast<bool>(pending_callback_);
  }

 private:
  Callback pending_callback_;
};

class RecordingValueAnimatorListener final : public ValueAnimatorListener {
 public:
  void OnStart() override {}
  void OnResume() override {}
  void OnPause() override {}
  void OnProgress(double, double) override { ++progress_count_; }
  void OnNewLoop(int32_t) override {}
  void OnEnd() override {}
  void OnCancel() override {}
  void OnWarning(EventWarning, const std::string&) override {}
  void OnRerender() override {}

  size_t progress_count() const { return progress_count_; }

 private:
  size_t progress_count_ = 0;
};

class ValueAnimatorTest : public testing::Test {
 protected:
  void SetUp() override {
    vsync_monitor_ = std::make_shared<FakeVSyncMonitor>();
    listener_ = std::make_shared<RecordingValueAnimatorListener>();
    animator_ = ValueAnimator::Create(vsync_monitor_, listener_);
    animator_->SetOriginFrameProperty(0.0, 1000.0, 30.0);
  }

  size_t FireVSyncs(int64_t first_timestamp_ns, int32_t callback_count,
                    int32_t refresh_rate) {
    const size_t initial_progress_count = listener_->progress_count();
    for (int32_t index = 0; index < callback_count; ++index) {
      vsync_monitor_->Fire(first_timestamp_ns +
                           FrameTimeNs(index, refresh_rate));
    }
    return listener_->progress_count() - initial_progress_count;
  }

  std::shared_ptr<FakeVSyncMonitor> vsync_monitor_;
  std::shared_ptr<RecordingValueAnimatorListener> listener_;
  std::shared_ptr<ValueAnimator> animator_;
};

TEST_F(ValueAnimatorTest, LimitsSteady120HzVSyncTo30Fps) {
  animator_->SetMaxFrameRate(30.0);
  animator_->Start();

  EXPECT_EQ(8u, FireVSyncs(0, 31, 120));
}

TEST_F(ValueAnimatorTest, Accepts60HzVSyncAt60FpsDeadline) {
  animator_->SetMaxFrameRate(60.0);
  animator_->Start();

  EXPECT_EQ(61u, FireVSyncs(0, 61, 60));
}

TEST_F(ValueAnimatorTest, Accepts60HzVSyncAt60FpsAfterFourDaysUptime) {
  animator_->SetMaxFrameRate(60.0);
  animator_->Start();

  constexpr int64_t kFourDaysNs = 4 * 24 * 60 * 60 * kNanosecondsPerSecond;
  EXPECT_EQ(61u, FireVSyncs(kFourDaysNs, 61, 60));
}

TEST_F(ValueAnimatorTest, AdvancesDeadlinePastAcceptedLongRunningVSync) {
  animator_->SetMaxFrameRate(60.0);
  animator_->SetLoopCount(0);
  animator_->Start();
  vsync_monitor_->Fire(0);
  ASSERT_EQ(1u, listener_->progress_count());

  constexpr int64_t kPreviousLongRunningVSyncNs = 569855616666666;
  vsync_monitor_->Fire(kPreviousLongRunningVSyncNs);
  ASSERT_EQ(2u, listener_->progress_count());

  constexpr int64_t kLongRunningVSyncNs = 569855633333332;
  vsync_monitor_->Fire(kLongRunningVSyncNs);
  ASSERT_EQ(3u, listener_->progress_count());

  vsync_monitor_->Fire(kLongRunningVSyncNs + FrameTimeNs(1, 120));
  EXPECT_EQ(3u, listener_->progress_count());
}

TEST_F(ValueAnimatorTest, ResumeAfterOneSecondPauseKeeps30FpsLimit) {
  animator_->SetMaxFrameRate(30.0);
  animator_->Start();
  FireVSyncs(0, 7, 120);

  animator_->Pause();
  const int64_t pause_callback_time_ns = FrameTimeNs(7, 120);
  vsync_monitor_->Fire(pause_callback_time_ns);
  EXPECT_FALSE(vsync_monitor_->HasPendingCallback());

  animator_->Resume();
  const int64_t first_resumed_vsync_ns =
      pause_callback_time_ns + kNanosecondsPerSecond;
  EXPECT_EQ(8u, FireVSyncs(first_resumed_vsync_ns, 31, 120));
}

TEST_F(ValueAnimatorTest, DelayedCallbackDoesNotCatchUpMissedFrames) {
  animator_->SetMaxFrameRate(30.0);
  animator_->Start();
  FireVSyncs(0, 7, 120);

  const int64_t delayed_vsync_ns = FrameTimeNs(6, 120) + kNanosecondsPerSecond;
  vsync_monitor_->Fire(delayed_vsync_ns);

  const int64_t next_vsync_ns = delayed_vsync_ns + FrameTimeNs(1, 120);
  EXPECT_EQ(8u, FireVSyncs(next_vsync_ns, 30, 120));
}

TEST_F(ValueAnimatorTest, ChangingMaxFrameRateRebuildsDeadline) {
  animator_->SetMaxFrameRate(30.0);
  animator_->Start();
  vsync_monitor_->Fire(0);
  ASSERT_EQ(1u, listener_->progress_count());

  animator_->SetMaxFrameRate(60.0);
  vsync_monitor_->Fire(FrameTimeNs(1, 120));
  EXPECT_EQ(1u, listener_->progress_count());

  vsync_monitor_->Fire(FrameTimeNs(3, 120));
  EXPECT_EQ(2u, listener_->progress_count());
}

}  // namespace
}  // namespace animax
}  // namespace lynx
