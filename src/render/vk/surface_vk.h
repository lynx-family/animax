// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_VK_SURFACE_VK_H_
#define ANIMAX_SRC_RENDER_VK_SURFACE_VK_H_

#include "skity/gpu/gpu_context_vk.hpp"
#include "src/render/canvas.h"
#include "src/render/surface.h"

namespace lynx {
namespace animax {

class SurfaceVk : public Surface {
 public:
  SurfaceVk(skity::GPUNativeWindowVK* native_window, bool enable_anti_aliasing);

  ~SurfaceVk() override = default;

  Canvas* GetCanvas() override;

  void Flush() override;

  void Clear() override;

  void Destroy() override;

 private:
  // Submits and presents the pending frame surface, or discards it when it no
  // longer belongs to the active presenter (a resize retired that presenter
  // while the frame was in flight). Returns the active presenter, or nullptr
  // when there is none.
  skity::GPUPresenter* PresentPendingSurface();

  skity::GPUNativeWindowVK* native_window_;

  bool enable_anti_aliasing_;

  std::unique_ptr<skity::GPUSurface> frame_surface_ = {};

  // Presenter that frame_surface_ was acquired from; a surface may only be
  // flushed and presented through its owning presenter.
  skity::GPUPresenter* frame_presenter_ = nullptr;

  std::unique_ptr<Canvas> wrap_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_VK_SURFACE_VK_H_
