// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/layer/text_content_default.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/layer/font_asset_manager.h"
#include "src/layer/text_layer_animations.h"
#include "src/model/value/document_data.h"
#include "src/render/canvas.h"
#include "src/resource/asset/font_asset.h"

using namespace lynx::animax;

class MockTextKeyframeAnimation : public TextKeyframeAnimation {
 public:
  MockTextKeyframeAnimation()
      : TextKeyframeAnimation(std::make_shared<KeyframeModelList>()) {}
  MOCK_METHOD(DocumentData&, GetValue, (), (override));
};

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
  EXPECT_NE(font_asset->GetFont(), nullptr);
  font_manager.GetFontAssetMap().emplace(asset_model.family, font_asset);

  TextContentDataSource data_source(animations, font_manager);
  TextContentDefault text_content(data_source);
  Canvas canvas(nullptr, 100, 100, nullptr);
  text_content.Draw(canvas, 255);
}
