// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/layer/text_content_data_source.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/animation/base_keyframe_animation.h"
#include "src/animation/text_keyframe_animation.h"
#include "src/layer/font_asset_manager.h"
#include "src/layer/text_layer_animations.h"
#include "src/model/value/document_data.h"

using namespace lynx::animax;

class MockTextKeyframeAnimation : public TextKeyframeAnimation {
 public:
  MockTextKeyframeAnimation()
      : TextKeyframeAnimation(std::make_shared<KeyframeModelList>()) {}
  MOCK_METHOD(DocumentData&, GetValue, (), (override));
};

class MockColorKeyframeAnimation : public ColorKeyframeAnimation {
 public:
  MockColorKeyframeAnimation()
      : ColorKeyframeAnimation(std::make_shared<KeyframeModelList>()) {}
  MOCK_METHOD(Color&, GetValue, (), (override));
};

class MockFloatKeyframeAnimation : public FloatKeyframeAnimation {
 public:
  MockFloatKeyframeAnimation()
      : FloatKeyframeAnimation(std::make_shared<KeyframeModelList>()) {}
  MOCK_METHOD(Float&, GetValue, (), (override));
};

class TextContentDataSourceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    FontAssetModel asset_model{
        .family = "Arial",
        .name = "Arial",
        .style = "Regular",
        .ascent = 71.5988159179688f,
    };
    font_manager_.GetFontAssetMap().emplace(
        asset_model.family, std::make_shared<FontAsset>(asset_model));
  }

  TextLayerAnimations CreateAnimations() {
    TextLayerAnimations animations;
    auto text_keyframe = std::make_unique<MockTextKeyframeAnimation>();
    EXPECT_CALL(*text_keyframe, GetValue())
        .WillRepeatedly(::testing::ReturnRef(doc_data));
    animations.text_keyframe = std::move(text_keyframe);
    return animations;
  }

  DocumentData doc_data =
      DocumentData("Hello World",                      // text
                   "Arial",                            // font_name
                   14.0f,                              // size
                   DocumentJustification::kLeftAlign,  // justification
                   10.f,                               // tracking
                   1.2f,                               // line_height
                   0.0f,                               // baseline_shift
                   0xFF000000,                         // color (ARGB)
                   0x00000000,                         // stroke_color
                   0.5f,                               // stroke_width
                   false,                              // stroke_overfill
                   std::make_unique<PointF>(0, 0),     // box_position
                   std::make_unique<PointF>(500, 500)  // box_size
      );
  FontAssetManager font_manager_;
};

TEST_F(TextContentDataSourceTest, DefaultProperties) {
  TextLayerAnimations animations = CreateAnimations();
  TextContentDataSource data_source(animations, font_manager_);
  EXPECT_FLOAT_EQ(data_source.GetTracking(), doc_data.GetTracking() / 10.0f);
  EXPECT_STREQ(data_source.GetFontName().c_str(),
               doc_data.GetFontName().c_str());
  EXPECT_NE(data_source.GetFontAsset(), nullptr);
}

TEST_F(TextContentDataSourceTest, MultipleAnimators_TrackingAccumulate) {
  TextLayerAnimations animations = CreateAnimations();
  const int32_t list_size = 3;
  animations.animator_property_list.resize(list_size);
  TextContentDataSource data_source(animations, font_manager_);
  for (int i = 0; i < list_size; ++i) {
    Float tracking(0.1f);
    auto tracking_keyframe = std::make_unique<MockFloatKeyframeAnimation>();
    EXPECT_CALL(*tracking_keyframe, GetValue())
        .WillRepeatedly(::testing::ReturnRef(tracking));
    animations.animator_property_list[i].tracking =
        std::move(tracking_keyframe);
    float expected_tracking =
        doc_data.GetTracking() / 10.0f + tracking.Get() * (i + 1);
    EXPECT_FLOAT_EQ(data_source.GetTracking(), expected_tracking);
  }
}

TEST_F(TextContentDataSourceTest, GetAnimatorPropertyListReturnsOriginalList) {
  TextLayerAnimations animations = CreateAnimations();
  animations.animator_property_list.resize(2);
  animations.animator_property_list[0].skew =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(12.f);
  animations.animator_property_list[1].color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFFFF0000);
  TextContentDataSource data_source(animations, font_manager_);

  const auto& properties = data_source.GetAnimatorPropertyList();

  EXPECT_EQ(&properties, &animations.animator_property_list);
  ASSERT_EQ(properties.size(), 2u);
  ASSERT_NE(properties[0].skew, nullptr);
  EXPECT_FLOAT_EQ(properties[0].skew->GetValue().Get(), 12.f);
  ASSERT_NE(properties[1].color, nullptr);
  EXPECT_EQ(properties[1].color->GetValue().GetInt(), 0xFFFF0000);
}

