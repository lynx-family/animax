// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SURFACE_GL_H_
#define ANIMAX_SRC_RENDER_SURFACE_GL_H_

#include "src/base/gl/gl_include.h"
#include "src/render/canvas.h"
#include "src/render/surface.h"

#if OS_IOS
#import <CoreFoundation/CoreFoundation.h>
#endif

namespace skity {
class GPUContext;
class GPUSurface;
}  // namespace skity
namespace lynx {
namespace animax {
class Drawable;
class SurfaceGL : public Surface {
 public:
  SurfaceGL(const Drawable &drawable);
  ~SurfaceGL() override;

  Canvas *GetCanvas() override;
  void Clear() override;
  void Flush() override;
  void Destroy() override;

 private:
  // Strong reference to currently used GPU context
  std::shared_ptr<skity::GPUContext> gpu_ctx_;
  std::unique_ptr<skity::GPUSurface> gpu_surface_;
  skity::Canvas *frame_canvas_;
  std::unique_ptr<Canvas> wrap_;
};

// Creates and initializes a GPU context for this surface
std::shared_ptr<skity::GPUContext> CreateGPUContext();

// Get a thread local GPU context for this thread.
std::shared_ptr<skity::GPUContext> GetGPUContext();

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SURFACE_GL_H_
