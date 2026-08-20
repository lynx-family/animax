// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

#include "src/layer/textra/textra_include.h"

#define private public
#include "src/layer/textra/text_content_layout_context.h"
#undef private

#include "gtest/gtest.h"
#include "include/resource/raw_data.h"
#include "skity/graphic/paint.hpp"
#include "src/animation/base_keyframe_animation.h"
#include "src/animation/text_keyframe_animation.h"
#include "src/base/thread/thread_assert.h"
#include "src/layer/text_content_data_source.h"
#include "src/layer/text_layer_animations.h"
#include "src/layer/textra/font_asset_manager_textra.h"
#include "src/model/value/document_data.h"
#include "src/resource/asset/font_asset.h"

namespace lynx {
namespace animax {
namespace {

const auto kFontAssetModel = FontAssetModel{.family = "Heiti SC",
                                            .name = "STHeitiSC-Light",
                                            .style = "Light",
                                            .ascent = 73.5989987850189};
constexpr char kTestFontPath[] =
    "example/darwin/ios/lynx_example/LynxExample/Resource/Fonts/"
    "notoserif.ttf";

std::unique_ptr<std::vector<uint8_t>> ReadTestFontData() {
  std::ifstream input(kTestFontPath, std::ios::binary);
  if (!input) {
    return nullptr;
  }
  return std::make_unique<std::vector<uint8_t>>(
      std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

class FixedTextKeyframeAnimation : public TextKeyframeAnimation {
 public:
  explicit FixedTextKeyframeAnimation(DocumentData& document_data)
      : TextKeyframeAnimation(std::make_shared<KeyframeModelList>()),
        document_data_(document_data) {}

  DocumentData& GetValue() override { return document_data_; }

 private:
  DocumentData& document_data_;
};

class TextContentLayoutContextTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ThreadAssert::Init(ThreadAssert::Type::kGPU);

    auto font_asset = std::make_shared<FontAsset>(kFontAssetModel);
    auto font_data = ReadTestFontData();
    ASSERT_NE(font_data, nullptr);
    ASSERT_FALSE(font_data->empty());
    font_asset->SetRawData(RawData::MakeRawData(
        font_data->data(), font_data->size(),
        [](const void* context) {
          delete static_cast<const std::vector<uint8_t>*>(context);
        },
        font_data.release()));
    font_manager_.GetFontAssetMap().emplace(kFontAssetModel.name, font_asset);
  }

  std::unique_ptr<TextContentDataSource> CreateDataSource(
      DocumentData& document_data) {
    animations_.text_keyframe =
        std::make_unique<FixedTextKeyframeAnimation>(document_data);
    return std::make_unique<TextContentDataSource>(animations_, font_manager_);
  }

  DocumentData CreateDocumentData(std::unique_ptr<PointF> box_size) {
    return DocumentData("Hello World",                      // text
                        kFontAssetModel.name,               // font_name
                        24.f,                               // size
                        DocumentJustification::kLeftAlign,  // justification
                        0,                                  // tracking
                        28.f,                               // line_height
                        0.f,                                // baseline_shift
                        0xFF000000,                         // color
                        0x00000000,                         // stroke_color
                        0.f,                                // stroke_width
                        false,                              // stroke_overfill
                        std::make_unique<PointF>(0.f, 0.f),
                        std::move(box_size));
  }

  skity::Paint CreatePaint() {
    skity::Paint paint;
    paint.SetFillColor(skity::ColorSetARGB(255, 0, 0, 0));
    return paint;
  }

  TextLayerAnimations animations_;
  FontAssetManagerTextra font_manager_;
};

TEST_F(TextContentLayoutContextTest, LayoutWithoutBoxUsesAtMostRegion) {
  auto document_data = CreateDocumentData(nullptr);
  auto data_source = CreateDataSource(document_data);
  TextContentLayoutContext context;

  context.Layout(*data_source, CreatePaint());

  auto* layout_region = context.GetLayoutRegion();
  ASSERT_NE(layout_region, nullptr);
  EXPECT_FALSE(context.IsBoxMode());
  EXPECT_GT(layout_region->GetLineCount(), 0u);
  EXPECT_GT(layout_region->GetLayoutedWidth(), 0.f);
}

TEST_F(TextContentLayoutContextTest, EmptyBoxFallsBackToAtMostRegion) {
  auto document_data = CreateDocumentData(std::make_unique<PointF>(0.f, 80.f));
  auto data_source = CreateDataSource(document_data);
  TextContentLayoutContext context;

  context.Layout(*data_source, CreatePaint());

  auto* layout_region = context.GetLayoutRegion();
  ASSERT_NE(layout_region, nullptr);
  EXPECT_FALSE(context.IsBoxMode());
  EXPECT_GT(layout_region->GetLineCount(), 0u);
  EXPECT_GT(layout_region->GetLayoutedWidth(), 0.f);
}

TEST_F(TextContentLayoutContextTest, LayoutWithBoxUsesFixedRegion) {
  auto document_data =
      CreateDocumentData(std::make_unique<PointF>(240.f, 80.f));
  auto data_source = CreateDataSource(document_data);
  TextContentLayoutContext context;

  context.Layout(*data_source, CreatePaint());

  auto* layout_region = context.GetLayoutRegion();
  ASSERT_NE(layout_region, nullptr);
  EXPECT_TRUE(context.IsBoxMode());
  EXPECT_FLOAT_EQ(layout_region->GetPageWidth(), 240.f);
  EXPECT_FLOAT_EQ(layout_region->GetPageHeight(), 80.f);
  EXPECT_GT(layout_region->GetLineCount(), 0u);
}

TEST_F(TextContentLayoutContextTest, LayoutWithRangeSelectorAppliesRangeStyle) {
  auto document_data =
      CreateDocumentData(std::make_unique<PointF>(240.f, 80.f));
  animations_.animator_property_list.resize(1);
  auto& property = animations_.animator_property_list[0];
  property.skew = AnimationFactory::Make<FloatKeyframeAnimation, Float>(12.f);
  property.color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFFFF0000);
  property.stroke_color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFF0000FF);
  property.stroke_width =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(2.f);
  property.range_selector = std::make_unique<RangeSelectorProperty>();
  property.range_selector->range_units = TextRangeUnits::kIndex;
  property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(1.f);
  property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(5.f);

