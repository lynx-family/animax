// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_main_controller.h"

#include <cmath>
#include <memory>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/vsync_monitor.h"
#include "src/base/thread/task_runner.h"
#include "src/model/keyframe/keyframe_model.h"
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

TEST_F(AnimaXMainControllerTest, LegacyProgressUpdateOmitsInstallationId) {
  auto* controller = controller_actor_->Impl();
  controller->SetAutoplay(false);
  mock_playback_handler_.reset();
  controller->UpdateProperties(CompositionModelMeta{
      .start_frame = 0.0f,
      .end_frame = 50.0f,
      .frame_rate = 30.0f,
  });
  int update_count = 0;
  controller->AddEventListener([&update_count](std::weak_ptr<AnimaXPlayer>,
                                               Event event,
                                               const EventParamMap& params) {
    if (event != Event::kUpdate) {
      return;
    }
    ++update_count;
    EXPECT_EQ(params.end(), params.find(EventKeys::kInstallationId));
    auto current = params.find(EventKeys::kCurrent);
    ASSERT_NE(params.end(), current);
    ASSERT_TRUE(current->second.double_val.has_value());
    EXPECT_DOUBLE_EQ(10, *current->second.double_val);
  });
  controller->SubscribeUpdateEvent(10);
  controller->OnProgress(0.2, 10.0);
  EXPECT_EQ(1, update_count);
}

TEST_F(AnimaXMainControllerTest, RejectsMismatchedTapAndKeepsLegacyTap) {
  auto* controller = controller_actor_->Impl();
  int tap_count = 0;
  controller->AddEventListener([&tap_count](std::weak_ptr<AnimaXPlayer>,
                                            Event event,
                                            const EventParamMap& params) {
    EXPECT_EQ(Event::kTapLayer, event);
    EXPECT_EQ(params.end(), params.find(EventKeys::kInstallationId));
    auto layers = params.find(EventKeys::kLayerList);
    ASSERT_NE(params.end(), layers);
    ASSERT_NE(nullptr, layers->second.string_vector_val);
    EXPECT_THAT(*layers->second.string_vector_val,
                ::testing::UnorderedElementsAre("coupon"));
    ++tap_count;
  });

  controller->NotifyTapEvent({"coupon"}, 42);
  EXPECT_EQ(0, tap_count);
  controller->NotifyTapEvent({"coupon"});
  EXPECT_EQ(1, tap_count);
}

TEST_F(AnimaXMainControllerTest,
       BindingCancellationOnlyCompletesMatchingRequest) {
  ASSERT_TRUE(player_->SetCompositionMode(true));
  StrictMock<::testing::MockFunction<void(bool, std::string, std::string)>>
      completion;
  auto request =
      player_->BeginCompositionBinding(2, completion.AsStdFunction());
  ASSERT_TRUE(request->IsPending());
  EXPECT_EQ(2u, request->Id());

  player_->CancelCompositionBinding(1);
  EXPECT_TRUE(request->IsPending());

  EXPECT_CALL(completion,
              Call(false, "Superseded", ::testing::Not(::testing::IsEmpty())))
      .Times(1);
  player_->CancelCompositionBinding(2);
  EXPECT_FALSE(request->IsPending());
  player_->CancelCompositionBinding(2);
}

TEST_F(AnimaXMainControllerTest,
       BindingSupersessionAndRemovalAllowAnotherRequest) {
  ASSERT_TRUE(player_->SetCompositionMode(true));
  StrictMock<::testing::MockFunction<void(bool, std::string, std::string)>>
      first_completion, second_completion, retry_completion;
  auto first =
      player_->BeginCompositionBinding(1, first_completion.AsStdFunction());
  ASSERT_TRUE(first->IsPending());

  EXPECT_CALL(first_completion, Call(false, "Superseded", ::testing::_))
      .Times(1);
  auto second =
      player_->BeginCompositionBinding(2, second_completion.AsStdFunction());
  EXPECT_FALSE(first->IsPending());
  ASSERT_TRUE(second->IsPending());

  EXPECT_CALL(second_completion, Call(false, "Destroyed", ::testing::_))
      .Times(1);
  player_->CancelPendingCompositionBinding();
  EXPECT_FALSE(second->IsPending());

  auto retry =
      player_->BeginCompositionBinding(3, retry_completion.AsStdFunction());
  ASSERT_TRUE(retry->IsPending());
  EXPECT_CALL(retry_completion, Call(false, "Destroyed", ::testing::_))
      .Times(1);
  player_->Destroy();
  EXPECT_FALSE(retry->IsPending());
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

  EXPECT_DOUBLE_EQ(69.99, completion_frame);
}

TEST_F(AnimaXMainControllerTest, IsAnimating_InitiallyFalse) {
  EXPECT_FALSE(controller_actor_->Impl()->IsAnimating());
}

