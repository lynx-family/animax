// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/player/animax_playback_event_handler.h"

#include <memory>

#include "base/include/lynx_actor.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/vsync_monitor.h"
#include "src/base/thread/task_runner.h"
#include "src/player/animax_player_context.h"
#include "src/player/animax_renderer.h"

using namespace lynx::animax;
using ::testing::_;
using ::testing::Return;

class MockAnimaXMainController : public AnimaXMainController {
 public:
  MockAnimaXMainController(
      std::weak_ptr<AnimaXPlayer> weak_player,
      std::shared_ptr<VSyncMonitor> vsync_monitor,
      std::shared_ptr<AnimaXPlaybackEventHandler> playback_handler)
      : AnimaXMainController(std::move(weak_player), std::move(vsync_monitor),
                             std::move(playback_handler)) {}
  ~MockAnimaXMainController() = default;

  MOCK_METHOD(void, NotifyCurrentFrameEvent, (const Event event), ());
  MOCK_METHOD(void, OnProgress, (double progress, double current_frame), ());
  MOCK_METHOD(void, OnNewLoop, (int32_t current_loop), ());
  MOCK_METHOD(void, OnEnd, (), ());
  MOCK_METHOD(void, NotifyWarning,
              (const EventWarning warning, const std::string& message), ());
  MOCK_METHOD(void, NotifyError,
              (const EventError error, const std::string& message), ());
  MOCK_METHOD(void, NotifyFpsEvent, (double fps, int32_t max_drop), ());
};

class AnimaXPlaybackEventHandlerTest : public ::testing::Test {
 protected:
  AnimaXPlaybackEventHandlerTest() {
    player_ = AnimaXPlayerBuilder().SetScale(1.0f).Build();
    handler_ = std::make_shared<AnimaXPlaybackEventHandler>();

    auto mock_controller =
        std::make_unique<MockAnimaXMainController>(player_, nullptr, handler_);
    mock_controller_ = mock_controller.get();

    test_controller_actor_ =
        std::make_shared<lynx::shell::LynxActor<AnimaXMainController>>(
            std::move(mock_controller), GetAnimaXMainThread());

    auto mock_renderer = std::make_unique<AnimaXRenderer>(handler_);

    test_renderer_actor_ =
        std::make_shared<lynx::shell::LynxActor<AnimaXRenderer>>(
            std::move(mock_renderer), GetAnimaXMainThread());
    context_->weak_main_controller = test_controller_actor_;
    context_->weak_renderer_actor = test_renderer_actor_;
    handler_->Init(context_);
  }

  std::shared_ptr<AnimaXPlayerContext> context_ =
      std::make_shared<AnimaXPlayerContext>();
  std::shared_ptr<AnimaXPlayer> player_;
  std::shared_ptr<AnimaXPlaybackEventHandler> handler_;
  MockAnimaXMainController* mock_controller_;
  std::shared_ptr<lynx::shell::LynxActor<AnimaXMainController>>
      test_controller_actor_;
  std::shared_ptr<lynx::shell::LynxActor<AnimaXRenderer>> test_renderer_actor_;
};

TEST_F(AnimaXPlaybackEventHandlerTest, OnStart_NotifiesStartEvent) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_, NotifyCurrentFrameEvent(Event::kStart));

  handler_->OnStart();
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnProgress_NotifiesUpdateEvent) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_, OnProgress(0.5, 10.0));

  handler_->OnProgress(0.5, 10.0);
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnNewLoop_NotifiesRepeatEvent) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_, OnNewLoop(1));

  handler_->OnNewLoop(1);
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnEnd_NotifiesCompletionEvent) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_, OnEnd());

  handler_->OnEnd();
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnCancel_NotifiesCancelEvent) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_, NotifyCurrentFrameEvent(Event::kCancel));

  handler_->OnCancel();
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnWarning_NotifiesWarning) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_,
              NotifyWarning(EventWarning::kExecuteBeforeReady, "Test warning"));

  handler_->OnWarning(EventWarning::kExecuteBeforeReady, "Test warning");
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnLayerError_NotifiesError) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_,
              NotifyError(EventError::kResourceNotFound, "Test error"));

  handler_->OnLayerError(EventError::kResourceNotFound, "Test error");
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnLayerWarning_NotifiesWarning) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(
      *mock_controller_,
      NotifyWarning(EventWarning::kExecuteBeforeReady, "Test layer warning"));

  handler_->OnLayerWarning(EventWarning::kExecuteBeforeReady,
                           "Test layer warning");
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnFps_NotifiesFps) {
  testing::Mock::AllowLeak(mock_controller_);
  EXPECT_CALL(*mock_controller_, NotifyFpsEvent(60.0f, 5));

  handler_->OnFps(60.0f, 5);
}

TEST_F(AnimaXPlaybackEventHandlerTest, OnPauseAndResume) {
  handler_->OnResume();
  handler_->OnPause();
}
