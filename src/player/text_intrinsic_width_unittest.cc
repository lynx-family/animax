// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <fstream>
#include <future>
#include <iterator>
#include <memory>
#include <mutex>
#include <vector>

#include "gtest/gtest.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/animax_surface.h"
#include "src/base/thread/task_runner.h"
#include "src/base/thread/thread_assert.h"
#include "src/layer/text_helper.h"
#include "src/layer/textra/text_helper_textra.h"
#include "src/model/composition_model.h"
#include "src/parser/composition_parser.h"

namespace lynx {
namespace animax {
namespace {

constexpr char kJSON[] = R"({
 "v":"5.6.10","w":100,"h":100,"ip":0,"op":60,"fr":30,
 "fonts":{"list":[{"fName":"test","fFamily":"Test","fStyle":"Regular","origin":3,"fPath":"https://example.com/font.ttf"}]},
 "assets":[{"id":"image","w":10,"h":10,"u":"https://example.com/","p":"image.png"}],
 "videos":[{"id":"video","x":10,"y":0,"w":10,"h":10,"ax":0,"ay":0,"aw":10,"ah":10,"u":"https://example.com/","p":"video.mp4"}],
 "layers":[{"ty":5,"ind":1,"nm":"text","ip":0,"op":60,"ks":{},
 "t":{"d":{"k":[{"s":{"t":"Hello World","f":"test","s":24,"j":0,"tr":0,"lh":28,"sz":[20,1],"ps":[0,0]}}]}}}]
})";

class MeasureSurface : public AnimaXSurface {
 public:
  MeasureSurface() : AnimaXSurface(100, 100) {}
  void Flush() override { ADD_FAILURE() << "Measurement must not draw"; }
  animax::Canvas* Canvas() override { return nullptr; }
  AnimaXBackend Type() const override { return AnimaXBackend::kGL; }
  bool Valid() const override { return true; }
};

class ControlledLoader : public Loader<ResourceRequest, ResourceResponse> {
 public:
  struct Pending {
    ResourceRequest request;
    CallbackType callback;
  };
  void Load(ResourceRequest request, CallbackType callback) override {
    std::lock_guard<std::mutex> lock(mutex_);
    requests_.push_back({std::move(request), std::move(callback)});
    ++request_count_;
    condition_.notify_one();
  }
  std::unique_ptr<Pending> Take() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!condition_.wait_for(lock, std::chrono::seconds(5),
                             [this] { return !requests_.empty(); })) {
      return nullptr;
    }
    auto pending = std::make_unique<Pending>(std::move(requests_.front()));
    requests_.pop_front();
    return pending;
  }
  int RequestCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return request_count_;
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::deque<Pending> requests_;
  int request_count_ = 0;
};

std::unique_ptr<RawData> ReadFont() {
  std::ifstream file(
      "example/darwin/ios/lynx_example/LynxExample/Resource/Fonts/"
      "notoserif.ttf",
      std::ios::binary);
  if (!file) {
    return nullptr;
  }
  auto bytes = std::make_unique<std::vector<uint8_t>>(
      std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  const auto* data = bytes->data();
  const auto size = bytes->size();
  return RawData::MakeRawData(
      data, size,
      [](const void* data) {
        delete static_cast<const std::vector<uint8_t>*>(data);
      },
      bytes.release());
}

class TextIntrinsicWidthPlayerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    previous_helper_ = &TextHelper::Impl();
    TextHelper::RegisterImpl(&helper_);
    loader_ = std::make_shared<ControlledLoader>();
    auto ready = std::make_shared<std::promise<void>>();
    auto ready_future = ready->get_future();
    AnimaXPlayerBuilder builder;
    builder.SetResourceLoader(loader_).AddEventListener(
        [ready](std::weak_ptr<AnimaXPlayer>, Event event,
                const EventParamMap&) {
          if (event == Event::kReady) {
            ready->set_value();
          }
        });
    player_ = builder.Build();
    player_->SetAutoplay(false);
    player_->SetDynamicResource(true);
    player_->CreateSurface([]() -> std::unique_ptr<AnimaXSurface> {
      return std::make_unique<MeasureSurface>();
    });
    model_ = CompositionParser::Parse(kJSON, std::strlen(kJSON), 2.f);
    ASSERT_NE(model_, nullptr);
    player_->SetComposition(model_);
    ASSERT_EQ(ready_future.wait_for(std::chrono::seconds(5)),
              std::future_status::ready);
  }
  void TearDown() override {
    if (player_) {
      player_->Destroy();
      player_.reset();
      auto drained = std::make_shared<std::promise<void>>();
      auto future = drained->get_future();
      GetAnimaXGPUThreadHolder(false)->PostTask(
          [drained] { drained->set_value(); });
      EXPECT_EQ(future.wait_for(std::chrono::seconds(5)),
                std::future_status::ready);
    }
    TextHelper::RegisterImpl(previous_helper_);
  }
  std::future<std::pair<bool, float>> Measure() {
    auto result = std::make_shared<std::promise<std::pair<bool, float>>>();
    auto future = result->get_future();
    player_->GetTextIntrinsicWidth("text", [result](bool success, float width) {
      EXPECT_EQ(ThreadAssert::GetCurrent(), ThreadAssert::Type::kGPU);
      result->set_value({success, width});
    });
    return future;
  }
  TextHelperTextra helper_;
  TextHelper* previous_helper_ = nullptr;
  std::shared_ptr<ControlledLoader> loader_;
  std::shared_ptr<AnimaXPlayer> player_;
  std::shared_ptr<CompositionModel> model_;
};

