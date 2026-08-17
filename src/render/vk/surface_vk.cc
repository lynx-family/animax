// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/vk/surface_vk.h"

namespace lynx {
namespace animax {

SurfaceVk::SurfaceVk(skity::GPUNativeWindowVK *native_window,
                     bool enable_anti_aliasing)
    : native_window_(native_window),
      enable_anti_aliasing_(enable_anti_aliasing) {}

skity::GPUPresenter *SurfaceVk::PresentPendingSurface() {
  auto presenter = native_window_->GetPresenter();

  if (frame_surface_ == nullptr) {
    return presenter;
  }

  if (presenter == nullptr || frame_presenter_ != presenter) {
    // The pending surface belongs to a retired presenter (a resize recreated
    // it while a frame was in flight). Its swapchain resources are already
    // destroyed, so neither Flush() nor Present() may touch it; discarding is
    // safe because the retired swapchain has returned its buffers to the
    // window's BufferQueue.
    wrap_.reset();
    frame_surface_.reset();
    frame_presenter_ = nullptr;
    return presenter;
  }

  // The previous surface was not flushed — submit pending GPU commands
  // before presenting so stale / partial content never reaches the screen.
  if (wrap_) {
    wrap_->GetSkityCanvas()->Flush();
    wrap_.reset();
  }
  frame_surface_->Flush();
  presenter->Present(std::move(frame_surface_));
  frame_presenter_ = nullptr;
  return presenter;
}

Canvas *SurfaceVk::GetCanvas() {
  auto presenter = PresentPendingSurface();

  if (presenter == nullptr) {
    return nullptr;
  }

  skity::GPUSurfaceAcquireDescriptor desc{};
  desc.sample_count = enable_anti_aliasing_ ? 4 : 1;
  desc.content_scale = 1.f;

  auto result = presenter->AcquireNextSurface(desc);

  if (result.status != skity::GPUPresenterStatus::kSuccess) {
    // the underline swapchain is out of date, the high level code will handle
    // surface change and resize
    return nullptr;
  }

  frame_surface_ = std::move(result.surface);
  frame_presenter_ = presenter;

  auto canvas = frame_surface_->LockCanvas(true);

  wrap_.reset(new Canvas(canvas, frame_surface_->GetWidth(),
                         frame_surface_->GetHeight(),
                         native_window_->GetContext(), frame_surface_.get()));

  return wrap_.get();
}

void SurfaceVk::Flush() { PresentPendingSurface(); }

void SurfaceVk::Clear() { PresentPendingSurface(); }

void SurfaceVk::Destroy() { PresentPendingSurface(); }

}  // namespace animax
}  // namespace lynx
