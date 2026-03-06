// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/audio_controller.h"

#include "gtest/gtest.h"
#include "src/audio/audio_player.h"
#include "src/resource/asset/audio_asset.h"

using namespace lynx::animax;

class TestPlayer : public AudioPlayer {
 public:
  double current_progress = 0;
  double GetAudioTime() override { return current_progress * duration; }
  void Advance() {
    if (is_playing) {
      current_progress += 0.1;
    }
  }

 private:
  bool is_playing = false;
  double duration = 1000;
  void Pause() override { is_playing = false; }
  void Resume() override { is_playing = true; }
  void SeekToProgress(double progress) override { current_progress = progress; }
  double GetDuration() override { return duration; }
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
  controller->OnProgress(0);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 200);
  controller->OnPause();
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 200);
}

TEST(AudioControllerUnittest, SyncProgress) {
  auto player = std::make_unique<TestPlayer>();
  auto test = player.get();
  auto controller = new AudioController();
  controller->SetAudioPlayer(std::move(player));
  controller->OnResume();
  controller->OnProgress(0);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->OnProgress(0.7);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 700);
  controller->OnPause();
  controller->OnProgress(0.2);
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
  controller->OnProgress(0);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->SetMuted(true);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->OnProgress(0.2);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 100);
  controller->SetMuted(false);
  controller->OnProgress(0.3);
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 300);
  test->Advance();
  EXPECT_DOUBLE_EQ(test->GetAudioTime(), 400);
}
