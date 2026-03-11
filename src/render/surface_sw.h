// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SURFACE_SW_H_
#define ANIMAX_SRC_RENDER_SURFACE_SW_H_

#include "src/render/canvas.h"
#include "src/render/surface.h"

namespace skity {
class Bitmap;
}
namespace lynx {
namespace animax {

class Drawable;

class SurfaceSW : public Surface {
 public:
  SurfaceSW(std::unique_ptr<skity::Bitmap> bitmap);

  ~SurfaceSW() override;

  Canvas* GetCanvas() override { return wrap_.get(); }

  void Clear() override;

  void Flush() override {}

  void Destroy() override {}

  static std::unique_ptr<SurfaceSW> Create(const Drawable& drawable);

 private:
  std::unique_ptr<skity::Bitmap> bitmap_;
  std::unique_ptr<skity::Canvas> canvas_ = {};
  std::unique_ptr<Canvas> wrap_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SURFACE_SW_H_
