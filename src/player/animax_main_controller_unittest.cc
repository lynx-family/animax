// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_main_controller.h"

#include <memory>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/vsync_monitor.h"
#include "src/base/thread/task_runner.h"
#include "src/player/animax_playback_event_handler.h"

using namespace lynx::animax;
using ::testing::StrictMock;

class MockVSyncMonitor : public VSyncMonitor {
 public:
  MOCK_METHOD(void, AsyncRequestVSync, (Callback));
  MOCK_METHOD(void, RequestVSync, (Callback));
};

class MockAnimaXPlaybackEventHandler : public AnimaXPlaybackEventHandler {
 public:
  explicit MockAnimaXPlaybackEventHandler() : AnimaXPlaybackEventHandler() {}

  MOCK_METHOD(void, OnNewLoop, (int32_t));
  MOCK_METHOD(void, OnEnd, ());
  MOCK_METHOD(void, OnProgress, (double, double));
  MOCK_METHOD(void, OnFps, (float, uint32_t));
  MOCK_METHOD(void, OnWarning, (EventWarning, const std::string&));
  MOCK_METHOD(void, OnError, (EventError, const std::string&));
};

class AnimaXMainControllerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mock_vsync_monitor_ = std::make_shared<StrictMock<MockVSyncMonitor>>();
    player_ = AnimaXPlayerBuilder().SetScale(1.0f).Build();
    weak_player_ = std::weak_ptr<AnimaXPlayer>(player_);

    mock_playback_handler_ =
        std::make_shared<StrictMock<MockAnimaXPlaybackEventHandler>>();

    controller_actor_ =
        std::make_shared<lynx::shell::LynxActor<AnimaXMainController>>(
            std::unique_ptr<AnimaXMainController>(new AnimaXMainController(
                player_->weak_from_this(), std::move(mock_vsync_monitor_),
                mock_playback_handler_)),
            GetAnimaXMainThread());
  }

  void TearDown() override {
    controller_actor_.reset();
    player_.reset();
  }

  std::shared_ptr<lynx::shell::LynxActor<AnimaXMainController>>
      controller_actor_;
  std::shared_ptr<AnimaXPlayer> player_;
  std::weak_ptr<AnimaXPlayer> weak_player_;
  std::shared_ptr<StrictMock<MockVSyncMonitor>> mock_vsync_monitor_;
  std::shared_ptr<StrictMock<MockAnimaXPlaybackEventHandler>>
      mock_playback_handler_;
};

TEST_F(AnimaXMainControllerTest, Constructor_CreatesValidController) {
  EXPECT_NE(controller_actor_->Impl(), nullptr);
  EXPECT_FALSE(controller_actor_->Impl()->GetPlayerID().empty());
  EXPECT_FALSE(controller_actor_->Impl()->GetAnimationID().empty());
  EXPECT_FLOAT_EQ(0.0f, controller_actor_->Impl()->GetTotalFrame());
  EXPECT_DOUBLE_EQ(0.0, controller_actor_->Impl()->GetCurrentFrame());
  EXPECT_DOUBLE_EQ(0.0, controller_actor_->Impl()->GetProgress());
}

TEST_F(AnimaXMainControllerTest, EnableDynamicResourceFeature_InitiallyFalse) {
  EXPECT_FALSE(controller_actor_->Impl()->EnableDynamicResourceFeature());
}

TEST_F(AnimaXMainControllerTest, GetPlayerID_ReturnsNonEmptyString) {
  std::string player_id = controller_actor_->Impl()->GetPlayerID();
  EXPECT_FALSE(player_id.empty());
  EXPECT_TRUE(player_id.find("ANIMAX_") == 0);
}

TEST_F(AnimaXMainControllerTest, GetAnimationID_ReturnsNonEmptyString) {
  std::string animation_id = controller_actor_->Impl()->GetAnimationID();
  EXPECT_FALSE(animation_id.empty());
  EXPECT_TRUE(animation_id.find("ANIMAX_") == 0);
  EXPECT_TRUE(animation_id.find("_ANIMATION_") != std::string::npos);
}

