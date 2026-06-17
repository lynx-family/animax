// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/monitor/timestamp_recorder.h"

#include <chrono>
#include <thread>

#include "gtest/gtest.h"

using namespace lynx::animax;

class PeriodicalTimeStampRecorderTest : public ::testing::Test {
 protected:
  std::shared_ptr<PeriodicalTimestampRecorder> timestamp_recorder_ =
      std::make_shared<PeriodicalTimestampRecorder>();
};

TEST_F(PeriodicalTimeStampRecorderTest, SingleEvent) {
  timestamp_recorder_->OnPlaybackStart();
  timestamp_recorder_->Trace(TraceEventType::kRenderFrameStart);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  timestamp_recorder_->Trace(TraceEventType::kRenderFrameEnd);
  timestamp_recorder_->OnPlaybackStop();

  EXPECT_GT(timestamp_recorder_->GetFPS(), 0);
  EXPECT_GT(timestamp_recorder_->GetMaxFrameTime(), 0);
  EXPECT_GT(timestamp_recorder_->GetAverageFrameTime(), 0);
}

TEST_F(PeriodicalTimeStampRecorderTest, FPSIgnoresIdleTimeAfterPlaybackStops) {
  timestamp_recorder_->OnPlaybackStart();
  timestamp_recorder_->Trace(TraceEventType::kRenderFrameStart);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  timestamp_recorder_->Trace(TraceEventType::kRenderFrameEnd);
  timestamp_recorder_->OnPlaybackStop();

  auto fps_before_idle = timestamp_recorder_->GetFPS();
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  auto fps_after_idle = timestamp_recorder_->GetFPS();

  EXPECT_GT(fps_before_idle, 0);
  EXPECT_EQ(fps_after_idle, fps_before_idle);
}
