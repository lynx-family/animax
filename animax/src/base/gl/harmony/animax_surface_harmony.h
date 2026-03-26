// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_HARMONY_ANIMAX_SURFACE_HARMONY_H_
#define ANIMAX_SRC_BASE_GL_HARMONY_ANIMAX_SURFACE_HARMONY_H_

#include <native_image/native_image.h>
#include <native_window/external_window.h>

#include <memory>

#include "include/player/animax_surface.h"

namespace lynx {
namespace animax {
class AnimaXSurfaceHarmony : public AnimaXSurface {
 public:
  struct Description {
    OHNativeWindow* native_window;
    // Dimensions of the surface in pixels.
    // If either of the dimension <= 0 will cause AnimaXSurfaceHarmony::Make to
    // return nullptr.
    int32_t width;
    int32_t height;
    AnimaXBackend backend;
    bool enable_anti_aliasing;
  };

  static std::unique_ptr<AnimaXSurface> Make(const Description& desc);

  struct ReconfigureDescription {
    // Desired dimensions to resize the Surface to, in pixels.
    int32_t width;
    int32_t height;
    bool enable_anti_aliasing;
  };

  [[nodiscard]] static std::unique_ptr<AnimaXSurface> Reconfigure(
      std::unique_ptr<AnimaXSurface> animax_surface,
      const ReconfigureDescription& desc);

  ~AnimaXSurfaceHarmony() override = default;

 protected:
  AnimaXSurfaceHarmony(float width, float height);

  virtual void OnReconfigure(const ReconfigureDescription& desc) = 0;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_HARMONY_ANIMAX_SURFACE_HARMONY_H_
