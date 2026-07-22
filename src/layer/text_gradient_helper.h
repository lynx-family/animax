// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXT_GRADIENT_HELPER_H_
#define ANIMAX_SRC_LAYER_TEXT_GRADIENT_HELPER_H_

#include <memory>

#include "include/base/macros.h"
#include "src/model/text/text_gradient_model.h"
#include "src/render/shader.h"

namespace lynx {
namespace animax {

ANIMAX_EXPORT std::unique_ptr<Shader> MakeTextGradientShader(
    const TextGradientItemModel& item, float bounds_width, float bounds_height,
    float bounds_origin_x = 0.f, float bounds_origin_y = 0.f);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_GRADIENT_HELPER_H_
