// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/model/animatable/animatable_text_range_selector.h"

#include "gtest/gtest.h"
#include "src/model/animatable/base_animatable_value.h"

namespace lynx {
namespace animax {
namespace {

TEST(AnimatableTextRangeSelectorTest, DefaultConstructor) {
  AnimatableTextRangeSelector selector;

  EXPECT_EQ(selector.GetRangeUnits(), TextRangeUnits::kPercent);
  EXPECT_EQ(selector.GetAnimatableOffset(), nullptr);
  EXPECT_EQ(selector.GetAnimatableStart(), nullptr);
  EXPECT_EQ(selector.GetAnimatableEnd(), nullptr);
}

TEST(AnimatableTextRangeSelectorTest, ConstructorStoresInputs) {
  auto offset = std::make_unique<AnimatableFloatValue>();
  auto* offset_ptr = offset.get();
  auto start = std::make_unique<AnimatableFloatValue>();
  auto* start_ptr = start.get();
  auto end = std::make_unique<AnimatableFloatValue>();
  auto* end_ptr = end.get();

  AnimatableTextRangeSelector selector(TextRangeUnits::kIndex,
                                       std::move(offset), std::move(start),
                                       std::move(end));

  EXPECT_EQ(selector.GetRangeUnits(), TextRangeUnits::kIndex);
  EXPECT_EQ(selector.GetAnimatableOffset(), offset_ptr);
  EXPECT_EQ(selector.GetAnimatableStart(), start_ptr);
  EXPECT_EQ(selector.GetAnimatableEnd(), end_ptr);
}

}  // namespace
}  // namespace animax
}  // namespace lynx
