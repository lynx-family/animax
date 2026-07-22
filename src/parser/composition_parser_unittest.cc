// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/parser/composition_parser.h"

#include <cstring>
#include <memory>

#include "gtest/gtest.h"
#include "src/base/util/composition_frame_util.h"
#include "src/model/text/text_gradient_model.h"

using namespace lynx::animax;

namespace {

std::shared_ptr<CompositionModel> ParseComposition(const char* json,
                                                   bool enable_audio = false) {
  return CompositionParser::Parse(json, std::strlen(json), 1.0f, enable_audio);
}

TEST(CompositionParserTest, BindsRootAndPrecompTextGradients) {
  static constexpr const char* kLottie = R"({
    "meta":{"custom":{"animax":{"v":1,"fs":{"tg":[
      {"ind":101,"t":1,"s":[0.1,0.2],"e":[0.8,0.9],
       "g":{"k":[
         0,1,0,0, 1,0,0,1,
         0,1, 0.5,0.25, 1,0.5
       ],"p":2}},
      {"refId":"nested","ind":101,"t":2,"s":[0.25,0.5],
       "rxy":[0.4,1.2],
       "g":{"p":2,"k":[0,0,1,0, 1,1,1,0]}}
    ]}}}},
    "v":"5.6.10","fr":30,"ip":0,"op":60,"w":100,"h":100,
    "layers":[{"ind":101,"ty":5,"nm":"root-text"}],
    "assets":[{"id":"nested","layers":[
      {"ind":101,"ty":5,"nm":"nested-text"}
    ]}],
    "markers":[]
  })";

  auto composition = ParseComposition(kLottie);

  ASSERT_NE(nullptr, composition);
  auto root_layer_it = composition->GetLayerMap().find(101);
  ASSERT_NE(composition->GetLayerMap().end(), root_layer_it);
  const auto* root_gradient = root_layer_it->second->GetTextGradientModel();
  ASSERT_NE(nullptr, root_gradient);
  ASSERT_EQ(1u, root_gradient->GetItems().size());
  const auto& linear = *root_gradient->GetItems()[0];
  EXPECT_EQ(GradientType::kLinear, linear.GetType());
  EXPECT_FLOAT_EQ(0.1f, linear.GetStartPoint().GetX());
  EXPECT_FLOAT_EQ(0.2f, linear.GetStartPoint().GetY());
  ASSERT_NE(nullptr, linear.GetEndPoint());
  EXPECT_FLOAT_EQ(0.8f, linear.GetEndPoint()->GetX());
  EXPECT_FLOAT_EQ(0.9f, linear.GetEndPoint()->GetY());
  EXPECT_EQ(nullptr, linear.GetRadiusXY());

  const auto& linear_colors = linear.GetGradientColor();
  ASSERT_EQ(3, linear_colors.GetSize());
  ASSERT_NE(nullptr, linear_colors.GetPositions());
  ASSERT_NE(nullptr, linear_colors.GetColors());
  EXPECT_FLOAT_EQ(0.f, linear_colors.GetPositions()[0]);
  EXPECT_FLOAT_EQ(0.5f, linear_colors.GetPositions()[1]);
  EXPECT_FLOAT_EQ(1.f, linear_colors.GetPositions()[2]);
  EXPECT_EQ(Color::ToInt(255, 255, 0, 0), linear_colors.GetColors()[0]);
  EXPECT_EQ(63, Color(linear_colors.GetColors()[1]).GetA());
  EXPECT_EQ(Color::ToInt(127, 0, 0, 255), linear_colors.GetColors()[2]);

  auto precomp_it = composition->GetPrecomps().find("nested");
  ASSERT_NE(composition->GetPrecomps().end(), precomp_it);
  ASSERT_EQ(1u, precomp_it->second.size());
  const auto* nested_layer = precomp_it->second[0].get();
  ASSERT_NE(nullptr, nested_layer);
  const auto* nested_gradient = nested_layer->GetTextGradientModel();
  ASSERT_NE(nullptr, nested_gradient);
  ASSERT_EQ(1u, nested_gradient->GetItems().size());
  const auto& radial = *nested_gradient->GetItems()[0];
  EXPECT_EQ(GradientType::kRadial, radial.GetType());
  EXPECT_FLOAT_EQ(0.25f, radial.GetStartPoint().GetX());
  EXPECT_FLOAT_EQ(0.5f, radial.GetStartPoint().GetY());
  EXPECT_EQ(nullptr, radial.GetEndPoint());
  ASSERT_NE(nullptr, radial.GetRadiusXY());
  EXPECT_FLOAT_EQ(0.4f, radial.GetRadiusXY()->GetX());
  EXPECT_FLOAT_EQ(1.2f, radial.GetRadiusXY()->GetY());
}

