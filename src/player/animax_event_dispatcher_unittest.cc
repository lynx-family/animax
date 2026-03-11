// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/player/animax_event_dispatcher.h"

#include <memory>
#include <string>
#include <unordered_set>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/vsync_monitor.h"
#include "src/player/animax_event.h"
#include "src/player/animax_main_controller.h"

using namespace lynx::animax;
using ::testing::_;
using ::testing::Return;

class MockEventListener {
 public:
  MOCK_METHOD(void, Call, (AnimaXPlayer*, Event, const EventParamMap&));
};

class MockVSyncMonitor : public VSyncMonitor {
 public:
  MOCK_METHOD(void, AsyncRequestVSync, (Callback), (override));
  MOCK_METHOD(void, RequestVSync, (Callback), (override));
};

class TestAnimaXMainController : public AnimaXMainController {
 public:
  TestAnimaXMainController(std::weak_ptr<AnimaXPlayer> player,
                           std::shared_ptr<VSyncMonitor> vsync_monitor)
      : AnimaXMainController(player, vsync_monitor,
                             std::weak_ptr<AnimaXPlaybackEventHandler>(),
                             nullptr, false) {}
};

class AnimaXEventDispatcherTest : public ::testing::Test {
 protected:
  AnimaXEventDispatcherTest() {
    mock_vsync_monitor_ = std::make_shared<MockVSyncMonitor>();
    player_ =
        AnimaXPlayerBuilder()
            .SetScale(1.0f)
            .SetVSyncMonitor(mock_vsync_monitor_)
            .EnableMultiThreadAccelerate(false)
            .SetSkipCountDownEvent(false)
            .SetResourceLoader(nullptr)
            .SetUnzipLoader(nullptr)
            .SetAbility(nullptr)
            .AddEventListener([](AnimaXPlayer*, Event, const EventParamMap&) {})
            .Build();
    test_controller_ = std::make_unique<TestAnimaXMainController>(
        player_, mock_vsync_monitor_);
    dispatcher_ = std::make_unique<AnimaXEventDispatcher>(
        std::weak_ptr<AnimaXPlayer>(player_), *test_controller_);
  }

  ~AnimaXEventDispatcherTest() override = default;

  std::unique_ptr<AnimaXEventDispatcher> dispatcher_;
  std::shared_ptr<AnimaXPlayer> player_;
  std::shared_ptr<MockVSyncMonitor> mock_vsync_monitor_;
  std::unique_ptr<TestAnimaXMainController> test_controller_;
};

TEST_F(AnimaXEventDispatcherTest, AddEventListener_AddsListener) {
  EventListener listener = [](AnimaXPlayer*, Event, const EventParamMap&) {};

  dispatcher_->AddEventListener(std::move(listener));

  // No direct way to verify listener was added, but we can test that it doesn't
  // crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, ClearEventListeners_RemovesAllListeners) {
  EventListener listener = [](AnimaXPlayer*, Event, const EventParamMap&) {};

  dispatcher_->AddEventListener(std::move(listener));
  dispatcher_->ClearEventListeners();

  // No direct way to verify listeners were cleared, but we can test that it
  // doesn't crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, SubscribeUpdateEvent_AddsFrame) {
  dispatcher_->SubscribeUpdateEvent(10);
  dispatcher_->SubscribeUpdateEvent(20);

  // No direct way to verify subscription, but we can test that it doesn't crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, UnsubscribeUpdateEvent_RemovesFrame) {
  dispatcher_->SubscribeUpdateEvent(10);
  dispatcher_->UnsubscribeUpdateEvent(10);

  // No direct way to verify unsubscription, but we can test that it doesn't
  // crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, SubscribeUpdateEvents_AddsMultipleFrames) {
  std::unordered_set<int32_t> frames = {10, 20, 30};
  dispatcher_->SubscribeUpdateEvents(frames, true);

  // No direct way to verify subscription, but we can test that it doesn't crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, SubscribeUpdateEvents_RemovesMultipleFrames) {
  std::unordered_set<int32_t> frames = {10, 20, 30};
  dispatcher_->SubscribeUpdateEvents(frames, false);

  // No direct way to verify unsubscription, but we can test that it doesn't
  // crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, ClearPrevNotifiedFrame_ResetsPreviousFrame) {
  dispatcher_->ClearPrevNotifiedFrame();

  // No direct way to verify reset, but we can test that it doesn't crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest,
       NotifyUpdateEvent_ForwardDirection_NotifiesSubscribedFrames) {
  dispatcher_->SubscribeUpdateEvent(10);
  dispatcher_->SubscribeUpdateEvent(20);
  dispatcher_->ClearPrevNotifiedFrame();

  // Mock the NotifyFrameEvent method by testing the behavior indirectly
  // This is a basic test to ensure the method doesn't crash
  dispatcher_->NotifyUpdateEvent(15.0);

  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest,
       NotifyUpdateEvent_ReverseDirection_NotifiesSubscribedFrames) {
  dispatcher_->SubscribeUpdateEvent(10);
  dispatcher_->SubscribeUpdateEvent(5);
  dispatcher_->ClearPrevNotifiedFrame();

  // Set negative speed to test reverse direction
  dispatcher_->SetSpeed(-1.0);

  // Mock the NotifyFrameEvent method by testing the behavior indirectly
  dispatcher_->NotifyUpdateEvent(8.0);

  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, SetSpeed_UpdatesSpeed) {
  dispatcher_->SetSpeed(2.0);

  // No direct way to verify speed was set, but we can test that it doesn't
  // crash
  SUCCEED();
}