TEST_F(TextIntrinsicWidthPlayerTest,
       DynamicReadyLoadsOnlyTargetFontBeforePlay) {
  EXPECT_EQ(loader_->RequestCount(), 0);
  auto result = Measure();
  auto pending = loader_->Take();
  ASSERT_NE(pending, nullptr);
  EXPECT_EQ(pending->request.uri_info.content_type,
            UriInfo::ContentType::kFont);
  EXPECT_EQ(pending->request.type, ResourceRequestType::kLoadRawData);
  EXPECT_EQ(pending->request.uri_info.uri, "https://example.com/font.ttf");
  auto font = ReadFont();
  ASSERT_NE(font, nullptr);
  pending->callback({.payload = MakeRawDataResourcePayload(std::move(font))},
                    {});
  ASSERT_EQ(result.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
  auto [success, width] = result.get();
  EXPECT_TRUE(success);
  EXPECT_GT(width, 20.f);
  EXPECT_EQ(loader_->RequestCount(), 1);
  EXPECT_FALSE(model_->IsAssetsLoaded());
  EXPECT_FALSE(model_->GetImages().begin()->second->IsLoaded());
  EXPECT_FALSE(model_->GetVideos().begin()->second->IsValid());
  // The font is retained for reuse, but each query lays out current text again.
  auto repeated = Measure();
  ASSERT_EQ(repeated.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
  auto [again_success, again_width] = repeated.get();
  EXPECT_TRUE(again_success);
  EXPECT_FLOAT_EQ(again_width, width);
  EXPECT_EQ(loader_->RequestCount(), 1);
}

TEST_F(TextIntrinsicWidthPlayerTest, FontLoadFailureReturnsFalseAndZero) {
  auto result = Measure();
  auto pending = loader_->Take();
  ASSERT_NE(pending, nullptr);
  pending->callback({}, {.code = LoaderErrorCode::kInvalidLoaderRequest,
                         .message = "test font failure"});
  ASSERT_EQ(result.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
  EXPECT_EQ(result.get(), (std::pair<bool, float>{false, 0.f}));
  EXPECT_FALSE(model_->GetFonts().begin()->second->IsLoaded());
}

TEST_F(TextIntrinsicWidthPlayerTest,
       CorruptFontPayloadDoesNotCommitAndCanRetry) {
  auto result = Measure();
  auto pending = loader_->Take();
  ASSERT_NE(pending, nullptr);
  static constexpr char kInvalidFont[] = "not a font";
  pending->callback(
      {.payload = MakeRawDataResourcePayload(RawData::MakeRawData(
           kInvalidFont, sizeof(kInvalidFont), nullptr, nullptr))},
      {});
  ASSERT_EQ(result.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
  EXPECT_EQ(result.get(), (std::pair<bool, float>{false, 0.f}));
  EXPECT_FALSE(model_->GetFonts().begin()->second->IsLoaded());
  EXPECT_EQ(loader_->RequestCount(), 1);

  auto retry = Measure();
  auto retried_request = loader_->Take();
  ASSERT_NE(retried_request, nullptr);
  EXPECT_EQ(retried_request->request.uri_info.content_type,
            UriInfo::ContentType::kFont);
  auto font = ReadFont();
  ASSERT_NE(font, nullptr);
  retried_request->callback(
      {.payload = MakeRawDataResourcePayload(std::move(font))}, {});
  ASSERT_EQ(retry.wait_for(std::chrono::seconds(5)), std::future_status::ready);
  auto [success, width] = retry.get();
  EXPECT_TRUE(success);
  EXPECT_GT(width, 20.f);
  EXPECT_TRUE(model_->GetFonts().begin()->second->IsLoaded());
  EXPECT_EQ(loader_->RequestCount(), 2);
  EXPECT_FALSE(model_->IsAssetsLoaded());
}

TEST_F(TextIntrinsicWidthPlayerTest, SourceChangeCancelsBeforeNewModelArrives) {
  auto result = Measure();
  auto font_request = loader_->Take();
  ASSERT_NE(font_request, nullptr);
  // Keep the new main resource pending so the renderer still owns the old
  // model.
  player_->SetSrc("https://example.com/new.json");
  auto source_request = loader_->Take();
  ASSERT_NE(source_request, nullptr);
  EXPECT_EQ(source_request->request.uri_info.content_type,
            UriInfo::ContentType::kJson);
  static constexpr char kLateBytes[] = "late font";
  font_request->callback(
      {.payload = MakeRawDataResourcePayload(RawData::MakeRawData(
           kLateBytes, sizeof(kLateBytes), nullptr, nullptr))},
      {});
  ASSERT_EQ(result.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
  EXPECT_EQ(result.get(), (std::pair<bool, float>{false, 0.f}));
  EXPECT_FALSE(model_->GetFonts().begin()->second->IsLoaded());
  auto pending_model_query = Measure();
  ASSERT_EQ(pending_model_query.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
  EXPECT_EQ(pending_model_query.get(), (std::pair<bool, float>{false, 0.f}));
}

TEST_F(TextIntrinsicWidthPlayerTest,
       DestroyCancelsFontCommitAndStillCompletes) {
  auto result = Measure();
  auto pending = loader_->Take();
  ASSERT_NE(pending, nullptr);
  player_->Destroy();
  static constexpr char kLateBytes[] = "late font";
  pending->callback({.payload = MakeRawDataResourcePayload(RawData::MakeRawData(
                         kLateBytes, sizeof(kLateBytes), nullptr, nullptr))},
                    {});
  ASSERT_EQ(result.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
  EXPECT_EQ(result.get(), (std::pair<bool, float>{false, 0.f}));
  EXPECT_FALSE(model_->GetFonts().begin()->second->IsLoaded());
}

}  // namespace
}  // namespace animax
}  // namespace lynx