TEST(CompositionParserTest, BindsUnorderedTextGradientMembers) {
  static constexpr const char* kLottie = R"({
    "v":"5.6.10","fr":30,"ip":0,"op":60,"w":100,"h":100,
    "layers":[{"ind":3,"ty":5,"nm":"unordered-text"}],"assets":[],
    "meta":{"custom":{"animax":{"v":1,"fs":{"tg":[
      {"g":{"k":[
         0,1,0,0, 1,0,0,1,
         0,1, 1,0.5
       ],"p":2},"e":[1,1],"s":[0,0],"t":1,"ind":3}
    ]}}}},"markers":[]
  })";

  auto composition = ParseComposition(kLottie);

  ASSERT_NE(nullptr, composition);
  auto layer_it = composition->GetLayerMap().find(3);
  ASSERT_NE(composition->GetLayerMap().end(), layer_it);
  const auto* gradient = layer_it->second->GetTextGradientModel();
  ASSERT_NE(nullptr, gradient);
  ASSERT_EQ(1u, gradient->GetItems().size());
  const auto& item = *gradient->GetItems()[0];
  EXPECT_EQ(GradientType::kLinear, item.GetType());
  ASSERT_NE(nullptr, item.GetEndPoint());
  EXPECT_FLOAT_EQ(1.f, item.GetEndPoint()->GetX());
  EXPECT_FLOAT_EQ(1.f, item.GetEndPoint()->GetY());
  const auto& colors = item.GetGradientColor();
  ASSERT_EQ(2, colors.GetSize());
  EXPECT_EQ(Color::ToInt(255, 255, 0, 0), colors.GetColors()[0]);
  EXPECT_EQ(Color::ToInt(127, 0, 0, 255), colors.GetColors()[1]);
}

TEST(CompositionParserTest, AppendsMultipleGradientsForDuplicateRootTarget) {
  static constexpr const char* kLottie = R"({
    "meta":{"custom":{"animax":{"v":1,"fs":{"tg":[
      {"ind":7,"t":1,"s":[0,0],"e":[1,0],
       "g":{"p":2,"k":[0,1,0,0, 1,0,0,1]}},
      {"ind":7,"t":1,"s":[0.2,0.3],"e":[0.6,0.7],
       "g":{"p":2,"k":[0,0,1,0, 1,1,1,0]}},
      {"ind":7,"t":2,"s":[0.9,0.8],"rxy":[0.7,0.6],
       "g":{"p":2,"k":[0,1,0,1, 1,0,1,1]}}
    ]}}}},
    "v":"5.6.10","fr":30,"ip":0,"op":60,"w":100,"h":100,
    "assets":[],"layers":[{"ind":7,"ty":5}],"markers":[]
  })";

  auto composition = ParseComposition(kLottie);

  ASSERT_NE(nullptr, composition);
  auto layer_it = composition->GetLayerMap().find(7);
  ASSERT_NE(composition->GetLayerMap().end(), layer_it);
  const auto* gradient = layer_it->second->GetTextGradientModel();
  ASSERT_NE(nullptr, gradient);
  ASSERT_EQ(3u, gradient->GetItems().size());
  const auto& item = *gradient->GetItems()[1];
  EXPECT_EQ(GradientType::kLinear, item.GetType());
  EXPECT_FLOAT_EQ(0.2f, item.GetStartPoint().GetX());
  EXPECT_FLOAT_EQ(0.3f, item.GetStartPoint().GetY());
  ASSERT_NE(nullptr, item.GetEndPoint());
  EXPECT_FLOAT_EQ(0.6f, item.GetEndPoint()->GetX());
  EXPECT_FLOAT_EQ(0.7f, item.GetEndPoint()->GetY());
  EXPECT_EQ(Color::ToInt(255, 0, 255, 0),
            item.GetGradientColor().GetColors()[0]);
}

