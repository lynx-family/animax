// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <chrono>
#include <future>
#include <sstream>

#include "src/audio/audio_controller.h"
#include "src/audio/audio_player.h"
#include "src/layer/base_layer.h"

#define private public
#include "src/layer/audio_layer.h"
#undef private

#include "gtest/gtest.h"
#include "include/player/animax_surface.h"
#include "skity/graphic/bitmap.hpp"
#include "src/base/thread/task_runner.h"
#include "src/layer/composition_layer.h"
#include "src/parser/alpha_player_asset/alpha_player_asset_parser.h"
#include "src/parser/composition_parser.h"
#include "src/player/animax_playback_event_handler.h"
#include "src/player/animax_renderer.h"
#include "src/render/surface_sw.h"

using namespace lynx::animax;

constexpr const char* lottie_json =
    "{\"v\":\"4.8.0\",\"fr\":30,\"ip\":0,\"op\":150,\"w\":160,\"h\":90,\"nm\":"
    "\"test\",\"ddd\":0,\"assets\":[{\"id\":\"audio_0\",\"u\":\"images/"
    "\",\"p\":\"aud_0.mp3\",\"e\":0,\"t\":2}],\"layers\":[{\"ddd\":0,\"ind\":1,"
    "\"ty\":6,\"nm\":\"27.mp3\",\"cl\":\"mp3\",\"refId\":\"audio_0\",\"sr\":1,"
    "\"ip\":0,\"op\":5006.88,\"st\":0,\"bm\":0}],\"markers\":[],\"props\":{}}";

class TestAsset : public AudioAsset {
 public:
  using AudioAsset::AudioAsset;
  void LoadLocal(const std::string& file_path) override { is_valid_ = true; }
};

TEST(AudioLayer, FullTest) {
  auto model = CompositionParser::Parse(lottie_json, std::strlen(lottie_json),
                                        1.0f, true);
  EXPECT_NE(nullptr, model);
  auto asset = std::make_shared<TestAsset>(AudioAssetModel{});
  asset->LoadLocal("");
  model->GetAudios()["audio_0"] = asset;
  auto layer_model = model->GetLayers().begin();
  auto layer = std::make_unique<AudioLayer>(**layer_model, *model);
  auto context = std::make_shared<AnimaXPlayerContext>();
  std::weak_ptr<AnimaXPlaybackEventHandler> weak_playback_handler;
  auto runner = GetAnimaXGPUThreadHolder(false);
  auto actor = std::make_shared<lynx::shell::LynxActor<AnimaXRenderer>>(
      std::make_unique<AnimaXRenderer>(weak_playback_handler), runner->Get());
  context->weak_renderer_actor = actor;
  layer->SetPlayerContext(context);
  layer->Init();
  layer->SetProgress(-1);
  layer->UpdateAudio();
  layer->SetProgress(0.5);
  layer->UpdateAudio();
  layer->SetProgress(2);
  layer->UpdateAudio();
}

TEST(AudioLayer, InvalidTest) {
  auto model = CompositionParser::Parse(lottie_json, std::strlen(lottie_json),
                                        1.0f, true);
  EXPECT_NE(nullptr, model);
  model->GetAudios()["audio_0"] = nullptr;
  auto layer_model = model->GetLayers().begin();
  auto layer = std::make_unique<AudioLayer>(**layer_model, *model);
  layer->Init();
}

namespace {

class RecordingAudioPlayer : public AudioPlayer {
 public:
  explicit RecordingAudioPlayer(double duration_ms)
      : duration_ms_(duration_ms) {}

  void Pause() override {
    is_playing = false;
    pause_count++;
  }
  void Resume() override {
    is_playing = true;
    resume_count++;
  }
  void SeekToProgress(double progress) override {
    last_progress = progress;
    current_time_ms = progress * duration_ms_;
    seek_count++;
  }
  double GetAudioTime() override { return current_time_ms; }
  double GetDuration() override { return duration_ms_; }

  double last_progress = 0;
  double current_time_ms = 0;
  int seek_count = 0;
  int pause_count = 0;
  int resume_count = 0;
  bool is_playing = false;

 private:
  double duration_ms_;
};

}  // namespace

