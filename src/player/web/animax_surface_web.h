// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WEB_ANIMAX_SURFACE_WEB_H_
#define ANIMAX_SRC_PLAYER_WEB_ANIMAX_SURFACE_WEB_H_

#include <memory>
#include <string>

#include "include/player/animax_surface.h"

namespace skity {
class GPUSurface;
class GPUContext;
}  // namespace skity

namespace lynx {
namespace animax {
class AnimaXWebGPUContext;
class AnimaXSurfaceWeb : public AnimaXSurface {
 public:
  struct Description {
    std::string canvas_id;
    int32_t width, height;  // without pixel ratio
  };

  AnimaXSurfaceWeb(const Description& desc,
                   const std::shared_ptr<AnimaXWebGPUContext>& web_gpu_ctx);
  ~AnimaXSurfaceWeb() override;

  void Flush() override;
  void Reconfigure(const Description& desc);
  lynx::animax::Canvas* Canvas() override;
  AnimaXBackend Type() const override { return backend_type_; }
  bool Valid() const override { return gpu_surface_ != nullptr; }

 private:
  void InitWebGLSurface(const Description& desc);
  void InitWebGPUSurface(const Description& desc);
  void UpdateWebGLSurface(const Description& desc);
  void UpdateWebGPUSurface(const Description& desc);

  const AnimaXBackend backend_type_;
  std::shared_ptr<AnimaXWebGPUContext> web_gpu_ctx_;
  std::unique_ptr<lynx::animax::Canvas> canvas_;
  uintptr_t gl_context_;
  std::unique_ptr<skity::GPUContext> gl_gpu_ctx_;
  void* web_gpu_surface_;
  std::unique_ptr<skity::GPUSurface> gpu_surface_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WEB_ANIMAX_SURFACE_WEB_H_
