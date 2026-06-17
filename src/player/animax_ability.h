// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_ABILITY_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_ABILITY_H_

#include "src/render/real_context.h"

namespace lynx {
namespace animax {

class AnimaXAbility {
 public:
  virtual ~AnimaXAbility() = default;

  /**
   * Get whether video downsampling is enabled
   * @return True if video downsampling is enabled, false otherwise.
   */
  virtual bool IsDownsampleVideoEnabled() const { return false; }

  /**
   * The GPU backend chosen for the current surface, filled by the renderer
   * after the surface is created. The video subsystem reads this to decide
   * whether to enable AHardwareBuffer bridging (GL -> Vulkan texture share)
   * together with an offscreen EGL context. Defaults to OpenGL.
   */
  ContextBackend GetBackend() const { return backend_; }
  void SetBackend(ContextBackend backend) { backend_ = backend; }

 private:
  ContextBackend backend_ = ContextBackend::kOpenGL;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_ABILITY_H_
