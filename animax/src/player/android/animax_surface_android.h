// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_SURFACE_ANDROID_H_
#define ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_SURFACE_ANDROID_H_

#include <android/native_window.h>

#include <memory>

#include "base/include/platform/android/jni_convert_helper.h"
#include "include/player/animax_surface.h"
#include "src/player/android/animax_surface_drawable_android.h"

namespace lynx {
namespace animax {
class AnimaXSurfaceAndroid : public AnimaXSurface {
 public:
  static std::unique_ptr<AnimaXSurface> Make(
      AnimaXSurfaceDrawableAndroid surface_drawable);

  struct SurfaceDrawableDescription {
    // Desired dimensions to resize the Surface to, in pixels.
    int32_t width;
    int32_t height;
    bool enable_anti_aliasing;
  };

  [[nodiscard]] static std::unique_ptr<AnimaXSurface> Reconfigure(
      std::unique_ptr<AnimaXSurface> animax_surface,
      const SurfaceDrawableDescription& desc);

  ~AnimaXSurfaceAndroid() override = default;

 protected:
  AnimaXSurfaceAndroid(float width, float height);

  virtual void OnReconfigure(const SurfaceDrawableDescription& desc) = 0;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_SURFACE_ANDROID_H_