TEST(AudioLayer, ElapsedTimeIndependentOfCompositionAndAudioDuration) {
  struct Timeline {
    float start_frame;
    float end_frame;
    float frame_rate;
    float audio_in_frame;
  };
  const Timeline timelines[] = {
      {0.f, 154.f, 24.201995f, 10.487532f},
      {0.f, 308.f, 24.201995f, 10.487532f},
      {48.f, 240.f, 30.f, 78.f},
      {0.f, 150.f, 30.f, 0.f},
  };
  auto context = std::make_shared<AnimaXPlayerContext>();
  auto runner = GetAnimaXGPUThreadHolder(false);
  auto actor = std::make_shared<lynx::shell::LynxActor<AnimaXRenderer>>(
      std::make_unique<AnimaXRenderer>(
          std::weak_ptr<AnimaXPlaybackEventHandler>{}),
      runner->Get());
  context->weak_renderer_actor = actor;

  for (const auto& timeline : timelines) {
    for (double duration_ms : {2040.0, 4080.0}) {
      std::ostringstream json;
      json.precision(9);
      json << R"({"v":"5.6.10","w":160,"h":90,"fr":)" << timeline.frame_rate
           << R"(,"ip":)" << timeline.start_frame << R"(,"op":)"
           << timeline.end_frame
           << R"(,"layers":[{"ty":6,"ind":1,"refId":"audio_0","sr":1,"ip":)"
           << timeline.audio_in_frame << R"(,"st":)" << timeline.audio_in_frame
           << R"(,"op":)" << timeline.end_frame << "}]}";
      auto text = json.str();
      auto model =
          CompositionParser::Parse(text.c_str(), text.size(), 1.f, true);
      ASSERT_NE(model, nullptr);
      ASSERT_EQ(model->GetLayers().size(), 1u);
      auto asset = std::make_shared<TestAsset>(AudioAssetModel{});
      asset->LoadLocal("");
      model->GetAudios()["audio_0"] = asset;

      auto done = std::make_shared<std::promise<void>>();
      auto future = done->get_future();
      actor->Act([context, model, timeline, duration_ms, done](auto&) {
        SCOPED_TRACE(::testing::Message()
                     << "composition frames=" << model->GetDurationFrames()
                     << ", audio duration=" << duration_ms);
        AudioLayer layer(*model->GetLayers().front(), *model);
        layer.SetPlayerContext(context);
        layer.Init();
        auto player = std::make_unique<RecordingAudioPlayer>(duration_ms);
        auto* test_player = player.get();
        layer.audio_controller_->SetAudioPlayer(std::move(player));
        layer.audio_controller_->SetMuted(false);
        layer.audio_controller_->OnResume();

        // One second after the audio in point must target one second of PCM,
        // regardless of either duration or the composition's start frame.
        float frame = timeline.audio_in_frame + timeline.frame_rate;
        layer.SetProgress((frame - timeline.start_frame) /
                          model->GetDurationFrames());
        layer.UpdateAudio();
        Canvas canvas(nullptr, 0, 0, nullptr);
        Matrix matrix;
        layer.Draw(canvas, matrix, 255);
        EXPECT_NEAR(test_player->GetAudioTime(), 1000.0, 0.001);
        EXPECT_NEAR(test_player->last_progress, 1000.0 / duration_ms, 0.000001);
        EXPECT_EQ(test_player->seek_count, 1);
        EXPECT_EQ(test_player->resume_count, 1);
        done->set_value();
      });
      ASSERT_EQ(future.wait_for(std::chrono::seconds(5)),
                std::future_status::ready);
    }
  }
}

namespace {

class AudioTestSurface : public AnimaXSurface {
 public:
  AudioTestSurface()
      : AnimaXSurface(100, 100),
        surface_(std::make_unique<skity::Bitmap>(100, 100)) {}

  void Flush() override { flush_count++; }
  lynx::animax::Canvas* Canvas() override {
    return has_canvas ? surface_.GetCanvas() : nullptr;
  }
  AnimaXBackend Type() const override { return AnimaXBackend::kSoftware; }
  bool Valid() const override { return valid; }

  bool valid = true;
  bool has_canvas = true;
  int flush_count = 0;

 private:
  SurfaceSW surface_;
};

constexpr const char* kAudioIntervalJSON = R"({
  "v":"5.6.10","w":100,"h":100,"fr":30,"ip":0,"op":180,
  "layers":[{"ty":6,"ind":1,"nm":"audio","refId":"audio_0",
             "sr":1,"ip":30,"op":90,"st":30}]
})";

constexpr const char* kNestedAudioIntervalJSON = R"({
  "v":"5.6.10","w":100,"h":100,"fr":30,"ip":0,"op":180,
  "layers":[{"ty":0,"ind":1,"nm":"parent","refId":"nested",
             "sr":1,"ip":30,"op":90,"st":30,"w":100,"h":100,"ks":{}}],
  "assets":[{"id":"nested","layers":[
    {"ty":6,"ind":2,"nm":"audio","refId":"audio_0",
     "sr":1,"ip":0,"op":150,"st":0}
  ]}]
})";