TEST_F(TextContentDataSourceTest,
       GetRangeAnimatorPropertyListFiltersPropertiesWithoutRangeStyle) {
  TextLayerAnimations animations = CreateAnimations();
  animations.animator_property_list.resize(2);
  animations.animator_property_list[0].tracking =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(12.f);
  animations.animator_property_list[1].color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFFFF0000);
  TextContentDataSource data_source(animations, font_manager_);

  auto range_styles =
      data_source.GetRangeAnimatorPropertyList(doc_data.GetText().size());

  ASSERT_EQ(range_styles.size(), 1u);
  EXPECT_EQ(range_styles[0].segment_start, 0u);
  EXPECT_EQ(range_styles[0].segment_end, doc_data.GetText().size());
  EXPECT_FALSE(range_styles[0].color.IsEmpty());
  EXPECT_EQ(range_styles[0].color.GetInt(), 0xFFFF0000);
}

TEST_F(TextContentDataSourceTest,
       GetRangeAnimatorPropertyListSortsAndDeduplicatesBoundaries) {
  TextLayerAnimations animations = CreateAnimations();
  animations.animator_property_list.resize(3);

  auto& first_property = animations.animator_property_list[0];
  first_property.color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFFFF0000);
  first_property.range_selector = std::make_unique<RangeSelectorProperty>();
  first_property.range_selector->range_units = TextRangeUnits::kIndex;
  first_property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(5.f);
  first_property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(10.f);

  auto& second_property = animations.animator_property_list[1];
  second_property.skew =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(12.f);
  second_property.range_selector = std::make_unique<RangeSelectorProperty>();
  second_property.range_selector->range_units = TextRangeUnits::kIndex;
  second_property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(0.f);
  second_property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(5.f);

  auto& empty_property = animations.animator_property_list[2];
  empty_property.stroke_width =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(2.f);
  empty_property.range_selector = std::make_unique<RangeSelectorProperty>();
  empty_property.range_selector->range_units = TextRangeUnits::kIndex;
  empty_property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(2.f);
  empty_property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(2.f);

  TextContentDataSource data_source(animations, font_manager_);

  auto range_styles =
      data_source.GetRangeAnimatorPropertyList(doc_data.GetText().size());

  ASSERT_EQ(range_styles.size(), 2u);
  EXPECT_EQ(range_styles[0].segment_start, 0u);
  EXPECT_EQ(range_styles[0].segment_end, 5u);
  EXPECT_FLOAT_EQ(range_styles[0].skew, 12.f);
  EXPECT_EQ(range_styles[1].segment_start, 5u);
  EXPECT_EQ(range_styles[1].segment_end, 10u);
  EXPECT_FALSE(range_styles[1].color.IsEmpty());
  EXPECT_EQ(range_styles[1].color.GetInt(), 0xFFFF0000);
}

TEST_F(TextContentDataSourceTest,
       GetRangeAnimatorPropertyListClampsRangeToTextLength) {
  TextLayerAnimations animations = CreateAnimations();
  animations.animator_property_list.resize(1);
  auto& property = animations.animator_property_list[0];
  property.color =
      AnimationFactory::Make<ColorKeyframeAnimation, Color>(0xFFFF0000);
  property.range_selector = std::make_unique<RangeSelectorProperty>();
  property.range_selector->range_units = TextRangeUnits::kIndex;
  property.range_selector->offset =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(7.f);
  property.range_selector->start =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(5.f);
  property.range_selector->end =
      AnimationFactory::Make<FloatKeyframeAnimation, Float>(20.f);

  TextContentDataSource data_source(animations, font_manager_);

  auto range_styles = data_source.GetRangeAnimatorPropertyList(15);

  ASSERT_EQ(range_styles.size(), 1u);
  EXPECT_EQ(range_styles[0].segment_start, 12u);
  EXPECT_EQ(range_styles[0].segment_end, 15u);
  EXPECT_FALSE(range_styles[0].color.IsEmpty());
  EXPECT_EQ(range_styles[0].color.GetInt(), 0xFFFF0000);
}
