// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/conformance_test/animax_surface_osmesa.h"

#include "src/base/gl/gl_include.h"
#include "src/render/drawable_gl.h"
#include "src/render/surface_gl.h"
#include "testing/conformance_test/osmesa_surface.h"

namespace lynx {
namespace animax {

AnimaXSurfaceGLOSMesa::AnimaXSurfaceGLOSMesa(int32_t width, int32_t height)
    : AnimaXSurface(width, height),
      osmesa_surface_{std::make_unique<GLSurfaceOSMesa>(Width(), Height())},
      skity_surface_{std::make_unique<SurfaceGL>(DrawableGL(
          osmesa_surface_->Framebuffer(), Width(), Height(), false))} {}

AnimaXSurfaceGLOSMesa::~AnimaXSurfaceGLOSMesa() = default;

void AnimaXSurfaceGLOSMesa::Flush() {
  MakeCurrent();
  skity_surface_->Flush();
  osmesa_surface_->Flush();
}

void AnimaXSurfaceGLOSMesa::MakeCurrent() { osmesa_surface_->MakeCurrent(); }

animax::Canvas* AnimaXSurfaceGLOSMesa::Canvas() {
  return skity_surface_->GetCanvas();
}

AnimaXBackend AnimaXSurfaceGLOSMesa::Type() const { return AnimaXBackend::kGL; }

bool AnimaXSurfaceGLOSMesa::Valid() const { return skity_surface_ != nullptr; }

const void* AnimaXSurfaceGLOSMesa::Buffer() const {
  return osmesa_surface_->Buffer();
}

void* AnimaXSurfaceGLOSMesa::Buffer() { return osmesa_surface_->Buffer(); }

}  // namespace animax
}  // namespace lynx
