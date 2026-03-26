// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_SURFACE_OSMESA_H_
#define ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_SURFACE_OSMESA_H_

#include "include/player/animax_surface.h"
#include "src/base/gl/gl_util.h"

namespace lynx {
namespace animax {

class GLSurfaceOSMesa;
class Surface;
class AnimaXSurfaceGLOSMesa : public AnimaXSurface {
 public:
  ~AnimaXSurfaceGLOSMesa() override;
  AnimaXSurfaceGLOSMesa(int32_t width, int32_t height);

  void Flush() override;

  void MakeCurrent();

  animax::Canvas* Canvas() override;

  AnimaXBackend Type() const override;

  bool Valid() const override;

  const void* Buffer() const;

  void* Buffer();

 private:
  std::unique_ptr<GLSurfaceOSMesa> osmesa_surface_;
  std::unique_ptr<Surface> skity_surface_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_SURFACE_OSMESA_H_
