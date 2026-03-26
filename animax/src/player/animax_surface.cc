// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/player/animax_surface.h"

#ifdef OS_WIN
#include <skity/gpu/gpu_context_gl.hpp>

#include "src/base/gl/gl_util.h"
#endif

#include "src/render/canvas.h"

namespace lynx {
namespace animax {

std::unique_ptr<animax::Canvas> AnimaXSurface::CreateAnimaXCanvas(
    skity::Canvas* canvas, int32_t width, int32_t height,
    skity::GPUContext* context) {
  return std::make_unique<animax::Canvas>(canvas, width, height, context);
}

std::unique_ptr<skity::GPUContext> MakeGPUContext() {
#ifdef OS_WIN
  auto gl_loader = GetGLProcLoader();
  return skity::GLContextCreate(gl_loader);
#else
  return nullptr;
#endif
}

}  // namespace animax
}  // namespace lynx
