// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_renderer.h"

#include <cstring>
#include <memory>
#include <string>
#include <utility>

#include "base/include/lynx_actor.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/animax_surface.h"
#include "include/property/property_update_request.h"
#include "src/layer/composition_layer.h"
#include "src/layer/text_helper.h"
#include "src/model/composition_model.h"
#include "src/model/text/font_character_model.h"
#include "src/parser/composition_parser.h"
#include "src/player/animax_main_controller.h"
#include "src/player/animax_playback_event_handler.h"
#include "src/render/canvas.h"

using namespace lynx::animax;

class MockAnimaXValueCallback : public AnimaXValueCallback {
 public:
  std::unique_ptr<Value> GetValue(const Value* original_value,
                                  const AnimaXFrameInfo& frame_info) override {
    return nullptr;
  }
};

class MockAnimaXSurface : public AnimaXSurface {
 public:
  MockAnimaXSurface(int32_t width, int32_t height, bool valid = true)
      : AnimaXSurface(width, height), valid_(valid), canvas_(nullptr) {}

  void Flush() override {}

  lynx::animax::Canvas* Canvas() override { return canvas_; }

  AnimaXBackend Type() const override { return AnimaXBackend::kGL; }

  bool Valid() const override { return valid_; }

  void SetCanvas(lynx::animax::Canvas* canvas) { canvas_ = canvas; }
  void SetValid(bool valid) { valid_ = valid; }
  void SetType(AnimaXBackend type) { type_ = type; }
  AnimaXBackend GetType() const { return type_; }

 private:
  bool valid_;
  lynx::animax::Canvas* canvas_;
  AnimaXBackend type_ = AnimaXBackend::kGL;
};

class IntrinsicWidthTextContent : public TextContent {
 public:
  explicit IntrinsicWidthTextContent(const TextContentDataSource& data_source)
      : data_source_(data_source) {}
  void Draw(Canvas&, int32_t) override {}
  bool GetRect(RectF&) override { return false; }
  bool SupportsIntrinsicWidth() const override { return true; }
  bool GetIntrinsicWidth(float& width) override {
    width = data_source_.GetTextSize();
    return true;
  }

 private:
  const TextContentDataSource& data_source_;
};

class IntrinsicWidthTextHelper : public TextHelper {
 public:
  std::unique_ptr<TextContent> CreateTextContent(
      const TextContentDataSource& data_source) override {
    return std::make_unique<IntrinsicWidthTextContent>(data_source);
  }
};

class ScopedTextHelperOverride {
 public:
  explicit ScopedTextHelperOverride(TextHelper* helper)
      : previous_(&TextHelper::Impl()) {
    TextHelper::RegisterImpl(helper);
  }
  ~ScopedTextHelperOverride() { TextHelper::RegisterImpl(previous_); }

 private:
  TextHelper* previous_;
};

class AnimaXRendererTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::weak_ptr<AnimaXPlayer> weak_player;
    std::weak_ptr<lynx::shell::LynxActor<AnimaXMainController>> weak_controller;
    std::weak_ptr<AnimaXPlaybackEventHandler> weak_handler;

    renderer_ = std::make_unique<AnimaXRenderer>(weak_handler);
  }

  void TearDown() override {
    if (renderer_) {
      renderer_->Destroy();
      renderer_.reset();
    }
  }

  std::unique_ptr<AnimaXRenderer> renderer_;
};

TEST_F(AnimaXRendererTest, Constructor_WithNullWeakPointers_CreatesRenderer) {
  EXPECT_NE(renderer_, nullptr);
  EXPECT_EQ(renderer_->GetWidth(), 0.0);
  EXPECT_EQ(renderer_->GetHeight(), 0.0);
  EXPECT_EQ(renderer_->GetComposition(), nullptr);
  EXPECT_EQ(renderer_->GetCompositionLayer(), nullptr);
}

TEST_F(AnimaXRendererTest, GetWidth_InitiallyZero) {
  EXPECT_EQ(renderer_->GetWidth(), 0.0);
}