  auto data_source = CreateDataSource(document_data);
  TextContentLayoutContext context;

  context.Layout(*data_source, CreatePaint());

  auto* layout_region = context.GetLayoutRegion();
  ASSERT_NE(layout_region, nullptr);
  EXPECT_TRUE(context.IsBoxMode());
  EXPECT_FLOAT_EQ(layout_region->GetPageWidth(), 240.f);
  EXPECT_FLOAT_EQ(layout_region->GetPageHeight(), 80.f);
  EXPECT_GT(layout_region->GetLineCount(), 0u);
}

TEST_F(TextContentLayoutContextTest,
       RangeColorUsesOwnAlphaAndLayerAlphaWhenBaseFillIsTransparent) {
  auto document_data =
      CreateDocumentData(std::make_unique<PointF>(240.f, 80.f));
  animations_.animator_property_list.resize(1);
  auto& property = animations_.animator_property_list[0];
  property.color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0x80FF0000);
  property.stroke_color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0x400000FF);
  property.stroke_width =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(2.f);
  property.range_selector = std::make_unique<RangeSelectorProperty>();
  property.range_selector->range_units = TextRangeUnits::kIndex;
  property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(1.f);
  property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(5.f);

  auto data_source = CreateDataSource(document_data);
  skity::Paint paint;
  paint.SetFillColor(skity::ColorSetARGB(0, 0, 0, 0));
  constexpr int32_t kLayerAlpha = 100;
  TextContentLayoutContext context;

  context.Layout(*data_source, paint, kLayerAlpha);

  ASSERT_EQ(context.range_painters_.size(), 1u);
  auto* range_paint = context.range_painters_[0]->platform_painter_.get();
  ASSERT_NE(range_paint, nullptr);
  EXPECT_FLOAT_EQ(range_paint->GetFillColor().a,
                  static_cast<uint8_t>(128 * kLayerAlpha / 255.f) / 255.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeColor().a,
                  static_cast<uint8_t>(64 * kLayerAlpha / 255.f) / 255.f);
}

TEST_F(TextContentLayoutContextTest, RangeFillColorPreservesBaseStroke) {
  auto document_data =
      CreateDocumentData(std::make_unique<PointF>(240.f, 80.f));
  animations_.animator_property_list.resize(1);
  auto& property = animations_.animator_property_list[0];
  property.color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFF00FF00);
  property.range_selector = std::make_unique<RangeSelectorProperty>();
  property.range_selector->range_units = TextRangeUnits::kIndex;
  property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(1.f);
  property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(5.f);

  auto data_source = CreateDataSource(document_data);
  skity::Paint paint;
  paint.SetFillColor(skity::ColorSetARGB(255, 255, 0, 0));
  paint.SetStrokeColor(skity::ColorSetARGB(255, 0, 0, 255));
  paint.SetStrokeWidth(10.f);
  paint.SetStyle(skity::Paint::kStrokeThenFill_Style);
  TextContentLayoutContext context;

  context.Layout(*data_source, paint);

  ASSERT_EQ(context.range_painters_.size(), 1u);
  auto* range_paint = context.range_painters_[0]->platform_painter_.get();
  ASSERT_NE(range_paint, nullptr);
  EXPECT_EQ(skity::Paint::kStrokeThenFill_Style, range_paint->GetStyle());
  EXPECT_FLOAT_EQ(range_paint->GetStrokeWidth(), 10.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeColor().b, 1.f);
  EXPECT_FLOAT_EQ(range_paint->GetFillColor().g, 1.f);
}

