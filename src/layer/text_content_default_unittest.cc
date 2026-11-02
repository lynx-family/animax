// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/layer/text_content_default.h"

#include <functional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "skity/render/canvas.hpp"
#include "skity/text/font.hpp"
#include "src/layer/font_asset_manager.h"
#include "src/layer/text_layer_animations.h"
#include "src/model/value/document_data.h"
#include "src/render/canvas.h"
#include "src/resource/asset/font_asset.h"
#include "src/testing/font_test_data.h"

using namespace lynx::animax;

class MockTextKeyframeAnimation : public TextKeyframeAnimation {
 public:
  MockTextKeyframeAnimation()
      : TextKeyframeAnimation(std::make_shared<KeyframeModelList>()) {}
  MOCK_METHOD(DocumentData&, GetValue, (), (override));
};

namespace {

class MockTextSizeKeyframeAnimation : public FloatKeyframeAnimation {
 public:
  MockTextSizeKeyframeAnimation()
      : FloatKeyframeAnimation(std::make_shared<KeyframeModelList>()) {}
  MOCK_METHOD(Float&, GetValue, (), (override));
};

class RecordingTextCanvas : public skity::Canvas {
 public:
  std::function<void()> on_first_line;
  std::vector<float> text_sizes;
  std::vector<float> line_positions;
  std::vector<std::shared_ptr<skity::Typeface>> typefaces;

 protected:
  void OnClipPath(const skity::Path&, ClipOp) override {}
  void OnDrawPath(const skity::Path&, const skity::Paint&) override {}
  void OnSaveLayer(const skity::Rect&, const skity::Paint&) override {}
  void OnDrawBlob(const skity::TextBlob*, float, float,
                  const skity::Paint& paint) override {
    text_sizes.push_back(paint.GetTextSize());
    typefaces.push_back(paint.GetTypeface());
    if (text_sizes.size() == 1 && on_first_line) {
      on_first_line();
    }
  }
  void OnDrawImageRect(std::shared_ptr<skity::Image>, const skity::Rect&,
                       const skity::Rect&, const skity::SamplingOptions&,
                       const skity::Paint*) override {}
  void OnDrawGlyphs(uint32_t, const skity::GlyphID[], const float[],
                    const float[], const skity::Font&,
                    const skity::Paint&) override {}
  void OnDrawPaint(const skity::Paint&) override {}
  void OnSave() override {}
  void OnRestore() override {}
  void OnRestoreToCount(int) override {}
  void OnTranslate(float x, float) override { line_positions.push_back(x); }
  void OnFlush() override {}
  uint32_t OnGetWidth() const override { return 500; }
  uint32_t OnGetHeight() const override { return 500; }
};

class TextContentDefaultSnapshotTest : public ::testing::Test {
 protected:
  void SetUp() override {
    font_asset_ = std::make_shared<FontAsset>(
        FontAssetModel{.family = "Arial",
                       .name = "Arial",
                       .style = "Regular",
                       .path_origin = FontPathOrigin::kFontUrl});
    auto raw_data = MakeTestFontData();
    ASSERT_NE(nullptr, raw_data);
    ASSERT_GT(raw_data->Length(), 0u);
    font_asset_->SetRawData(std::move(raw_data));
    font_manager_.GetFontAssetMap().emplace("Arial", font_asset_);
  }

  DocumentData MakeDocumentData() {
    return DocumentData("W\rW", "Arial", 48.f, DocumentJustification::kCenter,
                        0, 60.f, 0.f, 0xFF000000, 0x00000000, 0.f, false,
                        std::make_unique<PointF>(0.f, 0.f),
                        std::make_unique<PointF>(500.f, 500.f));
  }

  void RefreshFont() {
    auto raw_data = MakeTestFontData();
    ASSERT_NE(nullptr, raw_data);
    ASSERT_GT(raw_data->Length(), 0u);
    font_asset_->SetRawData(std::move(raw_data));
    font_asset_->GetFontSnapshot()->SetTextSize(1.f);
  }