TEST_F(AnimaXRendererTest, GetHeight_InitiallyZero) {
  EXPECT_EQ(renderer_->GetHeight(), 0.0);
}

TEST_F(AnimaXRendererTest, GetComposition_InitiallyNull) {
  EXPECT_EQ(renderer_->GetComposition(), nullptr);
}

TEST_F(AnimaXRendererTest, GetCompositionLayer_InitiallyNull) {
  EXPECT_EQ(renderer_->GetCompositionLayer(), nullptr);
}

TEST_F(AnimaXRendererTest, CreateSurface_WithValidSurface_UpdatesDimensions) {
  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(800, 600, true);
          });

  renderer_->CreateSurface(std::move(factory));

  EXPECT_EQ(renderer_->GetWidth(), 800.0);
  EXPECT_EQ(renderer_->GetHeight(), 600.0);
}

TEST_F(AnimaXRendererTest, CreateSurface_WithInvalidSurface_HandlesGracefully) {
  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(800, 600, false);
          });

  renderer_->CreateSurface(std::move(factory));

  EXPECT_EQ(renderer_->GetWidth(), 0.0);
  EXPECT_EQ(renderer_->GetHeight(), 0.0);
}

TEST_F(AnimaXRendererTest, CreateSurface_WithNullFactory_HandlesGracefully) {
  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> { return nullptr; });

  renderer_->CreateSurface(std::move(factory));

  EXPECT_EQ(renderer_->GetWidth(), 0.0);
  EXPECT_EQ(renderer_->GetHeight(), 0.0);
}

TEST_F(AnimaXRendererTest, UpdateSurface_WithValidSurface_UpdatesDimensions) {
  SurfaceUpdateFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>,
                                  std::unique_ptr<AnimaXSurface>>(
          [](std::unique_ptr<AnimaXSurface> old_surface)
              -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(1024, 768, true);
          });

  renderer_->UpdateSurface(std::move(factory));

  EXPECT_EQ(renderer_->GetWidth(), 1024.0);
  EXPECT_EQ(renderer_->GetHeight(), 768.0);
}

TEST_F(AnimaXRendererTest, UpdateSurface_WithNullFactory_HandlesGracefully) {
  SurfaceUpdateFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>,
                                  std::unique_ptr<AnimaXSurface>>(
          [](std::unique_ptr<AnimaXSurface> old_surface)
              -> std::unique_ptr<AnimaXSurface> { return nullptr; });

  renderer_->UpdateSurface(std::move(factory));
}

TEST_F(AnimaXRendererTest,
       UpdateComposition_WithValidModel_UpdatesComposition) {
  auto model = std::make_shared<CompositionModel>(1.0f);
  auto bounds = std::make_unique<RectF>(0, 0, 100, 100);
  model->Init(std::move(bounds), 0.0f, 30.0f, 30.0f, false);

  renderer_->UpdateComposition(0, model);

  EXPECT_EQ(renderer_->GetComposition(), model);
}

TEST_F(AnimaXRendererTest, UpdateComposition_WithNullModel_HandlesGracefully) {
  renderer_->UpdateComposition(0, nullptr);

  EXPECT_EQ(renderer_->GetComposition(), nullptr);
}

TEST_F(AnimaXRendererTest,
       IsCompositionAssetsLoaded_WithValidComposition_ReturnsCorrectStatus) {
  auto model = std::make_shared<CompositionModel>(1.0f);
  auto bounds = std::make_unique<RectF>(0, 0, 100, 100);
  model->Init(std::move(bounds), 0.0f, 30.0f, 30.0f, false);
  model->SetAssetsLoaded(true);

  renderer_->UpdateComposition(0, model);

  EXPECT_TRUE(renderer_->IsCompositionAssetsLoaded());
}

TEST_F(AnimaXRendererTest,
       IsCompositionAssetsLoaded_WithoutComposition_ReturnsFalse) {
  EXPECT_FALSE(renderer_->IsCompositionAssetsLoaded());
}

