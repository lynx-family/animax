// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WEB_ANIMAX_SURFACE_WEB_H_
#define ANIMAX_SRC_PLAYER_WEB_ANIMAX_SURFACE_WEB_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

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
  using FrameCaptureCallback =
      std::function<void(std::vector<uint8_t>, int32_t, int32_t)>;

  struct Description {
    std::string canvas_id;
    int32_t width, height;  // without pixel ratio
    uintptr_t gl_context_handle = 0;
    FrameCaptureCallback frame_capture_callback;
  };

  AnimaXSurfaceWeb(const Description& desc,
                   const std::shared_ptr<AnimaXWebGPUContext>& web_gpu_ctx);
  ~AnimaXSurfaceWeb() override;

  void Flush() override;
  void Reconfigure(const Description& desc);
  void SetFrameCaptureCallback(FrameCaptureCallback callback);
  lynx::animax::Canvas* Canvas() override;
  AnimaXBackend Type() const override { return backend_type_; }
  bool Valid() const override { return gpu_surface_ != nullptr; }

 private:
  bool MakeWebGLContextCurrent();
  void InitWebGLSurface(const Description& desc);
  void InitWebGPUSurface(const Description& desc);
  void UpdateWebGLSurface(const Description& desc);
  void UpdateWebGPUSurface(const Description& desc);
  void CaptureWebGLFrameIfNeeded();

  const AnimaXBackend backend_type_;
  std::shared_ptr<AnimaXWebGPUContext> web_gpu_ctx_;
  std::unique_ptr<lynx::animax::Canvas> canvas_;
  uintptr_t gl_context_;
  std::unique_ptr<skity::GPUContext> gl_gpu_ctx_;
  void* web_gpu_surface_;
  std::unique_ptr<skity::GPUSurface> gpu_surface_;
  FrameCaptureCallback frame_capture_callback_;
  bool frame_capture_completed_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WEB_ANIMAX_SURFACE_WEB_H_
