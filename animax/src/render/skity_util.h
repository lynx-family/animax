// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SKITY_UTIL_H_
#define ANIMAX_SRC_RENDER_SKITY_UTIL_H_

#include "skity/geometry/rect.hpp"
#include "src/model/rect_model.h"

namespace lynx {
namespace animax {

class SkityUtil final {
 public:
  SkityUtil() = delete;
  ~SkityUtil() = delete;

  static skity::Rect MakeSkityRect(RectF const &rec);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SKITY_UTIL_H_