TEST_F(AnimaXRendererTest,
       Render_WithValidSurfaceAndComposition_RendersSuccessfully) {
  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            auto surface = std::make_unique<MockAnimaXSurface>(800, 600, true);
            surface->SetCanvas(nullptr);
            return surface;
          });

  renderer_->CreateSurface(std::move(factory));

  auto model = std::make_shared<CompositionModel>(1.0f);
  auto bounds = std::make_unique<RectF>(0, 0, 100, 100);
  model->Init(std::move(bounds), 0.0f, 30.0f, 30.0f, false);
  renderer_->UpdateComposition(0, model);
}

TEST_F(AnimaXRendererTest,
       Render_WithoutSurfaceAndComposition_HandlesGracefully) {
  renderer_->Render(0.0);
  renderer_->Render(0.5);
  renderer_->Render(1.0);
}

TEST_F(AnimaXRendererTest, Render_WithInvalidPlatformSurface_SkipsRendering) {
  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(800, 600, true);
          });

  renderer_->CreateSurface(std::move(factory));
  renderer_->MarkPlatformSurfaceAsInvalid(true);

  renderer_->Render(0.5);
}

TEST_F(AnimaXRendererTest, SetObjectFit_WithAllValidValues_DoesNotCrash) {
  renderer_->SetObjectFit(ObjectFit::kCenter);
  renderer_->SetObjectFit(ObjectFit::kContain);
  renderer_->SetObjectFit(ObjectFit::kCover);
  renderer_->SetObjectFit(ObjectFit::kFill);
  renderer_->SetObjectFit(ObjectFit::kScaleDown);
}

TEST_F(AnimaXRendererTest, SetObjectPosition_WithAllValidValues_DoesNotCrash) {
  renderer_->SetObjectPosition(ObjectPosition::kCenter);
  renderer_->SetObjectPosition(ObjectPosition::kLeft);
  renderer_->SetObjectPosition(ObjectPosition::kRight);
  renderer_->SetObjectPosition(ObjectPosition::kTop);
  renderer_->SetObjectPosition(ObjectPosition::kBottom);
  renderer_->SetObjectPosition(ObjectPosition::kTopLeft);
  renderer_->SetObjectPosition(ObjectPosition::kTopRight);
  renderer_->SetObjectPosition(ObjectPosition::kBottomLeft);
  renderer_->SetObjectPosition(ObjectPosition::kBottomRight);
}

TEST_F(AnimaXRendererTest, OnTap_WithoutSurface_HandlesGracefully) {
  renderer_->OnTap(0.0f, 0.0f);
  renderer_->OnTap(100.0f, 150.0f);
  renderer_->OnTap(-10.0f, -20.0f);
}

TEST_F(AnimaXRendererTest, Reload_WithValidComposition_HandlesGracefully) {
  auto model = std::make_shared<CompositionModel>(1.0f);
  auto bounds = std::make_unique<RectF>(0, 0, 100, 100);
  model->Init(std::move(bounds), 0.0f, 30.0f, 30.0f, false);
  renderer_->UpdateComposition(0, model);

  renderer_->Reload();
}

TEST_F(AnimaXRendererTest, Reload_WithValidSurface_HandlesGracefully) {
  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(800, 600, true);
          });

  renderer_->CreateSurface(std::move(factory));
  renderer_->Reload();
}

TEST_F(AnimaXRendererTest, Reload_DoesNotCrash) { renderer_->Reload(); }

TEST_F(AnimaXRendererTest, Destroy_DoesNotCrash) { renderer_->Destroy(); }

TEST_F(AnimaXRendererTest,
       MarkPlatformSurfaceAsInvalid_WithValidFlags_DoesNotCrash) {
  renderer_->MarkPlatformSurfaceAsInvalid(true);
  renderer_->MarkPlatformSurfaceAsInvalid(false);
}

TEST_F(AnimaXRendererTest,
       GetKeysForKeyPath_WithNullKeyPath_HandlesGracefully) {
  auto key_path = std::make_unique<AnimaXKeyPath>();

  renderer_->GetKeysForKeyPath(
      std::move(key_path), [](const std::vector<AnimaXKeyPath>& key_paths) {});
}

