// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/player/animax_surface.h"

#include "src/render/canvas.h"

namespace lynx {
namespace animax {

std::unique_ptr<animax::Canvas> AnimaXSurface::CreateAnimaXCanvas(
    skity::Canvas* canvas, int32_t width, int32_t height,
    skity::GPUContext* context) {
  return std::make_unique<animax::Canvas>(canvas, width, height, context);
}

}  // namespace animax
}  // namespace lynx
