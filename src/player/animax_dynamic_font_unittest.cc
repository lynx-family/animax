// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/vsync_monitor.h"
#include "include/resource/raw_data.h"
#include "include/resource/resource_task.h"
#include "skity/graphic/bitmap.hpp"
#include "skity/render/canvas.hpp"
#include "skity/text/font.hpp"
#include "src/base/thread/task_runner.h"
#include "src/layer/text_helper.h"
#include "src/layer/textra/text_helper_textra.h"
#include "src/layer/textra/textra_include.h"
#include "src/render/canvas.h"
#include "src/render/font.h"
#include "src/testing/font_test_data.h"

namespace lynx {
namespace animax {
namespace {

constexpr auto kTimeout = std::chrono::seconds(5);
constexpr char kFontURL[] = "https://example.com/dynamic-font.ttf";
constexpr char kTextLayer[] = "static-text";
constexpr int32_t kWidth = 800;
constexpr int32_t kHeight = 160;

// The static object form gives Textra a cached layout, so a font response must
// invalidate it even though preview and playback both stay at frame zero.
constexpr char kTextComposition[] = R"({
  "v":"5.7.4","fr":30,"ip":0,"op":60,"w":800,"h":160,"ddd":0,
  "assets":[],"markers":[],
  "fonts":{"list":[{"fName":"DynamicTestFont",
    "fFamily":"__animax_dynamic_font_test__","fStyle":"Regular",
    "ascent":75,"fPath":"https://example.com/dynamic-font.ttf","origin":3}]},
  "layers":[{"ddd":0,"ind":1,"ty":5,"nm":"static-text","sr":1,
    "ks":{"o":{"a":0,"k":100},"r":{"a":0,"k":0},
      "p":{"a":0,"k":[20,100,0]},"a":{"a":0,"k":[0,0,0]},
      "s":{"a":0,"k":[100,100,100]}},
    "t":{"d":{"k":{"t":"Wi.mixed-width 0123456789",
      "f":"DynamicTestFont","s":40,"j":0,"tr":0,"lh":48,
      "ls":0,"fc":[0,0,0]}},
      "m":{"g":1,"a":{"a":0,"k":[0,0]}},"a":[]},
    "ip":0,"op":60,"st":0,"bm":0}]
})";

constexpr char kEmptyComposition[] = R"({
  "v":"5.7.4","fr":30,"ip":0,"op":60,"w":800,"h":160,
  "assets":[],"layers":[],"markers":[]
})";

std::unique_ptr<RawData> CopyFontData(const std::vector<uint8_t>& bytes) {
  auto* copy = new std::vector<uint8_t>(bytes);
  return RawData::MakeRawData(
      copy->data(), copy->size(),
      [](const void* context) {
        delete static_cast<const std::vector<uint8_t>*>(context);
      },
      copy);
}

class ControlledFontLoader final
    : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ControlledFontLoader(std::shared_ptr<const std::vector<uint8_t>> bytes,
                       bool complete_immediately)
      : bytes_(std::move(bytes)), complete_immediately_(complete_immediately) {}

  void Load(ResourceRequest request, CallbackType callback) override {
    EXPECT_EQ(ResourceRequestType::kLoadRawData, request.type);
    EXPECT_EQ(kFontURL, request.uri_info.uri);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      ++requests_;
      if (!complete_immediately_) {
        pending_.push_back(std::move(callback));
      }
    }
    condition_.notify_all();
    if (complete_immediately_) {
      callback(
          ResourceResponse{MakeRawDataResourcePayload(CopyFontData(*bytes_))},
          {});
    }
  }

  bool WaitForRequest() {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, kTimeout,
                               [this] { return !pending_.empty(); });
  }

  size_t RequestCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return requests_;
  }

  bool Complete(bool success) {
    CallbackType callback;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (pending_.empty()) {
        return false;
      }
      callback = std::move(pending_.front());
      pending_.erase(pending_.begin());
    }
    GetAnimaXResourceThread()->PostTask([callback = std::move(callback),
                                         bytes = bytes_, success]() mutable {
      if (success) {
        callback(
            ResourceResponse{MakeRawDataResourcePayload(CopyFontData(*bytes))},
            {});
      } else {
        callback({}, {kUserDefined, "font request failed"});
      }
    });
    return true;
  }

 private:
  std::shared_ptr<const std::vector<uint8_t>> bytes_;
  const bool complete_immediately_;
  std::mutex mutex_;
  std::condition_variable condition_;
  size_t requests_ = 0;
  std::vector<CallbackType> pending_;
};