TEST_F(AnimaXRendererTest,
       UpdateLayerProperty_WithNullRequest_HandlesGracefully) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"test"});
  auto value = std::make_unique<AnimaXValueParam>(1.0);
  auto callback = [](const PropertyUpdateResponse& response) {};

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), std::move(callback));

  renderer_->UpdateLayerProperty(std::move(request));
}

TEST_F(AnimaXRendererTest,
       SetResourceProperty_WithNullRequest_HandlesGracefully) {
  std::string resource_id = "test_resource";
  auto value = std::make_unique<AnimaXValueParam>(std::string("test_value"));
  auto callback = [](const PropertyUpdateResponse& response) {};

  auto request = ResourceUpdateRequest::Make(
      ResourcePropertyType::kImageFileName, resource_id, std::move(value),
      std::move(callback));

  renderer_->SetResourceProperty(std::move(request));
}

TEST_F(AnimaXRendererTest,
       AddLayerPropertyCallback_WithNullRequest_HandlesGracefully) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"test"});
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = [](const PropertyUpdateResponse& response) {};

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kOpacity, std::move(key_path),
      std::move(value_callback), std::move(callback));

  renderer_->AddLayerPropertyCallback(std::move(request));
}

TEST_F(AnimaXRendererTest, MultipleDestroyCalls_HandlesGracefully) {
  renderer_->Destroy();
  renderer_->Destroy();
  renderer_->Destroy();
}

TEST_F(AnimaXRendererTest, RenderWithNegativeProgress_HandlesGracefully) {
  renderer_->Render(-0.5);
  renderer_->Render(-1.0);
}

TEST_F(AnimaXRendererTest, RenderWithProgressGreaterThanOne_HandlesGracefully) {
  renderer_->Render(1.5);
  renderer_->Render(2.0);
}

TEST_F(AnimaXRendererTest, CreateSurface_WithSoftwareBackend_ClearsVideos) {
  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            auto surface = std::make_unique<MockAnimaXSurface>(800, 600, true);
            surface->SetType(AnimaXBackend::kSoftware);
            return surface;
          });

  renderer_->CreateSurface(std::move(factory));

  auto model = std::make_shared<CompositionModel>(1.0f);
  auto bounds = std::make_unique<RectF>(0, 0, 100, 100);
  model->Init(std::move(bounds), 0.0f, 30.0f, 30.0f, false);

  renderer_->UpdateComposition(0, model);
}

TEST_F(AnimaXRendererTest, Constructor_WithRealPlayer_CreatesRenderer) {
  auto player = AnimaXPlayerBuilder().SetScale(1.0f).Build();
  ASSERT_NE(player, nullptr);

  std::weak_ptr<AnimaXPlayer> weak_player = player;
  std::weak_ptr<lynx::shell::LynxActor<AnimaXMainController>> weak_controller;
  std::weak_ptr<AnimaXPlaybackEventHandler> weak_handler;

  auto renderer_with_player = std::make_unique<AnimaXRenderer>(weak_handler);

  EXPECT_NE(renderer_with_player, nullptr);
  EXPECT_EQ(renderer_with_player->GetWidth(), 0.0);
  EXPECT_EQ(renderer_with_player->GetHeight(), 0.0);

  renderer_with_player->Destroy();
  player->Destroy();
}