TEST(CompositionParserTest, ParsesCssConicTextGradient) {
  static constexpr const char* kLottie = R"({
    "meta":{"custom":{"animax":{"v":1,"fs":{"tg":[
      {"ind":1,"t":3,"s":[0.5,0.5],"a":90,
       "g":{"p":2,"k":[0,1,0,0, 1,0,0,1]}}
    ]}}}},
    "v":"5.6.10","fr":30,"ip":0,"op":60,"w":100,"h":100,
    "assets":[],"layers":[{"ind":1,"ty":5,"nm":"conic-text"}],
    "markers":[]
  })";

  auto composition = ParseComposition(kLottie);

  ASSERT_NE(nullptr, composition);
  auto layer_it = composition->GetLayerMap().find(1);
  ASSERT_NE(composition->GetLayerMap().end(), layer_it);
  const auto* gradient = layer_it->second->GetTextGradientModel();
  ASSERT_NE(nullptr, gradient);
  ASSERT_EQ(1u, gradient->GetItems().size());
  const auto& item = *gradient->GetItems()[0];
  EXPECT_EQ(GradientType::kConic, item.GetType());
  EXPECT_FLOAT_EQ(0.5f, item.GetStartPoint().GetX());
  EXPECT_FLOAT_EQ(0.5f, item.GetStartPoint().GetY());
  ASSERT_TRUE(item.GetStartAngle().has_value());
  EXPECT_FLOAT_EQ(90.f, item.GetStartAngle().value());
  EXPECT_EQ(nullptr, item.GetEndPoint());
  EXPECT_EQ(nullptr, item.GetRadiusXY());
}

TEST(CompositionParserTest, SkipsMalformedAndNonTextGradientTargets) {
  static constexpr const char* kLottie = R"({
    "meta":{"custom":{"animax":{"v":1,"fs":{"tg":[
      17,
      {"ind":2,"t":1,"s":[0,0],"e":[1,1],
       "g":{"p":2,"k":[0,1,0,0, 1,0,0,1]}},
      {"refId":"missing","ind":9,"t":1,"s":[0,0],"e":[1,1],
       "g":{"p":2,"k":[0,1,0,0, 1,0,0,1]}},
      {"refId":123,"ind":1,"t":1,"s":[0,0],"e":[1,1],
       "g":{"p":2,"k":[0,1,0,0, 1,0,0,1]}},
      {"refId":null,"ind":1,"t":1,"s":[0,0],"e":[1,1],
       "g":{"p":2,"k":[0,1,0,0, 1,0,0,1]}},
      {"ind":3,"t":1,"s":[0,0],"e":[1,1],
       "g":{"p":1073741824,"k":[]}},
      {"ind":1,"t":1,"s":[0,0],"e":[1,1],
       "g":{"p":2,"k":[0,1,0,0, 1,0,0,1, 0]}},
      {"ind":1,"t":2,"s":[0.5,0.5],"e":[1,0.5],
       "g":{"p":2,"k":[0,1,1,1, 1,0,0,0]}}
    ]}}}},
    "v":"5.6.10","fr":30,"ip":0,"op":60,"w":100,"h":100,
    "assets":[],"layers":[
      {"ind":1,"ty":5,"nm":"text"},
      {"ind":2,"ty":4,"nm":"shape"},
      {"ind":3,"ty":5,"nm":"large-color-point-count"}
    ],"markers":[]
  })";

  auto composition = ParseComposition(kLottie);

  ASSERT_NE(nullptr, composition);
  auto text_layer_it = composition->GetLayerMap().find(1);
  ASSERT_NE(composition->GetLayerMap().end(), text_layer_it);
  const auto* text_gradient = text_layer_it->second->GetTextGradientModel();
  ASSERT_NE(nullptr, text_gradient);
  ASSERT_EQ(1u, text_gradient->GetItems().size());
  const auto& valid_item = *text_gradient->GetItems()[0];
  EXPECT_EQ(GradientType::kRadial, valid_item.GetType());
  ASSERT_NE(nullptr, valid_item.GetEndPoint());
  EXPECT_FLOAT_EQ(1.f, valid_item.GetEndPoint()->GetX());
  EXPECT_FLOAT_EQ(0.5f, valid_item.GetEndPoint()->GetY());
  EXPECT_EQ(nullptr, valid_item.GetRadiusXY());

  auto shape_layer_it = composition->GetLayerMap().find(2);
  ASSERT_NE(composition->GetLayerMap().end(), shape_layer_it);
  EXPECT_EQ(LayerType::kShape, shape_layer_it->second->GetLayerType());
  EXPECT_EQ(nullptr, shape_layer_it->second->GetTextGradientModel());

  auto large_count_layer_it = composition->GetLayerMap().find(3);
  ASSERT_NE(composition->GetLayerMap().end(), large_count_layer_it);
  EXPECT_EQ(nullptr, large_count_layer_it->second->GetTextGradientModel());
  EXPECT_EQ(composition->GetPrecomps().end(),
            composition->GetPrecomps().find("missing"));
}

