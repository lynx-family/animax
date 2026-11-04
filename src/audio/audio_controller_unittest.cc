// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/audio_controller.h"

#include <chrono>
#include <limits>
#include <thread>

#include "gtest/gtest.h"
#include "src/audio/audio_player.h"
#include "src/resource/asset/audio_asset.h"

using namespace lynx::animax;

class TestPlayer : public AudioPlayer {
 public:
  explicit TestPlayer(double duration_ms = 1000) : duration(duration_ms) {}

  double current_progress = 0;
  int seek_count = 0;
  int resume_count = 0;
  int pause_count = 0;
  int duration_read_count = 0;
  bool is_playing = false;
  double GetAudioTime() override { return current_progress * duration; }
  void Advance(double elapsed_ms = 100) {
    if (is_playing) {
      current_progress =
          std::fmod(GetAudioTime() + elapsed_ms, duration) / duration;
    }
  }

 private:
  double duration;
  void Pause() override {
    is_playing = false;
    pause_count++;
  }
  void Resume() override {
    is_playing = true;
    resume_count++;
  }
  void SeekToProgress(double progress) override {
    // Android pauses and flushes on seek; the controller must resume afterward.
    is_playing = false;
    current_progress = progress;
    seek_count++;
  }
  double GetDuration() override {
    duration_read_count++;
    return duration;
  }
};

TEST(AudioControllerUnittest, PlayAndPause) {
  auto player = std::make_unique<TestPlayer>();
  auto test = player.get();
  auto controller = new AudioController();
  controller->SetAudioPlayer(std::move(player));
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 0);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 0);
  controller->OnResume();
  controller->OnTimeUpdate(0);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 200);
  controller->OnPause();
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 200);
}

TEST(AudioControllerUnittest, SyncTime) {
  auto player = std::make_unique<TestPlayer>();
  auto test = player.get();
  auto controller = new AudioController();
  controller->SetAudioPlayer(std::move(player));
  controller->OnResume();
  controller->OnTimeUpdate(0);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->OnTimeUpdate(700);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 700);
  controller->OnPause();
  controller->OnTimeUpdate(200);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 700);
}

TEST(AudioControllerUnittest, DefaultAudio) {
  auto asset = AudioAsset::Make(AudioAssetModel{});
  auto player = AudioPlayer::MakeAudioPlayer(nullptr, asset);
  auto controller = new AudioController();
  controller->SetAudioPlayer(std::move(player));
  EXPECT_EQ(asset, nullptr);
  EXPECT_EQ(player, nullptr);
  EXPECT_FALSE(controller->HasAudioPlayer());
}

TEST(AudioControllerUnittest, MuteAudio) {
  auto player = std::make_unique<TestPlayer>();
  auto test = player.get();
  auto controller = new AudioController();
  controller->SetAudioPlayer(std::move(player));
  controller->OnResume();
  controller->OnTimeUpdate(0);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->SetMuted(true);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->OnTimeUpdate(200);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->SetMuted(false);
  controller->OnTimeUpdate(300);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 300);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 400);
}

TEST(AudioControllerUnittest, FrequentSyncTime) {
  auto player = std::make_unique<TestPlayer>();
  auto test = player.get();
  auto controller = new AudioController();
  controller->SetAudioPlayer(std::move(player));
  controller->OnResume();
  controller->OnTimeUpdate(0);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 0);
  controller->OnTimeUpdate(100);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->OnTimeUpdate(300);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 300);
  controller->OnTimeUpdate(500);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 500);
  controller->OnTimeUpdate(600);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 500);
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
  controller->OnTimeUpdate(900);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 900);
  controller->OnTimeUpdate(100);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->OnTimeUpdate(500);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 500);
  controller->OnTimeUpdate(700);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 500);
}

TEST(AudioControllerUnittest, MillisecondTargetIndependentOfAudioDuration) {
  for (double duration_ms : {2040.0, 4080.0}) {
    SCOPED_TRACE(duration_ms);
    auto player = std::make_unique<TestPlayer>(duration_ms);
    auto* test = player.get();
    AudioController controller;
    controller.SetAudioPlayer(std::move(player));
    controller.OnResume();

    controller.OnTimeUpdate(1000);
    EXPECT_NEAR(test->GetAudioTime(), 1000, 0.000001);
    EXPECT_DOUBLE_EQ(test->current_progress, 1000 / duration_ms);
    EXPECT_EQ(test->seek_count, 1);
    EXPECT_TRUE(test->is_playing);
    EXPECT_EQ(test->duration_read_count, 1);

    controller.OnTimeUpdate(1000);
    EXPECT_EQ(test->seek_count, 1);
    EXPECT_EQ(test->resume_count, 2);
    EXPECT_EQ(test->duration_read_count, 2);
  }
}