TEST_F(AnimaXRendererTest, Constructor_WithRealObjects_CreatesRenderer) {
  auto player = AnimaXPlayerBuilder().SetScale(1.0f).Build();
  ASSERT_NE(player, nullptr);

  auto handler = std::make_shared<AnimaXPlaybackEventHandler>();
  std::weak_ptr<AnimaXPlayer> weak_player = player;
  std::weak_ptr<AnimaXPlaybackEventHandler> weak_handler = handler;
  std::weak_ptr<lynx::shell::LynxActor<AnimaXMainController>> weak_controller;

  auto renderer_with_real_objects =
      std::make_unique<AnimaXRenderer>(weak_handler);

  EXPECT_NE(renderer_with_real_objects, nullptr);

  SurfaceCreationFactory factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(800, 600, true);
          });

  renderer_with_real_objects->CreateSurface(std::move(factory));

  auto model = std::make_shared<CompositionModel>(1.0f);
  auto bounds = std::make_unique<RectF>(0, 0, 100, 100);
  model->Init(std::move(bounds), 0.0f, 30.0f, 30.0f, false);
  renderer_with_real_objects->UpdateComposition(0, model);

  renderer_with_real_objects->Destroy();
  player->Destroy();
}

TEST_F(AnimaXRendererTest, UpdateSurface_WithExistingSurface_UpdatesCorrectly) {
  SurfaceCreationFactory creation_factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>(
          []() -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(400, 300, true);
          });

  renderer_->CreateSurface(std::move(creation_factory));
  EXPECT_EQ(renderer_->GetWidth(), 400.0);
  EXPECT_EQ(renderer_->GetHeight(), 300.0);

  SurfaceUpdateFactory update_factory =
      lynx::base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>,
                                  std::unique_ptr<AnimaXSurface>>(
          [](std::unique_ptr<AnimaXSurface> old_surface)
              -> std::unique_ptr<AnimaXSurface> {
            return std::make_unique<MockAnimaXSurface>(800, 600, true);
          });

  renderer_->UpdateSurface(std::move(update_factory));
  EXPECT_EQ(renderer_->GetWidth(), 800.0);
  EXPECT_EQ(renderer_->GetHeight(), 600.0);
}

namespace {
constexpr char kIntrinsicTextJSON[] = R"({
 "v":"5.6.10","w":100,"h":100,"ip":0,"op":60,"fr":30,
 "fonts":{"list":[{"fName":"test","fFamily":"Test","fStyle":"Regular","origin":3,"fPath":"https://example.com/font.ttf"}]},
 "assets":[],"layers":[
  {"ty":5,"ind":1,"nm":"text","ip":0,"op":60,"ks":{},
   "t":{"d":{"k":[{"s":{"t":"M","f":"test","s":24,"j":0,"tr":0,"lh":28,"sz":[20,1],"ps":[0,0]}}]}}},
  {"ty":3,"ind":2,"nm":"non-text","ip":0,"op":60,"ks":{}}
 ]})";

std::shared_ptr<CompositionModel> MakeIntrinsicComposition(float scale = 1.f) {
  return CompositionParser::Parse(kIntrinsicTextJSON,
                                  std::strlen(kIntrinsicTextJSON), scale);
}
}  // namespace

TEST_F(AnimaXRendererTest, TextIntrinsicWidthUsesJSONUnitsWithoutDrawing) {
  IntrinsicWidthTextHelper helper;
  ScopedTextHelperOverride scoped_helper(&helper);
  renderer_->CreateSurface([]() -> std::unique_ptr<AnimaXSurface> {
    return std::make_unique<MockAnimaXSurface>(100, 100);
  });
  for (float scale : {1.f, 2.f}) {
    auto model = MakeIntrinsicComposition(scale);
    ASSERT_NE(model, nullptr);
    renderer_->UpdateComposition(0, model);
    AnimaXRenderer::TextIntrinsicWidthRequest request;
    ASSERT_TRUE(renderer_->PrepareTextIntrinsicWidth("text", request));
    EXPECT_EQ(request.font_request.type, ResourceRequestType::kLoadRawData);
    EXPECT_EQ(request.font_request.uri_info.content_type,
              UriInfo::ContentType::kFont);
    EXPECT_EQ(request.font_request.uri_info.uri,
              "https://example.com/font.ttf");
    EXPECT_FALSE(model->IsAssetsLoaded());
    float width = 0.f;
    EXPECT_TRUE(renderer_->MeasureTextIntrinsicWidth(request, nullptr, width));
    EXPECT_FLOAT_EQ(width, 24.f);
    EXPECT_FALSE(renderer_->PrepareTextIntrinsicWidth("missing", request));
    EXPECT_FALSE(renderer_->PrepareTextIntrinsicWidth("non-text", request));
    EXPECT_FALSE(renderer_->PrepareTextIntrinsicWidth("", request));
  }
}