TEST_F(TextContentLayoutContextTest,
       RangeFillColorAppliesAndPreservesRangeStroke) {
  auto document_data =
      DocumentData("Hello World", kFontAssetModel.name, 24.f,
                   DocumentJustification::kLeftAlign, 0, 28.f, 0.f, 0x00000000,
                   0xFF0000FF, 3.f, false, std::make_unique<PointF>(0.f, 0.f),
                   std::make_unique<PointF>(240.f, 80.f));
  animations_.animator_property_list.resize(2);
  auto& fill_property = animations_.animator_property_list[0];
  fill_property.color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFF00FF00);
  fill_property.range_selector = std::make_unique<RangeSelectorProperty>();
  fill_property.range_selector->range_units = TextRangeUnits::kIndex;
  fill_property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(0.f);
  fill_property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(11.f);
  auto& stroke_property = animations_.animator_property_list[1];
  stroke_property.stroke_color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFFFF00FF);
  stroke_property.range_selector = std::make_unique<RangeSelectorProperty>();
  stroke_property.range_selector->range_units = TextRangeUnits::kIndex;
  stroke_property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(0.f);
  stroke_property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(11.f);

  auto data_source = CreateDataSource(document_data);
  skity::Paint paint;
  paint.SetFillColor(skity::ColorSetARGB(0, 0, 0, 0));
  paint.SetStrokeColor(skity::ColorSetARGB(255, 0, 0, 255));
  paint.SetStrokeWidth(3.f);
  paint.SetStyle(skity::Paint::kStrokeThenFill_Style);
  TextContentLayoutContext context;

  context.Layout(*data_source, paint);

  ASSERT_EQ(context.range_painters_.size(), 1u);
  auto* range_paint = context.range_painters_[0]->platform_painter_.get();
  ASSERT_NE(range_paint, nullptr);
  EXPECT_FLOAT_EQ(range_paint->GetFillColor().g, 1.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeColor().r, 1.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeColor().b, 1.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeWidth(), 3.f);
}

TEST_F(TextContentLayoutContextTest, RangeFillColorEnablesFillOnStrokePaint) {
  auto document_data =
      CreateDocumentData(std::make_unique<PointF>(240.f, 80.f));
  animations_.animator_property_list.resize(1);
  auto& property = animations_.animator_property_list[0];
  property.color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFF00FF00);
  property.range_selector = std::make_unique<RangeSelectorProperty>();
  property.range_selector->range_units = TextRangeUnits::kIndex;
  property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(0.f);
  property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(11.f);

  auto data_source = CreateDataSource(document_data);
  skity::Paint paint;
  paint.SetFillColor(skity::ColorSetARGB(0, 0, 0, 0));
  paint.SetStrokeColor(skity::ColorSetARGB(255, 0, 0, 255));
  paint.SetStrokeWidth(3.f);
  paint.SetStyle(skity::Paint::kStroke_Style);
  TextContentLayoutContext context;

  context.Layout(*data_source, paint);

  ASSERT_EQ(context.range_painters_.size(), 1u);
  auto* range_paint = context.range_painters_[0]->platform_painter_.get();
  ASSERT_NE(range_paint, nullptr);
  EXPECT_EQ(skity::Paint::kStrokeThenFill_Style, range_paint->GetStyle());
  EXPECT_FLOAT_EQ(range_paint->GetFillColor().g, 1.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeColor().b, 1.f);
}

TEST_F(TextContentLayoutContextTest,
       RangeStrokeColorKeepsStrokeStyleWhenFillIsInvisible) {
  auto document_data =
      CreateDocumentData(std::make_unique<PointF>(240.f, 80.f));
  animations_.animator_property_list.resize(1);
  auto& property = animations_.animator_property_list[0];
  property.stroke_color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFFFF00FF);
  property.range_selector = std::make_unique<RangeSelectorProperty>();
  property.range_selector->range_units = TextRangeUnits::kIndex;
  property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(0.f);
  property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(11.f);

  auto data_source = CreateDataSource(document_data);
  skity::Paint paint;
  paint.SetFillColor(skity::ColorSetARGB(0, 0, 0, 0));
  paint.SetStrokeColor(skity::ColorSetARGB(255, 0, 0, 255));
  paint.SetStrokeWidth(3.f);
  paint.SetStyle(skity::Paint::kStroke_Style);
  TextContentLayoutContext context;

  context.Layout(*data_source, paint);

  ASSERT_EQ(context.range_painters_.size(), 1u);
  auto* range_paint = context.range_painters_[0]->platform_painter_.get();
  ASSERT_NE(range_paint, nullptr);
  EXPECT_EQ(skity::Paint::kStroke_Style, range_paint->GetStyle());
  EXPECT_FLOAT_EQ(range_paint->GetFillColor().a, 0.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeColor().r, 1.f);
  EXPECT_FLOAT_EQ(range_paint->GetStrokeColor().b, 1.f);
}

}  // namespace
}  // namespace animax
}  // namespace lynx
