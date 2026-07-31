// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/parser/composition_parser.h"

#include <cstring>
#include <memory>

#include "gtest/gtest.h"
#include "src/base/util/composition_frame_util.h"

using namespace lynx::animax;

namespace {

std::shared_ptr<CompositionModel> ParseComposition(const char* json,
                                                   bool enable_audio = false) {
  return CompositionParser::Parse(json, std::strlen(json), 1.0f, enable_audio);
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
