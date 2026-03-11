// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/audio_controller.h"

#include "gtest/gtest.h"
#include "src/audio/audio_player.h"

using namespace lynx::animax;

class TestPlayer : public AudioPlayer {
 public:
  double current_progress = 0;
  double GetCurrentTime() override { return current_progress * duration; }

 private:
  double duration = 1000;
  void Resume() override { current_progress += 0.1; }
  void SeekToProgress(double progress) override { current_progress = progress; }
  double GetDuration() override { return duration; }
};

TEST(AudioControllerUnittest, PlayAndPause) {
  auto player = std::make_unique<TestPlayer>();
  auto test = player.get();
  auto controller = new AudioController(std::move(player));
  controller->Init("Test");
  controller->SetVolume(1);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 0);
  controller->OnProgress(0);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 0);
  controller->OnResume();
  controller->OnProgress(0);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 100);
  controller->OnProgress(0.2);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 200);
  controller->OnPause();
  controller->OnProgress(0.3);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 200);
}

TEST(AudioControllerUnittest, SyncProgress) {
  auto player = std::make_unique<TestPlayer>();
  auto test = player.get();
  auto controller = new AudioController(std::move(player));
  controller->Init("Test");
  controller->SetVolume(1);
  controller->OnResume();
  controller->OnProgress(0);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 100);
  controller->OnProgress(0.7);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 700);
  controller->OnPause();
  controller->OnProgress(0.2);
  EXPECT_DOUBLE_EQ(test->GetCurrentTime(), 700);
}