class ManualVSyncMonitor final : public VSyncMonitor {
 public:
  // Seek and Play's initial draw run normally. Suppressing recurring
  // ticks keeps playback at frame zero and avoids a self-scheduling test loop.
  void AsyncRequestVSync(Callback) override {}
  void RequestVSync(Callback) override {}
};

struct PlayerSignals {
  std::mutex mutex;
  std::condition_variable condition;
  size_t ready = 0;
  size_t frames = 0;
  std::vector<uint8_t> pixels;

  void OnReady() {
    {
      std::lock_guard<std::mutex> lock(mutex);
      ++ready;
    }
    condition.notify_all();
  }

  void OnFrame(const skity::Bitmap& bitmap) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      const auto* data = bitmap.GetPixelAddr();
      pixels.assign(data, data + bitmap.RowBytes() * bitmap.Height());
      ++frames;
    }
    condition.notify_all();
  }

  bool WaitForReady(size_t count) {
    std::unique_lock<std::mutex> lock(mutex);
    return condition.wait_for(lock, kTimeout,
                              [this, count] { return ready >= count; });
  }

  bool WaitForFrameAfter(size_t count) {
    std::unique_lock<std::mutex> lock(mutex);
    return condition.wait_for(lock, kTimeout,
                              [this, count] { return frames > count; });
  }

  size_t FrameCount() {
    std::lock_guard<std::mutex> lock(mutex);
    return frames;
  }

  std::vector<uint8_t> Pixels() {
    std::lock_guard<std::mutex> lock(mutex);
    return pixels;
  }
};

class FontTestSurface final : public AnimaXSurface {
 public:
  explicit FontTestSurface(std::shared_ptr<PlayerSignals> signals)
      : AnimaXSurface(kWidth, kHeight),
        signals_(std::move(signals)),
        bitmap_(kWidth, kHeight),
        skity_canvas_(skity::Canvas::MakeSoftwareCanvas(&bitmap_)),
        canvas_(CreateAnimaXCanvas(skity_canvas_.get(), kWidth, kHeight,
                                   nullptr)) {}

  void Flush() override {
    skity_canvas_->Flush();
    signals_->OnFrame(bitmap_);
  }

  animax::Canvas* Canvas() override {
    skity_canvas_->Clear(skity::Color{0});
    return canvas_.get();
  }

  AnimaXBackend Type() const override { return AnimaXBackend::kSoftware; }
  bool Valid() const override { return skity_canvas_ != nullptr; }

 private:
  std::shared_ptr<PlayerSignals> signals_;
  skity::Bitmap bitmap_;
  std::unique_ptr<skity::Canvas> skity_canvas_;
  std::unique_ptr<animax::Canvas> canvas_;
};

bool Drain(const RunnerPtr& runner) {
  auto done = std::make_shared<std::promise<void>>();
  auto result = done->get_future();
  runner->PostTask([done] { done->set_value(); });
  return result.wait_for(kTimeout) == std::future_status::ready;
}

struct TextBounds {
  bool success = false;
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;
};

struct PlayerRun {
  std::shared_ptr<AnimaXPlayer> player;
  std::shared_ptr<ControlledFontLoader> loader;
  std::shared_ptr<PlayerSignals> signals;
};

enum class TextBackend { kDefault, kTextra };

class TestFallbackFontManager : public ttoffice::tttext::SkityFontManager {
 public:
  explicit TestFallbackFontManager(TypefaceRef typeface)
      : typeface_(std::move(typeface)) {}

  TypefaceRef legacyMakeTypeface(const char*,
                                 ttoffice::tttext::FontStyle) const override {
    return typeface_;
  }

 private:
  TypefaceRef typeface_;
};

class TestTextraTextHelper : public TextHelper {
 public:
  void SetFallbackTypeface(std::shared_ptr<skity::Typeface> typeface) {
    fallback_ = std::make_shared<TestFallbackFontManager>(
        std::make_shared<skity::textlayout::SkityTypefaceHelper>(
            std::move(typeface)));
  }