TEST(AudioControllerUnittest, ContinuousPlaybackDoesNotRepeatedlySeek) {
  for (double duration_ms : {2040.0, 4080.0}) {
    SCOPED_TRACE(duration_ms);
    auto player = std::make_unique<TestPlayer>(duration_ms);
    auto* test = player.get();
    AudioController controller;
    controller.SetAudioPlayer(std::move(player));
    controller.OnResume();
    controller.OnTimeUpdate(1000);
    ASSERT_EQ(test->seek_count, 1);

    // Advance the audio clock and layer time together across several loops.
    for (int frame = 1; frame <= 100; frame++) {
      test->Advance();
      controller.OnTimeUpdate(1000 + frame * 100);
      EXPECT_NEAR(test->GetAudioTime(),
                  std::fmod(1000 + frame * 100, duration_ms), 0.000001);
      EXPECT_EQ(test->seek_count, 1);
      EXPECT_EQ(test->resume_count, frame + 1);
    }
  }
}

TEST(AudioControllerUnittest, SourceLoopUsesSameTargetForSyncAndSeek) {
  auto player = std::make_unique<TestPlayer>(2000);
  auto* test = player.get();
  AudioController controller;
  controller.SetAudioPlayer(std::move(player));
  controller.OnResume();

  controller.OnTimeUpdate(5000);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 1000);
  EXPECT_EQ(test->seek_count, 1);
  controller.OnTimeUpdate(6000);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 0);
  EXPECT_EQ(test->seek_count, 2);
  controller.OnTimeUpdate(6000);
  EXPECT_EQ(test->seek_count, 2);
  EXPECT_TRUE(test->is_playing);
}

TEST(AudioControllerUnittest, NaturalLoopDoesNotForceSeek) {
  auto player = std::make_unique<TestPlayer>(2000);
  auto* test = player.get();
  AudioController controller;
  controller.SetAudioPlayer(std::move(player));
  controller.OnResume();
  controller.OnTimeUpdate(1990);
  ASSERT_EQ(test->seek_count, 1);

  // Both source looping and composition rewinding can cross the boundary.
  controller.OnTimeUpdate(2010);
  controller.OnTimeUpdate(10);
  EXPECT_EQ(test->seek_count, 1);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 1990);
  test->Advance(20);
  controller.OnTimeUpdate(10);
  EXPECT_EQ(test->seek_count, 1);
  EXPECT_NEAR(test->GetAudioTime(), 10, 0.000001);

  controller.OnTimeUpdate(500);
  EXPECT_EQ(test->seek_count, 2);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 500);
  EXPECT_TRUE(test->is_playing);
}

TEST(AudioControllerUnittest, InvalidDurationDoesNotSeekOrStart) {
  for (double duration_ms : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                             std::numeric_limits<double>::quiet_NaN()}) {
    SCOPED_TRACE(duration_ms);
    auto player = std::make_unique<TestPlayer>(duration_ms);
    auto* test = player.get();
    AudioController controller;
    controller.SetAudioPlayer(std::move(player));
    controller.OnResume();
    controller.OnTimeUpdate(1000);
    EXPECT_EQ(test->seek_count, 0);
    EXPECT_EQ(test->resume_count, 0);
  }
}

TEST(AudioControllerUnittest, InvalidTimeDoesNotSeekOrStart) {
  auto player = std::make_unique<TestPlayer>();
  auto* test = player.get();
  AudioController controller;
  controller.SetAudioPlayer(std::move(player));
  controller.OnResume();
  for (double time_ms : {-1.0, std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::quiet_NaN()}) {
    controller.OnTimeUpdate(time_ms);
    EXPECT_EQ(test->seek_count, 0);
    EXPECT_EQ(test->resume_count, 0);
  }
  controller.OnTimeUpdate(0);
  EXPECT_EQ(test->seek_count, 1);
  EXPECT_EQ(test->resume_count, 1);
}