TEST_F(AnimaXMainControllerTest, GetCurrentSrc_InitiallyEmpty) {
  EXPECT_TRUE(controller_actor_->Impl()->GetCurrentSrc().empty());
}

TEST_F(AnimaXMainControllerTest, SetCurrentSrc_UpdatesSrc) {
  std::string test_src = "test_animation.json";
  controller_actor_->Impl()->SetCurrentSrc(test_src);
  EXPECT_EQ(controller_actor_->Impl()->GetCurrentSrc(), test_src);
}

TEST_F(AnimaXMainControllerTest, OnNewLoop_UpdatesLoopIndex) {
  controller_actor_->Impl()->OnNewLoop(2);
  EXPECT_EQ(controller_actor_->Impl()->GetLoopIndex(), 2);
}

TEST_F(AnimaXMainControllerTest, OnProgress_UpdatesCurrentFrame) {
  controller_actor_->Impl()->OnProgress(0.5, 50.0);
  EXPECT_EQ(controller_actor_->Impl()->GetCurrentFrame(), 50.0);
  EXPECT_DOUBLE_EQ(0.5, controller_actor_->Impl()->GetProgress());
}

TEST_F(AnimaXMainControllerTest,
       UpdateProperties_ReportsTimelineTotalAndDuration) {
  controller_actor_->Impl()->SetAutoplay(false);
  mock_playback_handler_.reset();

  auto meta = CompositionModelMeta{
      .start_frame = 10.0f,
      .end_frame = 69.99f,
      .duration = 2000,
      .frame_rate = 30.0f,
  };

  controller_actor_->Impl()->UpdateProperties(meta);

  EXPECT_FLOAT_EQ(60.0f, controller_actor_->Impl()->GetTotalFrame());
  EXPECT_DOUBLE_EQ(2000.0, controller_actor_->Impl()->GetDurationMs());
}

TEST_F(AnimaXMainControllerTest, ReportsTotalIndependentOfFrameRate) {
  controller_actor_->Impl()->SetAutoplay(false);
  mock_playback_handler_.reset();
  controller_actor_->Impl()->UpdateProperties(CompositionModelMeta{
      .start_frame = 10.0f,
      .end_frame = 69.99f,
      .duration = 0,
      .frame_rate = 0.0f,
  });

  EXPECT_FLOAT_EQ(60.0f, controller_actor_->Impl()->GetTotalFrame());
  EXPECT_DOUBLE_EQ(0.0, controller_actor_->Impl()->GetDurationMs());
}

TEST_F(AnimaXMainControllerTest, KeepsPlaybackFrameAtCompletion) {
  controller_actor_->Impl()->SetAutoplay(false);
  mock_playback_handler_.reset();
  controller_actor_->Impl()->UpdateProperties(CompositionModelMeta{
      .start_frame = 10.0f,
      .end_frame = 69.99f,
      .duration = 2000,
      .frame_rate = 30.0f,
  });

  controller_actor_->Impl()->OnProgress(1.0, 69.99f);

  EXPECT_DOUBLE_EQ(69.99f, controller_actor_->Impl()->GetCurrentFrame());
  EXPECT_DOUBLE_EQ(1.0, controller_actor_->Impl()->GetProgress());

  double completion_frame = 0.0;
  controller_actor_->Impl()->AddEventListener(
      [&completion_frame](std::weak_ptr<AnimaXPlayer>, Event event,
                          const EventParamMap& params) {
        if (event == Event::kCompletion) {
          completion_frame =
              params.at(EventKeys::kCurrent).double_val.value_or(0.0);
        }
      });
  controller_actor_->Impl()->NotifyCurrentFrameEvent(Event::kCompletion);

  EXPECT_DOUBLE_EQ(69.99f, completion_frame);
}

TEST_F(AnimaXMainControllerTest, IsAnimating_InitiallyFalse) {
  EXPECT_FALSE(controller_actor_->Impl()->IsAnimating());
}