  std::unique_ptr<TextContent> CreateTextContent(
      const TextContentDataSource& data_source) override {
    // CI may have no system fonts. Keep the real asset manager first so the
    // external font replaces this fallback through the normal cache path.
    auto* collection =
        static_cast<TTFontMgrCollection*>(data_source.GetFontMgrCollection());
    collection->SetTestFontManager(fallback_);
    return helper_.CreateTextContent(data_source);
  }

  std::unique_ptr<FontAssetManager> CreateFontAssetManager() override {
    return helper_.CreateFontAssetManager();
  }

 private:
  TextHelperTextra helper_;
  std::shared_ptr<TestFallbackFontManager> fallback_;
};

class AnimaXDynamicFontTest : public ::testing::TestWithParam<TextBackend> {
 protected:
  void SetUp() override {
    font_bytes_ =
        std::make_shared<const std::vector<uint8_t>>(MakeTestFontBytes());
    ASSERT_FALSE(font_bytes_->empty());
    gpu_ = GetAnimaXGPUThreadHolder(false);
    previous_text_helper_ = &TextHelper::Impl();
    TextHelper::RegisterImpl(GetParam() == TextBackend::kDefault
                                 ? &default_text_helper_
                                 : &textra_text_helper_);
    if (GetParam() == TextBackend::kTextra) {
      auto data = MakeTestFontData(TestFontVariant::kWide);
      auto font = Font::MakeFont(data->Data(), data->Length(), "Regular");
      ASSERT_TRUE(font->HasValidTypeface());
      textra_text_helper_.SetFallbackTypeface(font->GetFont().GetTypeface());
    }
  }

  void TearDown() override {
    for (const auto& run : runs_) {
      run->player->Destroy();
    }
    EXPECT_TRUE(Drain(GetAnimaXMainThread()));
    EXPECT_TRUE(Drain(GetAnimaXResourceThread()));
    if (gpu_) {
      EXPECT_TRUE(Drain(gpu_->Get()));
    }
    runs_.clear();
    EXPECT_TRUE(Drain(GetAnimaXMainThread()));
    if (gpu_) {
      EXPECT_TRUE(Drain(gpu_->Get()));
    }
    TextHelper::RegisterImpl(previous_text_helper_);
    gpu_.reset();
  }

  PlayerRun& CreatePlayer(bool dynamic,
                          const char* composition = kTextComposition) {
    auto run = std::make_unique<PlayerRun>();
    run->signals = std::make_shared<PlayerSignals>();
    run->loader = std::make_shared<ControlledFontLoader>(font_bytes_, !dynamic);
    run->player = AnimaXPlayerBuilder()
                      .SetResourceLoader(run->loader)
                      .SetVSyncMonitor(std::make_shared<ManualVSyncMonitor>())
                      .AddEventListener([signals = run->signals](
                                            std::weak_ptr<AnimaXPlayer>,
                                            Event event, const EventParamMap&) {
                        if (event == Event::kReady) {
                          signals->OnReady();
                        }
                      })
                      .Build();
    run->player->SetAutoplay(false);
    run->player->SetDynamicResource(dynamic);
    run->player->CreateSurface(
        [signals = run->signals]() -> std::unique_ptr<AnimaXSurface> {
          return std::make_unique<FontTestSurface>(signals);
        });
    run->player->SetJson(composition);
    runs_.push_back(std::move(run));
    return *runs_.back();
  }

  TextBounds GetTextBounds(PlayerRun& run) {
    auto done = std::make_shared<std::promise<TextBounds>>();
    auto result = done->get_future();
    run.player->GetLayerBounds(
        kTextLayer, LayerBoundsSpace::kParent,
        [done](bool success, float x, float y, float width, float height) {
          done->set_value({success, x, y, width, height});
        });
    if (result.wait_for(kTimeout) != std::future_status::ready) {
      ADD_FAILURE() << "GetLayerBounds did not complete";
      return {};
    }
    return result.get();
  }

  bool SeekToFirstFrame(PlayerRun& run) {
    const auto frames = run.signals->FrameCount();
    run.player->Seek(0);
    return run.signals->WaitForFrameAfter(frames);
  }