TEST(AudioControllerUnittest, InactiveFramesPauseOnceAndAllowReentry) {
  auto player = std::make_unique<TestPlayer>();
  auto* test = player.get();
  AudioController controller;
  controller.SetAudioPlayer(std::move(player));
  controller.OnResume();

  controller.BeginFrame();
  controller.OnTimeUpdate(20);
  controller.EndFrame();
  EXPECT_TRUE(test->is_playing);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 20);
  EXPECT_EQ(test->seek_count, 1);
  EXPECT_EQ(test->pause_count, 0);

  for (int frame = 0; frame < 3; frame++) {
    controller.BeginFrame();
    controller.EndFrame();
    EXPECT_FALSE(test->is_playing);
    EXPECT_EQ(test->pause_count, 1);
  }

  // The 10 ms difference is below the drift threshold but reentry must seek.
  controller.BeginFrame();
  controller.OnTimeUpdate(30);
  controller.EndFrame();
  EXPECT_TRUE(test->is_playing);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 30);
  EXPECT_EQ(test->seek_count, 2);
  EXPECT_EQ(test->resume_count, 2);

  controller.BeginFrame();
  controller.OnTimeUpdate(30);
  controller.EndFrame();
  EXPECT_EQ(test->seek_count, 2);
  EXPECT_EQ(test->resume_count, 3);
  EXPECT_EQ(test->pause_count, 1);
}

TEST(AudioControllerUnittest, GlobalPausePreventsReentryUntilResume) {
  auto player = std::make_unique<TestPlayer>();
  auto* test = player.get();
  AudioController controller;
  controller.SetAudioPlayer(std::move(player));
  controller.OnResume();
  controller.OnTimeUpdate(500);
  controller.OnPause();

  controller.BeginFrame();
  controller.EndFrame();
  controller.BeginFrame();
  controller.OnTimeUpdate(20);
  controller.EndFrame();
  EXPECT_FALSE(test->is_playing);
  EXPECT_EQ(test->resume_count, 1);
  EXPECT_EQ(test->seek_count, 1);
  EXPECT_EQ(test->pause_count, 1);

  controller.OnResume();
  EXPECT_FALSE(test->is_playing);
  controller.BeginFrame();
  controller.OnTimeUpdate(20);
  controller.EndFrame();
  EXPECT_TRUE(test->is_playing);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 20);
}

TEST(AudioControllerUnittest, UnmuteWaitsForAnActiveFrame) {
  auto player = std::make_unique<TestPlayer>();
  auto* test = player.get();
  AudioController controller;
  controller.SetAudioPlayer(std::move(player));
  controller.OnResume();
  controller.OnTimeUpdate(500);
  controller.SetMuted(true);

  controller.BeginFrame();
  controller.OnTimeUpdate(520);
  controller.EndFrame();
  EXPECT_FALSE(test->is_playing);
  EXPECT_EQ(test->pause_count, 1);

  controller.SetMuted(false);
  controller.BeginFrame();
  controller.EndFrame();
  EXPECT_FALSE(test->is_playing);
  EXPECT_EQ(test->resume_count, 1);

  controller.BeginFrame();
  controller.OnTimeUpdate(520);
  controller.EndFrame();
  EXPECT_TRUE(test->is_playing);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 520);
  EXPECT_EQ(test->seek_count, 2);
}

TEST(AudioControllerUnittest, DelayedPlayerSeeksBeforeStarting) {
  AudioController controller;
  controller.OnResume();
  controller.BeginFrame();
  controller.OnTimeUpdate(20);
  controller.EndFrame();

  auto player = std::make_unique<TestPlayer>();
  auto* test = player.get();
  controller.SetAudioPlayer(std::move(player));
  controller.BeginFrame();
  controller.OnTimeUpdate(30);
  controller.EndFrame();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 30);
  EXPECT_EQ(test->seek_count, 1);
  EXPECT_EQ(test->resume_count, 1);
  EXPECT_TRUE(test->is_playing);

  // Replacing a player must also discard the previous player's playing state.
  auto replacement = std::make_unique<TestPlayer>();
  test = replacement.get();
  controller.SetAudioPlayer(std::move(replacement));
  controller.BeginFrame();
  controller.OnTimeUpdate(40);
  controller.EndFrame();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 40);
  EXPECT_EQ(test->seek_count, 1);
  EXPECT_TRUE(test->is_playing);

  controller.SetAudioPlayer(nullptr);
  controller.BeginFrame();
  controller.EndFrame();
  EXPECT_FALSE(controller.HasAudioPlayer());
}

TEST(AudioControllerUnittest, ReentryDoesNotRaiseDriftThreshold) {
  auto player = std::make_unique<TestPlayer>();
  auto* test = player.get();
  AudioController controller;
  controller.SetAudioPlayer(std::move(player));
  controller.OnResume();
  for (double time_ms : {10.0, 20.0, 30.0}) {
    controller.BeginFrame();
    controller.EndFrame();
    controller.BeginFrame();
    controller.OnTimeUpdate(time_ms);
    controller.EndFrame();
  }
  ASSERT_EQ(test->seek_count, 3);

  // The three lifecycle seeks must not increase the normal 50 ms threshold.
  controller.BeginFrame();
  controller.OnTimeUpdate(90);
  controller.EndFrame();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 90);
  EXPECT_EQ(test->seek_count, 4);
}