constexpr const char* kMovingAudioParentJSON = R"({
  "v":"5.6.10","w":100,"h":100,"fr":30,"ip":0,"op":180,
  "layers":[{"ty":0,"ind":1,"nm":"parent","refId":"nested",
    "sr":1,"ip":30,"op":90,"st":30,"w":100,"h":100,"bm":1,
    "ks":{"p":{"a":1,"k":[
      {"t":0,"s":[0,0,0],"h":1},
      {"t":46,"s":[200,0,0],"h":1},
      {"t":47,"s":[0,0,0],"h":1},
      {"t":180,"s":[0,0,0]}
    ]}}}],
  "assets":[{"id":"nested","layers":[
    {"ty":6,"ind":2,"nm":"audio","refId":"audio_0",
     "sr":1,"ip":0,"op":150,"st":0},
    {"ty":1,"ind":3,"nm":"solid","sw":100,"sh":100,"sc":"#ff0000",
     "sr":1,"ip":0,"op":150,"st":0,"ks":{}}
  ]}]
})";

class AudioLayerPlaybackTest : public ::testing::Test {
 protected:
  void SetUp() override {
    context_ = std::make_shared<AnimaXPlayerContext>();
    context_->disable_render_in_background = true;
    auto runner = GetAnimaXGPUThreadHolder(false);
    actor_ = std::make_shared<lynx::shell::LynxActor<AnimaXRenderer>>(
        std::make_unique<AnimaXRenderer>(
            std::weak_ptr<AnimaXPlaybackEventHandler>{}),
        runner->Get());
    context_->weak_renderer_actor = actor_;
    actor_->ActSync([this](auto& renderer) { renderer->Init(context_); });
  }

  void TearDown() override {
    actor_->ActSync([](auto& renderer) { renderer->Destroy(); });
  }

  void Load(const char* json) {
    model_ = CompositionParser::Parse(json, std::strlen(json), 1.f, true);
    ASSERT_NE(model_, nullptr);
    actor_->ActSync([this](auto& renderer) {
      renderer->CreateSurface(
          SurfaceCreationFactory([this]() -> std::unique_ptr<AnimaXSurface> {
            auto surface = std::make_unique<AudioTestSurface>();
            surface_ = surface.get();
            return surface;
          }));
      renderer->UpdateComposition(0, model_);
      ResolvedKeyPathElements elements;
      renderer->GetCompositionLayer()->ResolveKeyPath(
          AnimaXKeyPath(std::vector<std::string>{"**", "audio"}), 0, elements,
          AnimaXKeyPath());
      ASSERT_EQ(elements.size(), 1u);
      auto* layer = static_cast<AudioLayer*>(elements.front().second);
      auto player = std::make_unique<RecordingAudioPlayer>(2000);
      player_ = player.get();
      layer->audio_controller_->SetAudioPlayer(std::move(player));
      renderer->OnResume();
    });
    ASSERT_NE(player_, nullptr);
  }

  void RenderFrame(AnimaXRenderer& renderer, float frame) {
    renderer.Render(frame / model_->GetDurationFrames());
  }

  std::shared_ptr<AnimaXPlayerContext> context_;
  std::shared_ptr<lynx::shell::LynxActor<AnimaXRenderer>> actor_;
  std::shared_ptr<CompositionModel> model_;
  AudioTestSurface* surface_ = nullptr;
  RecordingAudioPlayer* player_ = nullptr;
};

TEST_F(AudioLayerPlaybackTest, OwnIntervalPausesAtOutPointAndResumesOnReentry) {
  ASSERT_NO_FATAL_FAILURE(Load(kAudioIntervalJSON));
  actor_->ActSync([this](auto& renderer) {
    RenderFrame(*renderer, 15);
    EXPECT_EQ(player_->resume_count, 0);
    RenderFrame(*renderer, 45);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_NEAR(player_->GetAudioTime(), 500, 0.001);
    EXPECT_EQ(player_->pause_count, 0);

    RenderFrame(*renderer, 91);
    EXPECT_FALSE(player_->is_playing);
    EXPECT_EQ(player_->pause_count, 1);
    RenderFrame(*renderer, 120);
    EXPECT_EQ(player_->pause_count, 1);

    RenderFrame(*renderer, 30.6f);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_NEAR(player_->GetAudioTime(), 20, 0.001);
    EXPECT_EQ(player_->seek_count, 2);
    RenderFrame(*renderer, 30.6f);
    EXPECT_EQ(player_->seek_count, 2);
  });
}