  std::shared_ptr<FontAsset> font_asset_;
  FontAssetManager font_manager_;
};

TEST_F(TextContentDefaultSnapshotTest, DrawKeepsOneFontAcrossResourceRefresh) {
  auto document_data = MakeDocumentData();
  auto text_keyframe = std::make_unique<MockTextKeyframeAnimation>();
  EXPECT_CALL(*text_keyframe, GetValue())
      .WillRepeatedly(::testing::ReturnRef(document_data));
  TextLayerAnimations animations;
  animations.text_keyframe = std::move(text_keyframe);
  TextContentDataSource data_source(animations, font_manager_);
  TextContentDefault content(data_source);

  auto snapshot = font_asset_->GetFontSnapshot();
  auto original_typeface = snapshot->GetFont().GetTypeface();
  std::weak_ptr<Font> previous_font = snapshot;
  snapshot.reset();
  RecordingTextCanvas recording_canvas;
  recording_canvas.on_first_line = [&] {
    RefreshFont();
    EXPECT_FALSE(previous_font.expired());
  };
  Canvas canvas(&recording_canvas, 500, 500, nullptr);

  content.Draw(canvas, 255);

  ASSERT_EQ(2u, recording_canvas.text_sizes.size());
  ASSERT_EQ(2u, recording_canvas.line_positions.size());
  EXPECT_FLOAT_EQ(48.f, recording_canvas.text_sizes[0]);
  EXPECT_FLOAT_EQ(48.f, recording_canvas.text_sizes[1]);
  EXPECT_EQ(original_typeface, recording_canvas.typefaces[0]);
  EXPECT_EQ(original_typeface, recording_canvas.typefaces[1]);
  EXPECT_FLOAT_EQ(recording_canvas.line_positions[0],
                  recording_canvas.line_positions[1]);
  EXPECT_FLOAT_EQ(1.f, font_asset_->GetFontSnapshot()->GetFont().GetSize());
  EXPECT_TRUE(previous_font.expired());
}

TEST_F(TextContentDefaultSnapshotTest,
       MeasurementKeepsOneFontAcrossResourceRefresh) {
  auto document_data = MakeDocumentData();
  auto snapshot = font_asset_->GetFontSnapshot();
  snapshot->SetTextSize(48.f);
  auto expected_width = snapshot->MeasureText("W");
  ASSERT_GT(expected_width, 0.f);
  std::weak_ptr<Font> previous_font = snapshot;
  snapshot.reset();

  auto text_keyframe = std::make_unique<MockTextKeyframeAnimation>();
  EXPECT_CALL(*text_keyframe, GetValue())
      .WillRepeatedly(::testing::ReturnRef(document_data));
  Float text_size(48.f);
  bool refreshed = false;
  auto text_size_callback = std::make_unique<MockTextSizeKeyframeAnimation>();
  EXPECT_CALL(*text_size_callback, GetValue())
      .WillRepeatedly(::testing::Invoke([&]() -> Float& {
        // A dynamic size callback can publish new font data during measurement.
        if (!refreshed) {
          refreshed = true;
          RefreshFont();
          EXPECT_FALSE(previous_font.expired());
        }
        return text_size;
      }));
  TextLayerAnimations animations;
  animations.text_keyframe = std::move(text_keyframe);
  animations.animator_property_list.emplace_back();
  animations.animator_property_list.back().text_size_callback =
      std::move(text_size_callback);
  TextContentDataSource data_source(animations, font_manager_);
  TextContentDefault content(data_source);

  RectF bounds;
  ASSERT_TRUE(content.GetRect(bounds));
  EXPECT_TRUE(refreshed);
  EXPECT_NEAR(expected_width, bounds.GetWidth(), 0.001f);
  EXPECT_FLOAT_EQ(1.f, font_asset_->GetFontSnapshot()->GetFont().GetSize());
  EXPECT_TRUE(previous_font.expired());
}

}  // namespace

TEST(TextContentDefaultTest, DrawCallsCanvasDrawText) {
  auto box_pos = std::make_unique<PointF>(0, 0);
  auto box_sz = std::make_unique<PointF>(500, 500);
  DocumentData doc_data("Hello World",                      // text
                        "Arial",                            // font_name
                        14.0f,                              // size
                        DocumentJustification::kLeftAlign,  // justification
                        0,                                  // tracking
                        1.2f,                               // line_height
                        0.0f,                               // baseline_shift
                        0xFF000000,                         // color (ARGB)
                        0x00000000,                         // stroke_color
                        0.5f,                               // stroke_width
                        false,                              // stroke_overfill
                        std::move(box_pos),                 // box_position
                        std::move(box_sz)                   // box_size
  );
  auto text_keyframe = std::make_unique<MockTextKeyframeAnimation>();
  EXPECT_CALL(*text_keyframe, GetValue())
      .WillRepeatedly(::testing::ReturnRef(doc_data));
  TextLayerAnimations animations;
  animations.text_keyframe = std::move(text_keyframe);

  FontAssetManager font_manager;
  FontAssetModel asset_model{
      .family = "Arial",
      .name = "Arial",
      .style = "Regular",
      .ascent = 71.5988159179688f,
  };
  auto font_asset = std::make_shared<FontAsset>(asset_model);
  auto raw_data = MakeTestFontData();
  ASSERT_NE(nullptr, raw_data);
  ASSERT_GT(raw_data->Length(), 0u);
  font_asset->SetRawData(std::move(raw_data));
  font_manager.GetFontAssetMap().emplace(asset_model.family, font_asset);

  TextContentDataSource data_source(animations, font_manager);
  TextContentDefault text_content(data_source);
  RecordingTextCanvas recording_canvas;
  Canvas canvas(&recording_canvas, 500, 500, nullptr);
  text_content.Draw(canvas, 255);
  ASSERT_EQ(1u, recording_canvas.text_sizes.size());
  EXPECT_FLOAT_EQ(14.f, recording_canvas.text_sizes.front());
  ASSERT_EQ(1u, recording_canvas.typefaces.size());
  EXPECT_NE(nullptr, recording_canvas.typefaces.front());
  EXPECT_EQ(font_asset->GetFontSnapshot()->GetFont().GetTypeface(),
            recording_canvas.typefaces.front());

  RectF rect;
  ASSERT_TRUE(text_content.GetRect(rect));
  EXPECT_FLOAT_EQ(0.f, rect.GetLeft());
  EXPECT_GT(rect.GetWidth(), 0.f);
  EXPECT_NEAR(1.2f, rect.GetHeight(), 0.001f);
}
