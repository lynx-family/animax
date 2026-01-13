// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/monitor/animax_metrics_manager.h"

#include <sys/types.h>

#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "include/base/animax_metrics.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "src/base/monitor/animax_metric_names.h"
#include "src/base/monitor/animax_metrics_listener.h"
#include "src/base/thread/task_runner.h"
#include "src/player/animax_composition_loader.h"
#include "src/player/animax_main_controller.h"
#include "src/player/animax_renderer.h"

using namespace lynx::animax;

namespace {

constexpr uint32_t kWaitTime = 200;

class TestOnlyFPSListener : public FPSListener {
 public:
  explicit TestOnlyFPSListener(std::promise<bool> fps_promise)
      : fps_promise_(std::move(fps_promise)) {}

  void OnFps(float fps, uint32_t session_max_drop_value) override {
    fps_promise_.set_value(true);
  }

 private:
  std::promise<bool> fps_promise_;
};

}  // namespace

class AnimaXMetricsManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    player_ = AnimaXPlayerBuilder().SetScale(1.0f).Build();

    std::weak_ptr<AnimaXPlaybackEventHandler> playback_handler;
    controller_actor_ =
        std::make_shared<lynx::shell::LynxActor<AnimaXMainController>>(
            std::unique_ptr<AnimaXMainController>(new AnimaXMainController(
                player_->weak_from_this(), nullptr, playback_handler)),
            GetAnimaXMainThread());

    renderer_actor_ = std::make_shared<lynx::shell::LynxActor<AnimaXRenderer>>(
        std::unique_ptr<AnimaXRenderer>(new AnimaXRenderer(playback_handler)),
        GetAnimaXGPUThreadHolder(false)->Get());
    loader_actor_ = AnimaXCompositionLoader::Create();
    metrics_manager_ = std::make_shared<AnimaXMetricsManager>(
        loader_actor_, renderer_actor_, controller_actor_);
  }

  void TearDown() override {
    if (renderer_actor_) {
      renderer_actor_->Impl()->Destroy();
      renderer_actor_ = nullptr;
    }
  }

  std::shared_ptr<AnimaXPlayer> player_;
  std::shared_ptr<lynx::shell::LynxActor<AnimaXCompositionLoader>>
      loader_actor_;
  std::shared_ptr<lynx::shell::LynxActor<AnimaXRenderer>> renderer_actor_;
  std::shared_ptr<AnimaXMetricsManager> metrics_manager_;
  std::shared_ptr<lynx::shell::LynxActor<AnimaXMainController>>
      controller_actor_;
};

TEST_F(AnimaXMetricsManagerTest, SingleEvent) {
  std::promise<MetricsMap> metrics_promise;
  auto metrics_future = metrics_promise.get_future();

  // Directly call Trace methods instead of using threads
  loader_actor_->Impl()->Trace(TraceEventType::kRequestCompositionStart);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  loader_actor_->Impl()->Trace(TraceEventType::kPrepareAssetsEnd);

  ExternalMetricsReadyCallback on_completion =
      [&metrics_promise](MetricsMap metrics_map) {
        metrics_promise.set_value(metrics_map);
      };

  metrics_manager_->Collect(std::move(on_completion));

  auto status = metrics_future.wait_for(std::chrono::milliseconds(kWaitTime));
  EXPECT_EQ(std::future_status::ready, status) << "Promise timed out!";

  if (status == std::future_status::ready) {
    auto metrics_map = metrics_future.get();
    EXPECT_EQ(metrics_map.size(), 1);
    EXPECT_GT(metrics_map[AnimaXMetricNames::kPrepareResourceTime], 0);
  }
}

TEST_F(AnimaXMetricsManagerTest, MultipleEvents) {
  std::promise<MetricsMap> metrics_promise;
  auto metrics_future = metrics_promise.get_future();

  // Directly call Trace methods for multiple events
  loader_actor_->Impl()->Trace(TraceEventType::kRequestCompositionStart);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  loader_actor_->Impl()->Trace(TraceEventType::kRequestCompositionEnd);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  loader_actor_->Impl()->Trace(TraceEventType::kParseCompositionStart);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  loader_actor_->Impl()->Trace(TraceEventType::kParseCompositionEnd);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  loader_actor_->Impl()->Trace(TraceEventType::kPrepareAssetsStart);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  loader_actor_->Impl()->Trace(TraceEventType::kPrepareAssetsEnd);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  renderer_actor_->Impl()->Trace(TraceEventType::kRenderFrameStart);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  renderer_actor_->Impl()->Trace(TraceEventType::kRenderFrameEnd);

  ExternalMetricsReadyCallback on_completion =
      [&metrics_promise](MetricsMap metrics_map) {
        metrics_promise.set_value(metrics_map);
      };

  metrics_manager_->Collect(std::move(on_completion));

  auto status =
      metrics_future.wait_for(std::chrono::milliseconds(kWaitTime * 2));
  EXPECT_EQ(std::future_status::ready, status) << "Promise timed out!";

  if (status == std::future_status::ready) {
    auto metrics_map = metrics_future.get();
    EXPECT_EQ(metrics_map.size(), 8);
  }
}

TEST_F(AnimaXMetricsManagerTest, SingleEventButFailed) {
  std::promise<MetricsMap> metrics_promise;
  auto metrics_future = metrics_promise.get_future();

  // Call events in wrong order to test failure case
  loader_actor_->Impl()->Trace(TraceEventType::kPrepareAssetsEnd);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  loader_actor_->Impl()->Trace(TraceEventType::kRequestCompositionStart);

  ExternalMetricsReadyCallback on_completion =
      [&metrics_promise](MetricsMap metrics_map) {
        metrics_promise.set_value(metrics_map);
      };

  metrics_manager_->Collect(std::move(on_completion));

  auto status = metrics_future.wait_for(std::chrono::milliseconds(kWaitTime));
  EXPECT_EQ(std::future_status::ready, status) << "Promise timed out!";

  if (status == std::future_status::ready) {
    auto metrics_map = metrics_future.get();
    EXPECT_EQ(metrics_map.size(), 0);
  }
}

TEST_F(AnimaXMetricsManagerTest, Collect_WithNullActors_ReturnsEmptyMap) {
  MetricsMap result;

  auto manager =
      std::make_shared<AnimaXMetricsManager>(nullptr, nullptr, nullptr);
  manager->Collect(
      [&result](MetricsMap metrics) { result = std::move(metrics); });

  EXPECT_TRUE(result.empty());
}
