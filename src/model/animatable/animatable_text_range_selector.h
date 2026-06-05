// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TEXT_RANGE_SELECTOR_H_
#define ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TEXT_RANGE_SELECTOR_H_

#include <cstdint>

#include "src/model/animatable/base_animatable_value.h"

namespace lynx {
namespace animax {

enum class TextRangeUnits : uint8_t {
  kPercent = 1,
  kIndex,
};
class AnimatableTextRangeSelector {
 public:
  AnimatableTextRangeSelector() = default;
  AnimatableTextRangeSelector(TextRangeUnits range_units,
                              std::unique_ptr<AnimatableFloatValue> offset,
                              std::unique_ptr<AnimatableFloatValue> start,
                              std::unique_ptr<AnimatableFloatValue> end)
      : range_units_(range_units),
        animatable_offset_(std::move(offset)),
        animatable_start_(std::move(start)),
        animatable_end_(std::move(end)){};
  TextRangeUnits GetRangeUnits() const { return range_units_; }
  AnimatableFloatValue* GetAnimatableOffset() const {
    return animatable_offset_.get();
  }
  AnimatableFloatValue* GetAnimatableStart() const {
    return animatable_start_.get();
  }
  AnimatableFloatValue* GetAnimatableEnd() const {
    return animatable_end_.get();
  }

 private:
  TextRangeUnits range_units_ = TextRangeUnits::kPercent;
  std::unique_ptr<AnimatableFloatValue> animatable_offset_;
  std::unique_ptr<AnimatableFloatValue> animatable_start_;
  std::unique_ptr<AnimatableFloatValue> animatable_end_;
};

}  // namespace animax
}  // namespace lynx
#endif  // ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TEXT_RANGE_SELECTOR_H_
