// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXT_RANGE_SELECTOR_PROPERTY_H_
#define ANIMAX_SRC_LAYER_TEXT_RANGE_SELECTOR_PROPERTY_H_

#include <cmath>
#include <cstddef>
#include <memory>

#include "src/animation/base_keyframe_animation.h"
#include "src/model/animatable/animatable_text_range_selector.h"

namespace lynx {
namespace animax {

struct RangeSelectorProperty {
  TextRangeUnits range_units = TextRangeUnits::kPercent;
  std::unique_ptr<FloatKeyframeAnimation> offset;
  std::unique_ptr<FloatKeyframeAnimation> start;
  std::unique_ptr<FloatKeyframeAnimation> end;

  size_t GetRangeStart(size_t text_length) const {
    if (text_length == 0) {
      return 0;
    }
    float ret = (start ? start->GetValue().Get() : 0.f) +
                (offset ? offset->GetValue().Get() : 0.f);
    if (range_units == TextRangeUnits::kPercent) {
      ret = text_length * ret / 100.f;
    }
    return static_cast<size_t>(std::floor(ret));
  }

  size_t GetRangeLength(size_t text_length) const {
    if (text_length == 0) {
      return 0;
    }
    const float default_end = range_units == TextRangeUnits::kPercent
                                  ? 100.f
                                  : static_cast<float>(text_length);
    float end_value = (end ? end->GetValue().Get() : default_end);
    float start_value = (start ? start->GetValue().Get() : 0.f);
    if (range_units == TextRangeUnits::kPercent) {
      end_value = text_length * end_value / 100.f;
      start_value = text_length * start_value / 100.f;
    }
    if (start_value > end_value) {
      return 0;
    }
    return static_cast<size_t>(std::ceil(end_value) - std::floor(start_value));
  }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_RANGE_SELECTOR_PROPERTY_H_