  void ExpectSameBounds(const TextBounds& actual, const TextBounds& expected) {
    ASSERT_TRUE(actual.success);
    EXPECT_NEAR(actual.x, expected.x, 0.01f);
    EXPECT_NEAR(actual.y, expected.y, 0.01f);
    EXPECT_NEAR(actual.width, expected.width, 0.01f);
    EXPECT_NEAR(actual.height, expected.height, 0.01f);
  }

  TextHelper default_text_helper_;
  TestTextraTextHelper textra_text_helper_;
  TextHelper* previous_text_helper_ = nullptr;
  std::shared_ptr<GPUThreadHolder> gpu_;
  std::shared_ptr<const std::vector<uint8_t>> font_bytes_;
  std::vector<std::unique_ptr<PlayerRun>> runs_;
};

TEST_P(AnimaXDynamicFontTest, SeekThenPlayRefreshesStaticTextAfterFontLoad) {
  auto& reference = CreatePlayer(false);
  ASSERT_TRUE(reference.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(reference));
  const auto reference_bounds = GetTextBounds(reference);
  ASSERT_TRUE(reference_bounds.success);
  ASSERT_GT(reference_bounds.width, 0.f);
  const auto reference_pixels = reference.signals->Pixels();
  ASSERT_TRUE(std::any_of(reference_pixels.begin(), reference_pixels.end(),
                          [](uint8_t value) { return value != 0; }));
  EXPECT_EQ(1u, reference.loader->RequestCount());

  auto& run = CreatePlayer(true);
  ASSERT_TRUE(run.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(run));
  const auto fallback_bounds = GetTextBounds(run);
  ASSERT_TRUE(fallback_bounds.success);
  const auto fallback_pixels = run.signals->Pixels();
  ASSERT_NE(fallback_pixels, reference_pixels);
  EXPECT_EQ(0u, run.loader->RequestCount());

  run.player->Play();
  ASSERT_TRUE(run.loader->WaitForRequest());
  EXPECT_EQ(1u, run.loader->RequestCount());
  ExpectSameBounds(GetTextBounds(run), fallback_bounds);
  EXPECT_EQ(fallback_pixels, run.signals->Pixels());

  const auto frames = run.signals->FrameCount();
  ASSERT_TRUE(run.loader->Complete(true));
  ASSERT_TRUE(run.signals->WaitForFrameAfter(frames));
  ExpectSameBounds(GetTextBounds(run), reference_bounds);
  EXPECT_EQ(reference_pixels, run.signals->Pixels());
  EXPECT_EQ(1u, run.loader->RequestCount());
  ASSERT_TRUE(Drain(gpu_->Get()));
  EXPECT_TRUE(run.player->IsAnimating());
}

TEST_P(AnimaXDynamicFontTest, LoadingAssetsWaitsForSeekToRefreshPausedPreview) {
  auto& reference = CreatePlayer(false);
  ASSERT_TRUE(reference.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(reference));
  const auto reference_bounds = GetTextBounds(reference);
  ASSERT_TRUE(reference_bounds.success);
  const auto reference_pixels = reference.signals->Pixels();

  auto& run = CreatePlayer(true);
  ASSERT_TRUE(run.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(run));
  const auto fallback_pixels = run.signals->Pixels();
  ASSERT_NE(reference_pixels, fallback_pixels);
  EXPECT_EQ(0u, run.loader->RequestCount());

  auto done = std::make_shared<std::promise<void>>();
  auto result = done->get_future();
  run.player->LoadAssetsWithCallback([done] { done->set_value(); });
  ASSERT_TRUE(run.loader->WaitForRequest());
  const auto frames = run.signals->FrameCount();
  ASSERT_TRUE(run.loader->Complete(true));
  ASSERT_EQ(std::future_status::ready, result.wait_for(kTimeout));
  ASSERT_TRUE(Drain(gpu_->Get()));
  EXPECT_EQ(frames, run.signals->FrameCount());
  EXPECT_EQ(fallback_pixels, run.signals->Pixels());
  EXPECT_FALSE(run.player->IsAnimating());

  // Measuring bounds can refresh font caches without drawing. Check the
  // pixels produced by Seek before asking for the updated bounds.
  ASSERT_TRUE(SeekToFirstFrame(run));
  EXPECT_EQ(reference_pixels, run.signals->Pixels());
  ExpectSameBounds(GetTextBounds(run), reference_bounds);
  EXPECT_EQ(1u, run.loader->RequestCount());
  EXPECT_FALSE(run.player->IsAnimating());
}

