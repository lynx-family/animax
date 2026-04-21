// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_OBJECT_FIT_UTIL_H_
#define ANIMAX_SRC_BASE_UTIL_OBJECT_FIT_UTIL_H_

#include "include/player/animax_fit_position.h"
#include "src/model/rect_model.h"

namespace lynx {
namespace animax {

struct ObjectFitLayoutResult {
  float scale_x = 1.f;
  float scale_y = 1.f;
  RectF rect;
};

ObjectFitLayoutResult CalculateObjectFitLayout(
    float container_width, float container_height, float content_width,
    float content_height, ObjectFit object_fit, ObjectPosition object_position);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_OBJECT_FIT_UTIL_H_
