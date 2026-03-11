// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SURFACE_MTL_H_
#define ANIMAX_SRC_RENDER_SURFACE_MTL_H_

#include "src/render/canvas.h"
#include "src/render/surface.h"

namespace skity {
class GPUContext;
class GPUSurface;
}  // namespace skity
namespace lynx {
namespace animax {
class Drawable;
class SkitySurfaceMTL : public Surface {
 public:
  SkitySurfaceMTL(const Drawable& drawable);

  ~SkitySurfaceMTL() override;

  Canvas* GetCanvas() override;

  void Clear() override;

  void Flush() override;

  void Destroy() override;

 private:
  std::shared_ptr<skity::GPUContext> gpu_ctx_ = {};
  std::unique_ptr<skity::GPUSurface> gpu_surface_ = {};
  std::unique_ptr<Canvas> canvas_ = {};
  skity::Canvas* frame_canvas_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SURFACE_MTL_H_