TEST_P(AnimaXDynamicFontTest, LoadingAssetsWaitsForSeekWithNonzeroStartFrame) {
  std::string composition = kTextComposition;
  const auto start_frame_position = composition.find("\"ip\":0");
  ASSERT_NE(std::string::npos, start_frame_position);
  composition.replace(start_frame_position, std::string("\"ip\":0").size(),
                      "\"ip\":10");

  auto& reference = CreatePlayer(false, composition.c_str());
  ASSERT_TRUE(reference.signals->WaitForReady(1));
  const auto reference_frames = reference.signals->FrameCount();
  reference.player->Seek(5);
  ASSERT_TRUE(reference.signals->WaitForFrameAfter(reference_frames));
  ASSERT_DOUBLE_EQ(15.0, reference.player->GetCurrentFrame());
  const auto reference_bounds = GetTextBounds(reference);
  ASSERT_TRUE(reference_bounds.success);
  const auto reference_pixels = reference.signals->Pixels();

  auto& run = CreatePlayer(true, composition.c_str());
  ASSERT_TRUE(run.signals->WaitForReady(1));
  const auto preview_frames = run.signals->FrameCount();
  run.player->Seek(5);
  ASSERT_TRUE(run.signals->WaitForFrameAfter(preview_frames));
  const auto preview_frame = run.player->GetCurrentFrame();
  const auto preview_progress = run.player->GetProgress();
  ASSERT_DOUBLE_EQ(15.0, preview_frame);
  ASSERT_GT(preview_progress, 0.0);
  ASSERT_DOUBLE_EQ(reference.player->GetProgress(), preview_progress);
  ASSERT_FALSE(run.player->IsAnimating());
  const auto fallback_bounds = GetTextBounds(run);
  ASSERT_TRUE(fallback_bounds.success);
  ASSERT_NE(reference_bounds.width, fallback_bounds.width);
  const auto fallback_pixels = run.signals->Pixels();
  ASSERT_NE(reference_pixels, fallback_pixels);
  EXPECT_EQ(0u, run.loader->RequestCount());

  auto done = std::make_shared<std::promise<void>>();
  auto result = done->get_future();
  run.player->LoadAssetsWithCallback([done] { done->set_value(); });
  ASSERT_TRUE(run.loader->WaitForRequest());
  const auto frames = run.signals->FrameCount();
  ASSERT_TRUE(run.loader->Complete(true));
  ASSERT_EQ(std::future_status::ready, result.wait_for(kTimeout));
  ASSERT_TRUE(Drain(gpu_->Get()));
  EXPECT_EQ(frames, run.signals->FrameCount());
  EXPECT_EQ(fallback_pixels, run.signals->Pixels());
  EXPECT_DOUBLE_EQ(preview_frame, run.player->GetCurrentFrame());
  EXPECT_DOUBLE_EQ(preview_progress, run.player->GetProgress());
  EXPECT_FALSE(run.player->IsAnimating());

  run.player->Seek(5);
  ASSERT_TRUE(run.signals->WaitForFrameAfter(frames));
  EXPECT_EQ(reference_pixels, run.signals->Pixels());
  ExpectSameBounds(GetTextBounds(run), reference_bounds);
  EXPECT_DOUBLE_EQ(preview_frame, run.player->GetCurrentFrame());
  EXPECT_DOUBLE_EQ(preview_progress, run.player->GetProgress());
  EXPECT_FALSE(run.player->IsAnimating());
  EXPECT_EQ(1u, run.loader->RequestCount());
}