TEST_F(AudioLayerPlaybackTest, ParentOutPointPausesStillActiveChildAudio) {
  ASSERT_NO_FATAL_FAILURE(Load(kNestedAudioIntervalJSON));
  actor_->ActSync([this](auto& renderer) {
    RenderFrame(*renderer, 45);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_NEAR(player_->GetAudioTime(), 500, 0.001);

    // The child remains within [0, 150), but its parent has left [30, 90).
    RenderFrame(*renderer, 91);
    EXPECT_FALSE(player_->is_playing);
    EXPECT_EQ(player_->pause_count, 1);
    RenderFrame(*renderer, 120);
    EXPECT_EQ(player_->pause_count, 1);

    RenderFrame(*renderer, 30.6f);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_NEAR(player_->GetAudioTime(), 20, 0.001);
    EXPECT_EQ(player_->seek_count, 2);
  });
}

TEST_F(AudioLayerPlaybackTest, CulledParentKeepsAudioPlayingWithoutExtraSeek) {
  ASSERT_NO_FATAL_FAILURE(Load(kMovingAudioParentJSON));
  actor_->ActSync([this](auto& renderer) {
    ResolvedKeyPathElements elements;
    renderer->GetCompositionLayer()->ResolveKeyPath(
        AnimaXKeyPath(std::vector<std::string>{"parent"}), 0, elements,
        AnimaXKeyPath());
    ASSERT_EQ(elements.size(), 1u);
    auto* parent = static_cast<BaseLayer*>(elements.front().second);
    ASSERT_NE(parent->GetLayerModel().GetBlendMode(), BlendModeType::kNormal);

    const struct {
      float frame;
      float left;
    } steps[] = {{45.f, 0.f}, {46.25f, 200.f}, {47.25f, 0.f}};
    int expected_resume_count = 0;
    for (const auto& step : steps) {
      SCOPED_TRACE(step.frame);
      // Advance the hardware clock so only a lifecycle restart can cause seek.
      player_->current_time_ms = (step.frame - 30) * 1000.0 / 30;
      RenderFrame(*renderer, step.frame);

      RectF bounds;
      Matrix matrix;
      parent->GetBounds(bounds, matrix, false);
      EXPECT_FLOAT_EQ(bounds.GetLeft(), step.left);
      EXPECT_FLOAT_EQ(bounds.GetWidth(), 100.f);
      EXPECT_EQ(bounds.Intersect(RectF(0, 0, 100, 100)), step.left == 0.f);
      EXPECT_TRUE(player_->is_playing);
      EXPECT_EQ(player_->pause_count, 0);
      EXPECT_EQ(player_->seek_count, 1);
      EXPECT_EQ(player_->resume_count, ++expected_resume_count);
    }

    // Culling must not pause audio, but leaving the parent's interval must.
    RenderFrame(*renderer, 91);
    EXPECT_FALSE(player_->is_playing);
    EXPECT_EQ(player_->pause_count, 1);
    EXPECT_EQ(player_->resume_count, expected_resume_count);

    // Reenter while still offscreen. The 33 ms change is below the drift
    // threshold, so this verifies lifecycle positioning is still forced.
    RenderFrame(*renderer, 46.25f);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_EQ(player_->pause_count, 1);
    EXPECT_EQ(player_->seek_count, 2);
    EXPECT_NEAR(player_->GetAudioTime(), 16.25 * 1000.0 / 30, 0.001);
    EXPECT_EQ(player_->resume_count, expected_resume_count + 1);
  });
}

TEST_F(AudioLayerPlaybackTest, AudioStartsInsideParentWithEmptyVisualBounds) {
  constexpr const char* kAudioOnlyParentJSON = R"({
    "v":"5.6.10","w":100,"h":100,"fr":30,"ip":0,"op":180,
    "layers":[{"ty":0,"ind":1,"nm":"parent","refId":"nested",
      "sr":1,"ip":30,"op":90,"st":30,"w":100,"h":100,"bm":1,"ks":{}}],
    "assets":[{"id":"nested","layers":[
      {"ty":6,"ind":2,"nm":"audio","refId":"audio_0",
       "sr":1,"ip":0,"op":150,"st":0}
    ]}]
  })";
  ASSERT_NO_FATAL_FAILURE(Load(kAudioOnlyParentJSON));
  actor_->ActSync([this](auto& renderer) {
    RenderFrame(*renderer, 45);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_NEAR(player_->GetAudioTime(), 500, 0.001);
    EXPECT_EQ(player_->seek_count, 1);
    EXPECT_EQ(player_->resume_count, 1);
    EXPECT_EQ(player_->pause_count, 0);
  });
}