TEST(CompositionParserTest, KeepsPlaybackAndTimelineValues) {
  static constexpr const char* kLottie = R"({
    "v":"5.6.10","fr":25,"ip":0,"op":200,"w":100,"h":100,
    "nm":"timeline","ddd":0,"assets":[],"layers":[],"markers":[]
  })";

  auto model = ParseComposition(kLottie);

  ASSERT_NE(nullptr, model);
  EXPECT_FLOAT_EQ(0.0f, model->GetStartFrame());
  EXPECT_FLOAT_EQ(199.99f, model->GetEndFrame());
  EXPECT_FLOAT_EQ(199.99f, model->GetDurationFrames());
  EXPECT_EQ(7999, model->GetDuration());
  EXPECT_FLOAT_EQ(200.0f, model->GetTimelineEndFrame());
  EXPECT_NEAR(200.0, model->GetTimelineDurationFrames(), 0.00001);
  EXPECT_EQ(8000, model->GetTimelineDurationMs());
}

TEST(CompositionParserTest, KeepsPlaybackValuesForFractionalTimeline) {
  static constexpr const char* kLottie = R"({
    "v":"5.6.10","fr":25,"ip":-10.5,"op":70.25,"w":100,"h":100,
    "nm":"timeline","ddd":0,"assets":[],"layers":[],"markers":[]
  })";

  auto model = ParseComposition(kLottie);

  ASSERT_NE(nullptr, model);
  EXPECT_FLOAT_EQ(-10.5f, model->GetStartFrame());
  EXPECT_FLOAT_EQ(70.24f, model->GetEndFrame());
  EXPECT_FLOAT_EQ(80.74f, model->GetDurationFrames());
  EXPECT_EQ(3229, model->GetDuration());
  EXPECT_FLOAT_EQ(70.25f, model->GetTimelineEndFrame());
  EXPECT_NEAR(80.75, model->GetTimelineDurationFrames(), 0.00001);
  EXPECT_EQ(3230, model->GetTimelineDurationMs());
}

TEST(CompositionParserTest, ReportsExactMillisecondsForIntegerTimeline) {
  static constexpr const char* kLottie = R"({
    "v":"5.6.10","fr":30,"ip":0,"op":969,"w":100,"h":100,
    "nm":"timeline","ddd":0,"assets":[],"layers":[],"markers":[]
  })";

  auto model = ParseComposition(kLottie);

  ASSERT_NE(nullptr, model);
  EXPECT_EQ(32300, model->GetTimelineDurationMs());
}