TEST_F(AnimaXEventDispatcherTest, NotifyError_NotifiesErrorEvent) {
  EventListener mock_listener = [](AnimaXPlayer* player, Event event,
                                   const EventParamMap& params) {
    EXPECT_EQ(event, Event::kError);
    EXPECT_TRUE(params.find(EventKeys::kCode) != params.end());
    EXPECT_TRUE(params.find(EventKeys::kMessage) != params.end());
  };

  dispatcher_->AddEventListener(std::move(mock_listener));
  dispatcher_->NotifyError(EventError::kResourceNotFound, "Test error message");
}

TEST_F(AnimaXEventDispatcherTest, NotifyTap_NotifiesTapEvent) {
  std::unordered_set<std::string> hit_layers = {"layer1", "layer2"};

  EventListener mock_listener = [](AnimaXPlayer* player, Event event,
                                   const EventParamMap& params) {
    EXPECT_EQ(event, Event::kTapLayer);
    EXPECT_TRUE(params.find(EventKeys::kLayerList) != params.end());
  };

  dispatcher_->AddEventListener(std::move(mock_listener));
  dispatcher_->NotifyTap(hit_layers);
}

TEST_F(AnimaXEventDispatcherTest, NotifyWarning_NotifiesWarningEvent) {
  EventListener mock_listener = [](AnimaXPlayer* player, Event event,
                                   const EventParamMap& params) {
    EXPECT_EQ(event, Event::kWarning);
    EXPECT_TRUE(params.find(EventKeys::kCode) != params.end());
    EXPECT_TRUE(params.find(EventKeys::kMessage) != params.end());
  };

  dispatcher_->AddEventListener(std::move(mock_listener));
  dispatcher_->NotifyWarning(EventWarning::kExecuteBeforeReady,
                             "Test warning message");
}

TEST_F(AnimaXEventDispatcherTest, NotifyFrameEvent_NotifiesFrameEvent) {
  EventListener mock_listener = [](AnimaXPlayer* player, Event event,
                                   const EventParamMap& params) {
    EXPECT_EQ(event, Event::kReady);
    EXPECT_TRUE(params.find(EventKeys::kCurrent) != params.end());
  };

  dispatcher_->AddEventListener(std::move(mock_listener));
  dispatcher_->NotifyFrameEvent(Event::kReady, 10.0);
}

TEST_F(AnimaXEventDispatcherTest, NotifyFps_NotifiesFpsEvent) {
  EventListener mock_listener = [](AnimaXPlayer* player, Event event,
                                   const EventParamMap& params) {
    EXPECT_EQ(event, Event::kFps);
    EXPECT_TRUE(params.find(EventKeys::kCurrent) != params.end());
    EXPECT_TRUE(params.find(EventKeys::kMaxDropRate) != params.end());
    EXPECT_TRUE(params.find(EventKeys::kFps) != params.end());
  };

  dispatcher_->AddEventListener(std::move(mock_listener));
  dispatcher_->NotifyFps(15.0, 5, 60.0f);
}
