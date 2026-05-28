// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXT_LAYER_ANIMATIONS_H_
#define ANIMAX_SRC_LAYER_TEXT_LAYER_ANIMATIONS_H_

#include <vector>

#include "src/animation/base_keyframe_animation.h"
#include "src/animation/text_keyframe_animation.h"

namespace lynx {
namespace animax {

struct TextLayerAnimations {
  std::unique_ptr<TextKeyframeAnimation> text_keyframe;

  struct AnimatorProperty {
    std::unique_ptr<ColorKeyframeAnimation> color;
    std::unique_ptr<ColorKeyframeAnimation> stroke_color;
    std::unique_ptr<FloatKeyframeAnimation> stroke_width;
    std::unique_ptr<FloatKeyframeAnimation> tracking;
    std::unique_ptr<FloatKeyframeAnimation> skew;
    std::unique_ptr<FloatKeyframeAnimation> text_size_callback;
  };
  std::vector<AnimatorProperty> animator_property_list;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_LAYER_ANIMATIONS_H_
