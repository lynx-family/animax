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
  EXPECT_EQ(data_source.GetColor(), doc_data.GetColor());
  EXPECT_EQ(data_source.GetStrokeColor(), doc_data.GetStrokeColor());
  EXPECT_EQ(data_source.GetStrokeWidth(), doc_data.GetStrokeWidth());
  EXPECT_FLOAT_EQ(data_source.GetTracking(), doc_data.GetTracking() / 10.0f);
  EXPECT_STREQ(data_source.GetFontName().c_str(),
               doc_data.GetFontName().c_str());
  EXPECT_NE(data_source.GetFontAsset(), nullptr);
}

TEST_F(TextContentDataSourceTest, MultipleAnimators_ColorOverride) {
  TextLayerAnimations animations = CreateAnimations();
  const int32_t list_size = 3;
  animations.animator_property_list.resize(list_size);
  TextContentDataSource data_source(animations, font_manager_);
  for (int i = 0; i < list_size; ++i) {
    Color color(int32_t(0xFF) << (i * 8));
    auto color_keyframe = std::make_unique<MockColorKeyframeAnimation>();
    EXPECT_CALL(*color_keyframe, GetValue())
        .WillRepeatedly(::testing::ReturnRef(color));
    animations.animator_property_list[i].color = std::move(color_keyframe);
    EXPECT_EQ(data_source.GetColor(), color.GetInt());
  }
}

TEST_F(TextContentDataSourceTest, MultipleAnimators_StrokeColorOverride) {
  TextLayerAnimations animations = CreateAnimations();
  const int32_t list_size = 3;
  animations.animator_property_list.resize(list_size);
  TextContentDataSource data_source(animations, font_manager_);
  for (int i = 0; i < list_size; ++i) {
    Color stroke_color(int32_t(0xAA) << (i * 8));
    auto stroke_color_keyframe = std::make_unique<MockColorKeyframeAnimation>();
    EXPECT_CALL(*stroke_color_keyframe, GetValue())
        .WillRepeatedly(::testing::ReturnRef(stroke_color));
    animations.animator_property_list[i].stroke_color =
        std::move(stroke_color_keyframe);
    EXPECT_EQ(data_source.GetStrokeColor(), stroke_color.GetInt());
  }
}

TEST_F(TextContentDataSourceTest, MultipleAnimators_StrokeWidthOverride) {
  TextLayerAnimations animations = CreateAnimations();
  const int32_t list_size = 3;
  animations.animator_property_list.resize(list_size);
  TextContentDataSource data_source(animations, font_manager_);
  for (int i = 0; i < list_size; ++i) {
    Float stroke_width(0.5f * i);
    auto stroke_width_keyframe = std::make_unique<MockFloatKeyframeAnimation>();
    EXPECT_CALL(*stroke_width_keyframe, GetValue())
        .WillRepeatedly(::testing::ReturnRef(stroke_width));
    animations.animator_property_list[i].stroke_width =
        std::move(stroke_width_keyframe);
    EXPECT_FLOAT_EQ(data_source.GetStrokeWidth(), stroke_width.Get());
  }
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