TEST_P(AnimaXDynamicFontTest,
       FailedFontLoadContinuesPlayAndKeepsPreviewUsable) {
  auto& run = CreatePlayer(true);
  ASSERT_TRUE(run.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(run));
  const auto fallback_bounds = GetTextBounds(run);
  ASSERT_TRUE(fallback_bounds.success);
  const auto fallback_pixels = run.signals->Pixels();

  run.player->Play();
  ASSERT_TRUE(run.loader->WaitForRequest());
  EXPECT_FALSE(run.player->IsAnimating());
  const auto frames = run.signals->FrameCount();
  ASSERT_TRUE(run.loader->Complete(false));
  ASSERT_TRUE(run.signals->WaitForFrameAfter(frames));
  ASSERT_TRUE(Drain(gpu_->Get()));
  EXPECT_TRUE(run.player->IsAnimating());

  ASSERT_TRUE(SeekToFirstFrame(run));
  ExpectSameBounds(GetTextBounds(run), fallback_bounds);
  EXPECT_EQ(fallback_pixels, run.signals->Pixels());
  EXPECT_EQ(1u, run.loader->RequestCount());
}

TEST_P(AnimaXDynamicFontTest,
       OldAssetCompletionDoesNotRedrawReplacementComposition) {
  auto& run = CreatePlayer(true);
  ASSERT_TRUE(run.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(run));

  // Completing a load-only request must neither start playback nor draw the
  // replacement composition.
  auto done = std::make_shared<std::promise<void>>();
  auto result = done->get_future();
  run.player->LoadAssetsWithCallback([done] { done->set_value(); });
  ASSERT_TRUE(run.loader->WaitForRequest());
  run.player->SetJson(kEmptyComposition);
  ASSERT_TRUE(run.signals->WaitForReady(2));
  ASSERT_TRUE(SeekToFirstFrame(run));
  const auto frames = run.signals->FrameCount();
  const auto pixels = run.signals->Pixels();

  ASSERT_TRUE(run.loader->Complete(true));
  ASSERT_EQ(std::future_status::ready, result.wait_for(kTimeout));
  // This public query also fences the GPU work scheduled before completion.
  EXPECT_FALSE(GetTextBounds(run).success);
  EXPECT_EQ(frames, run.signals->FrameCount());
  EXPECT_EQ(pixels, run.signals->Pixels());
  EXPECT_EQ(1u, run.loader->RequestCount());
}

TEST_P(AnimaXDynamicFontTest, NonDynamicPlayDoesNotRequestFontAgain) {
  auto& run = CreatePlayer(false);
  ASSERT_TRUE(run.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(run));
  const auto bounds = GetTextBounds(run);
  ASSERT_TRUE(bounds.success);
  const auto pixels = run.signals->Pixels();
  EXPECT_EQ(1u, run.loader->RequestCount());

  run.player->Play();
  ASSERT_TRUE(run.player->IsAnimating());
  ASSERT_TRUE(Drain(gpu_->Get()));
  ExpectSameBounds(GetTextBounds(run), bounds);
  EXPECT_EQ(pixels, run.signals->Pixels());
  EXPECT_EQ(1u, run.loader->RequestCount());
}

TEST_P(AnimaXDynamicFontTest, AssetCompletionAfterDestroyDoesNotDraw) {
  auto& run = CreatePlayer(true);
  ASSERT_TRUE(run.signals->WaitForReady(1));
  ASSERT_TRUE(SeekToFirstFrame(run));

  auto done = std::make_shared<std::promise<void>>();
  auto result = done->get_future();
  run.player->LoadAssetsWithCallback([done] { done->set_value(); });
  ASSERT_TRUE(run.loader->WaitForRequest());
  run.player->Destroy();
  ASSERT_TRUE(Drain(gpu_->Get()));
  const auto frames = run.signals->FrameCount();
  const auto pixels = run.signals->Pixels();

  ASSERT_TRUE(run.loader->Complete(true));
  ASSERT_EQ(std::future_status::ready, result.wait_for(kTimeout));
  EXPECT_FALSE(GetTextBounds(run).success);
  EXPECT_EQ(frames, run.signals->FrameCount());
  EXPECT_EQ(pixels, run.signals->Pixels());
  EXPECT_EQ(1u, run.loader->RequestCount());
}

INSTANTIATE_TEST_SUITE_P(TextBackends, AnimaXDynamicFontTest,
                         ::testing::Values(TextBackend::kDefault,
                                           TextBackend::kTextra),
                         [](const ::testing::TestParamInfo<TextBackend>& info) {
                           return info.param == TextBackend::kDefault
                                      ? "Default"
                                      : "Textra";
                         });

}  // namespace
}  // namespace animax
}  // namespace lynx