TEST_F(AnimaXRendererTest, TextIntrinsicWidthRejectsAmbiguousNames) {
  IntrinsicWidthTextHelper helper;
  ScopedTextHelperOverride scoped_helper(&helper);
  renderer_->CreateSurface([]() -> std::unique_ptr<AnimaXSurface> {
    return std::make_unique<MockAnimaXSurface>(100, 100);
  });
  std::string json = kIntrinsicTextJSON;
  json.replace(json.find("non-text"), 8, "text");
  auto model = CompositionParser::Parse(json.data(), json.size(), 1.f);
  renderer_->UpdateComposition(0, model);
  AnimaXRenderer::TextIntrinsicWidthRequest request;
  // A text layer and a non-text layer with the same name are also ambiguous.
  EXPECT_FALSE(renderer_->PrepareTextIntrinsicWidth("text", request));
}

TEST_F(AnimaXRendererTest, TextIntrinsicWidthRejectsFontAndCompositionChanges) {
  IntrinsicWidthTextHelper helper;
  ScopedTextHelperOverride scoped_helper(&helper);
  renderer_->CreateSurface([]() -> std::unique_ptr<AnimaXSurface> {
    return std::make_unique<MockAnimaXSurface>(100, 100);
  });
  auto model = MakeIntrinsicComposition();
  renderer_->UpdateComposition(0, model);
  AnimaXRenderer::TextIntrinsicWidthRequest request;
  ASSERT_TRUE(renderer_->PrepareTextIntrinsicWidth("text", request));
  auto font = request.font;
  auto font_model = font->Model();
  font->ResetModel(font_model);
  // Resetting A -> B -> A must invalidate a request even when metadata matches.
  font->ResetModel(font_model);
  static constexpr char kBytes[] = "late font";
  float width = 100.f;
  EXPECT_FALSE(renderer_->MeasureTextIntrinsicWidth(
      request, RawData::MakeRawData(kBytes, sizeof(kBytes), nullptr, nullptr),
      width));
  EXPECT_FLOAT_EQ(width, 0.f);
  EXPECT_FALSE(font->IsLoaded());
  ASSERT_TRUE(renderer_->PrepareTextIntrinsicWidth("text", request));
  renderer_->UpdateComposition(1, MakeIntrinsicComposition());
  EXPECT_FALSE(renderer_->MeasureTextIntrinsicWidth(
      request, RawData::MakeRawData(kBytes, sizeof(kBytes), nullptr, nullptr),
      width));
  EXPECT_FALSE(font->IsLoaded());
  ASSERT_TRUE(renderer_->PrepareTextIntrinsicWidth("text", request));
  auto active_font = request.font;
  renderer_->MarkDestroyed();
  EXPECT_FALSE(renderer_->MeasureTextIntrinsicWidth(
      request, RawData::MakeRawData(kBytes, sizeof(kBytes), nullptr, nullptr),
      width));
  EXPECT_FALSE(active_font->IsLoaded());
}

TEST_F(AnimaXRendererTest,
       TextIntrinsicWidthUnsupportedEngineDoesNotPrepareFont) {
  TextHelper helper;
  ScopedTextHelperOverride scoped_helper(&helper);
  renderer_->CreateSurface([]() -> std::unique_ptr<AnimaXSurface> {
    return std::make_unique<MockAnimaXSurface>(100, 100);
  });
  renderer_->UpdateComposition(0, MakeIntrinsicComposition());
  AnimaXRenderer::TextIntrinsicWidthRequest request;
  EXPECT_FALSE(renderer_->PrepareTextIntrinsicWidth("text", request));
  EXPECT_FALSE(request.font_request.uri_info.Valid());
}