TEST_F(AnimaXMainControllerTest, VideoProgressUsesPlaybackKeyframeDomain) {
  auto* controller = controller_actor_->Impl();
  controller->SetAutoplay(false);
  mock_playback_handler_.reset();

  struct FrameCase {
    float start;
    float end;
    float frame;
  };
  for (const auto item :
       {FrameCase{0.f, 179.99f, 60.f}, FrameCase{15.5f, 195.74f, 75.f},
        FrameCase{-10.5f, 70.24f, 30.f}, FrameCase{0.1f, 70.24f, 3.f},
        FrameCase{0.6f, 29.99f, 3.f}}) {
    SCOPED_TRACE(::testing::Message()
                 << "start=" << item.start << ", end=" << item.end);
    CompositionModel composition(1.f);
    composition.Init(std::make_unique<RectF>(0.f, 0.f, 1.f, 1.f), item.start,
                     item.end, 30.f, false);
    controller->UpdateProperties(CompositionModelMeta{
        .start_frame = item.start,
        .end_frame = item.end,
        .duration = composition.GetTimelineDurationMs(),
        .frame_rate = 30.f,
        .has_video_layer = true,
    });

    controller->OnProgress(0.5, item.frame + 0.25);

    KeyframeModel child(composition, std::make_unique<Float>(1.f),
                        std::make_unique<Float>(1.f), nullptr, item.frame,
                        item.frame + 1.f);
    EXPECT_DOUBLE_EQ(item.frame, controller->GetCurrentFrame());
    const auto render_progress = static_cast<float>(controller->GetProgress());
    EXPECT_FLOAT_EQ(child.GetStartProgress(), render_progress);
    EXPECT_TRUE(child.ContainsProgress(render_progress));

    KeyframeModel previous(composition, std::make_unique<Float>(1.f),
                           std::make_unique<Float>(1.f), nullptr,
                           item.frame - 1.f, item.frame);
    EXPECT_FALSE(previous.ContainsProgress(render_progress));

    controller->OnProgress(0.5, item.frame + 1.25);

    EXPECT_DOUBLE_EQ(item.frame + 1.f, controller->GetCurrentFrame());
    const auto end_progress = static_cast<float>(controller->GetProgress());
    EXPECT_FLOAT_EQ(child.GetEndProgress(), end_progress);
    EXPECT_FALSE(child.ContainsProgress(end_progress));
  }
}

TEST_F(AnimaXMainControllerTest, VideoProgressClampsAtPlaybackEndpoints) {
  auto* controller = controller_actor_->Impl();
  controller->SetAutoplay(false);
  mock_playback_handler_.reset();

  struct FrameCase {
    float start;
    float end;
  };
  for (const auto item : {FrameCase{0.f, 179.99f}, FrameCase{10.f, 189.99f},
                          FrameCase{-10.5f, 70.24f}}) {
    CompositionModel composition(1.f);
    composition.Init(std::make_unique<RectF>(0.f, 0.f, 1.f, 1.f), item.start,
                     item.end, 30.f, false);
    controller->UpdateProperties(CompositionModelMeta{
        .start_frame = item.start,
        .end_frame = item.end,
        .duration = composition.GetTimelineDurationMs(),
        .frame_rate = 30.f,
        .has_video_layer = true,
    });

    controller->OnProgress(1.0, item.end);

    EXPECT_DOUBLE_EQ(std::round(item.end), controller->GetCurrentFrame());
    if (std::round(item.end) >= item.end) {
      EXPECT_DOUBLE_EQ(1.0, controller->GetProgress());
    } else {
      const float rounded_frame = std::round(item.end);
      KeyframeModel sample(composition, std::make_unique<Float>(1.f),
                           std::make_unique<Float>(1.f), nullptr, rounded_frame,
                           rounded_frame + 1.f);
      EXPECT_FLOAT_EQ(sample.GetStartProgress(),
                      static_cast<float>(controller->GetProgress()));
    }
    EXPECT_GE(controller->GetProgress(), 0.0);
    EXPECT_LE(controller->GetProgress(), 1.0);
    KeyframeModel visible(composition, std::make_unique<Float>(1.f),
                          std::make_unique<Float>(1.f), nullptr, item.start,
                          composition.GetTimelineEndFrame());
    EXPECT_TRUE(visible.ContainsProgress(controller->GetProgress()));
    EXPECT_DOUBLE_EQ(composition.GetTimelineDurationMs(),
                     controller->GetDurationMs());

    const double before_start = item.start - 2.25;
    controller->OnProgress(-0.1, before_start);
    EXPECT_DOUBLE_EQ(std::round(before_start), controller->GetCurrentFrame());
    EXPECT_DOUBLE_EQ(0.0, controller->GetProgress());
  }
}

TEST_F(AnimaXMainControllerTest, VideoProgressDoesNotDivideByRoundedZero) {
  auto* controller = controller_actor_->Impl();
  controller->SetAutoplay(false);
  mock_playback_handler_.reset();
  controller->UpdateProperties(CompositionModelMeta{
      .start_frame = 0.f,
      .end_frame = 0.49f,
      .duration = 16,
      .frame_rate = 30.f,
      .has_video_layer = true,
  });

  controller->OnProgress(0.4, 0.2);

  EXPECT_TRUE(std::isfinite(controller->GetProgress()));
  EXPECT_DOUBLE_EQ(0.0, controller->GetProgress());
  EXPECT_DOUBLE_EQ(0.0, controller->GetCurrentFrame());
}

TEST_F(AnimaXMainControllerTest, NonVideoProgressKeepsFractionalFrame) {
  auto* controller = controller_actor_->Impl();
  controller->SetAutoplay(false);
  mock_playback_handler_.reset();
  controller->UpdateProperties(CompositionModelMeta{
      .start_frame = 10.f,
      .end_frame = 69.99f,
      .duration = 2000,
      .frame_rate = 30.f,
      .has_video_layer = false,
  });

  controller->OnProgress(0.25, 25.125);

  EXPECT_DOUBLE_EQ(0.25, controller->GetProgress());
  EXPECT_DOUBLE_EQ(25.125, controller->GetCurrentFrame());
}
