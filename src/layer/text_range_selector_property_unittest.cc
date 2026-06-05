// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/text_range_selector_property.h"

#include "gtest/gtest.h"
#include "src/animation/base_keyframe_animation.h"

namespace lynx {
namespace animax {
namespace {

TEST(TextRangeSelectorPropertyTest, DefaultPercentRangeUsesFullLength) {
  RangeSelectorProperty property;

  EXPECT_EQ(property.GetRangeStart(10), 0u);
  EXPECT_EQ(property.GetRangeLength(10), 10u);
}

TEST(TextRangeSelectorPropertyTest, ZeroTextLengthReturnsZeroRange) {
  RangeSelectorProperty property;
  property.range_units = TextRangeUnits::kIndex;
  property.offset = AnimationFactory::Make<FloatKeyframeAnimation, Float>(3.f);
  property.start = AnimationFactory::Make<FloatKeyframeAnimation, Float>(4.f);
  property.end = AnimationFactory::Make<FloatKeyframeAnimation, Float>(8.f);

  EXPECT_EQ(property.GetRangeStart(0), 0u);
  EXPECT_EQ(property.GetRangeLength(0), 0u);
}

TEST(TextRangeSelectorPropertyTest, IndexRangeUsesAbsoluteValues) {
  RangeSelectorProperty property;
  property.range_units = TextRangeUnits::kIndex;
  property.offset = AnimationFactory::Make<FloatKeyframeAnimation, Float>(3.f);
  property.start = AnimationFactory::Make<FloatKeyframeAnimation, Float>(4.f);
  property.end = AnimationFactory::Make<FloatKeyframeAnimation, Float>(11.f);

  EXPECT_EQ(property.GetRangeStart(100), 7u);
  EXPECT_EQ(property.GetRangeLength(100), 7u);
}

TEST(TextRangeSelectorPropertyTest, PercentRangeScalesByTextLength) {
  RangeSelectorProperty property;
  property.range_units = TextRangeUnits::kPercent;
  property.offset = AnimationFactory::Make<FloatKeyframeAnimation, Float>(10.f);
  property.start = AnimationFactory::Make<FloatKeyframeAnimation, Float>(20.f);
  property.end = AnimationFactory::Make<FloatKeyframeAnimation, Float>(55.f);

  EXPECT_EQ(property.GetRangeStart(40), 12u);
  EXPECT_EQ(property.GetRangeLength(40), 14u);
}

}  // namespace
}  // namespace animax
}  // namespace lynx