TEST(CompositionParserTest, SkipsAssetsWithInvalidStringFields) {
  static constexpr const char* kLottie = R"({
    "v":"5.6.10","fr":30,"ip":0,"op":60,"w":100,"h":100,
    "assets":[
      {"id":null,"w":10,"h":10,"p":"missing-id.png","u":""},
      {"id":"missing-file","w":10,"h":10,"p":null,"u":""},
      {"id":"valid-image","w":10,"h":10,"p":"image.png","u":null},
      {"id":"valid-audio","p":"audio.mp3","u":""},
      {"id":"valid-precomp","layers":[]}
    ],
    "layers":[],"markers":[]
  })";

  auto model = ParseComposition(kLottie, true);

  ASSERT_NE(nullptr, model);
  ASSERT_EQ(1u, model->GetImages().size());
  EXPECT_EQ(0u, model->GetImages().count(""));
  EXPECT_EQ(1u, model->GetImages().count("valid-image"));
  EXPECT_TRUE(model->GetImages().at("valid-image")->Model().dir_name.empty());
  EXPECT_EQ(1u, model->GetAudios().size());
  EXPECT_EQ(1u, model->GetAudios().count("valid-audio"));
  EXPECT_EQ(1u, model->GetPrecomps().size());
  EXPECT_EQ(1u, model->GetPrecomps().count("valid-precomp"));
}

TEST(CompositionModelTest, PreservesDirectPlaybackInitialization) {
  CompositionModel model(1.0f);
  model.Init(std::make_unique<RectF>(0, 0, 100, 100), 10.0f, 70.0f, 30.0f,
             false);

  EXPECT_FLOAT_EQ(70.0f, model.GetEndFrame());
  EXPECT_FLOAT_EQ(60.0f, model.GetDurationFrames());
  EXPECT_EQ(2000, model.GetDuration());
  EXPECT_FLOAT_EQ(70.01f, model.GetTimelineEndFrame());
  EXPECT_NEAR(60.01, model.GetTimelineDurationFrames(), 0.00001);
  EXPECT_EQ(2000, model.GetTimelineDurationMs());
}

TEST(CompositionFrameUtilTest, ConvertsBetweenTimelineAndPlaybackValues) {
  const auto playback_end = CompositionFrameUtil::ToPlaybackEndFrame(70.0f);

  EXPECT_FLOAT_EQ(69.99f, playback_end);
  EXPECT_FLOAT_EQ(70.0f,
                  CompositionFrameUtil::ToTimelineEndFrame(playback_end));
  EXPECT_NEAR(
      60.0, CompositionFrameUtil::ToTimelineDurationFrames(10.0f, playback_end),
      0.00001);
  EXPECT_EQ(2000, CompositionFrameUtil::ToTimelineDurationMs(
                      10.0f, playback_end, 30.0f));

  const auto fractional_end = CompositionFrameUtil::ToPlaybackEndFrame(70.25f);
  EXPECT_NEAR(
      80.75,
      CompositionFrameUtil::ToTimelineDurationFrames(-10.5f, fractional_end),
      0.00001);
  EXPECT_EQ(3230, CompositionFrameUtil::ToTimelineDurationMs(
                      -10.5f, fractional_end, 25.0f));
}

TEST(CompositionFrameUtilTest, ConvertsEmptyAndDegenerateTimelines) {
  const auto playback_end = CompositionFrameUtil::ToPlaybackEndFrame(10.0f);

  EXPECT_FLOAT_EQ(9.99f, playback_end);
  EXPECT_FLOAT_EQ(10.0f,
                  CompositionFrameUtil::ToTimelineEndFrame(playback_end));
  EXPECT_DOUBLE_EQ(
      0.0, CompositionFrameUtil::ToTimelineDurationFrames(10.0f, playback_end));
  EXPECT_EQ(0, CompositionFrameUtil::ToTimelineDurationMs(10.0f, playback_end,
                                                          25.0f));
  EXPECT_EQ(0, CompositionFrameUtil::ToTimelineDurationMs(10.0f, 20.0f, 0.0f));

  EXPECT_FLOAT_EQ(10.01f, CompositionFrameUtil::ToTimelineEndFrame(10.0f));
  EXPECT_NEAR(0.01,
              CompositionFrameUtil::ToTimelineDurationFrames(10.0f, 10.0f),
              0.000001);
}

}  // namespace