TEST_F(AudioLayerPlaybackTest, HiddenAudioOrParentStillPausesPlayback) {
  ASSERT_NO_FATAL_FAILURE(Load(kNestedAudioIntervalJSON));
  actor_->ActSync([this](auto& renderer) {
    RenderFrame(*renderer, 45);
    ASSERT_TRUE(player_->is_playing);

    LayerModel* layers[] = {model_->GetLayers().front().get(),
                            model_->GetPrecomps().at("nested").front().get()};
    int expected_pause_count = 0;
    for (auto* layer : layers) {
      SCOPED_TRACE(layer->GetName());
      layer->SetHidden(true);
      RenderFrame(*renderer, 45);
      EXPECT_FALSE(player_->is_playing);
      EXPECT_EQ(player_->pause_count, ++expected_pause_count);
      RenderFrame(*renderer, 45);
      EXPECT_EQ(player_->pause_count, expected_pause_count);

      layer->SetHidden(false);
      RenderFrame(*renderer, 45);
      EXPECT_TRUE(player_->is_playing);
      EXPECT_EQ(player_->seek_count, expected_pause_count + 1);
      EXPECT_EQ(player_->resume_count, expected_pause_count + 1);
    }
  });
}

TEST_F(AudioLayerPlaybackTest, SkippedRenderDoesNotCountAsAudioOutPoint) {
  ASSERT_NO_FATAL_FAILURE(Load(kAudioIntervalJSON));
  actor_->ActSync([this](auto& renderer) {
    RenderFrame(*renderer, 45);
    ASSERT_TRUE(player_->is_playing);
    surface_->has_canvas = false;
    RenderFrame(*renderer, 91);
    surface_->has_canvas = true;
    surface_->valid = false;
    RenderFrame(*renderer, 91);
    surface_->valid = true;
    renderer->MarkPlatformSurfaceAsInvalid(true);
    RenderFrame(*renderer, 91);
    renderer->MarkPlatformSurfaceAsInvalid(false);
    renderer->SetInBackground(true);
    RenderFrame(*renderer, 91);
    renderer->SetInBackground(false);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_EQ(player_->pause_count, 0);
    EXPECT_EQ(surface_->flush_count, 1);

    RenderFrame(*renderer, 91);
    EXPECT_FALSE(player_->is_playing);
    EXPECT_EQ(player_->pause_count, 1);
    EXPECT_EQ(surface_->flush_count, 2);

    RenderFrame(*renderer, 45);
    ASSERT_TRUE(player_->is_playing);
    renderer->CreateSurface(SurfaceCreationFactory(
        []() -> std::unique_ptr<AnimaXSurface> { return nullptr; }));
    RenderFrame(*renderer, 91);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_EQ(player_->pause_count, 1);
  });
}

TEST_F(AudioLayerPlaybackTest, GlobalStopAndMutePreventAudioFromRestarting) {
  ASSERT_NO_FATAL_FAILURE(Load(kAudioIntervalJSON));
  actor_->ActSync([this](auto& renderer) {
    RenderFrame(*renderer, 45);
    renderer->OnEnd();
    RenderFrame(*renderer, 0);
    RenderFrame(*renderer, 45);
    EXPECT_FALSE(player_->is_playing);
    EXPECT_EQ(player_->resume_count, 1);

    renderer->OnResume();
    RenderFrame(*renderer, 45);
    renderer->OnCancel();
    RenderFrame(*renderer, 45);
    EXPECT_FALSE(player_->is_playing);
    EXPECT_EQ(player_->resume_count, 2);

    renderer->SetMuted(true);
    renderer->OnResume();
    RenderFrame(*renderer, 45);
    EXPECT_FALSE(player_->is_playing);
    renderer->SetMuted(false);
    RenderFrame(*renderer, 120);
    EXPECT_FALSE(player_->is_playing);
    EXPECT_EQ(player_->resume_count, 2);
    RenderFrame(*renderer, 30.6f);
    EXPECT_TRUE(player_->is_playing);
    EXPECT_NEAR(player_->GetAudioTime(), 20, 0.001);
  });
}

}  // namespace
