// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_IOS_ANIMAX_SURFACE_IOS_H_
#define ANIMAX_SRC_PLAYER_IOS_ANIMAX_SURFACE_IOS_H_

#import <AnimaX/AnimaXSurfaceDrawable.h>

#include <memory>
#include "include/player/animax_surface.h"

namespace lynx {
namespace animax {
class AnimaXSurfaceIOS : public AnimaXSurface {
 public:
  struct ReconfigureDescription {
    // Desired dimensions to resize the Surface to, in pixels.
    CGSize size;
    AnimaXRenderTarget target;
  };

  static std::unique_ptr<AnimaXSurface> Make(AnimaXSurfaceDrawable* drawable);
  static std::unique_ptr<AnimaXSurface> Reconfigure(std::unique_ptr<AnimaXSurface> animax_surface,
                                                    const ReconfigureDescription& desc);

  ~AnimaXSurfaceIOS() override = default;

 protected:
  AnimaXSurfaceIOS(const ReconfigureDescription& desc);
  virtual void OnReconfigure(const ReconfigureDescription& desc) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_IOS_ANIMAX_SURFACE_IOS_H_
