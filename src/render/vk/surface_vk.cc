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

Canvas *SurfaceVk::GetCanvas() {
  auto presenter = native_window_->GetPresenter();

  if (presenter == nullptr) {
    return nullptr;
  }

  if (frame_surface_) {
    // The previous surface was not flushed — submit pending GPU commands
    // before presenting so stale / partial content never reaches the screen.
    if (wrap_) {
      wrap_->GetSkityCanvas()->Flush();
      wrap_.reset();
    }
    frame_surface_->Flush();
    presenter->Present(std::move(frame_surface_));
  }

  skity::GPUSurfaceAcquireDescriptor desc{};
  desc.sample_count = enable_anti_aliasing_ ? 4 : 1;
  desc.content_scale = 1.f;

  auto result = presenter->AcquireNextSurface(desc);

  if (result.status != skity::GPUPresenterStatus::kSuccess) {
    // the underline swapchain is out of date, the high level code will handle
    // surface change and resizea
    return nullptr;
  }

  frame_surface_ = std::move(result.surface);

  auto canvas = frame_surface_->LockCanvas(true);

  wrap_.reset(new Canvas(canvas, frame_surface_->GetWidth(),
                         frame_surface_->GetHeight(),
                         native_window_->GetContext()));

  return wrap_.get();
}

void SurfaceVk::Flush() {
  if (frame_surface_ == nullptr) {
    return;
  }

  wrap_->GetSkityCanvas()->Flush();

  wrap_.reset();

  frame_surface_->Flush();

  auto presenter = native_window_->GetPresenter();

  if (presenter == nullptr) {
    return;
  }

  presenter->Present(std::move(frame_surface_));
}

void SurfaceVk::Clear() {
  if (frame_surface_ == nullptr) {
    return;
  }

  if (wrap_) {
    wrap_->GetSkityCanvas()->Flush();
    wrap_.reset();
  }
  frame_surface_->Flush();

  auto presenter = native_window_->GetPresenter();
  if (presenter) {
    presenter->Present(std::move(frame_surface_));
  }
}

void SurfaceVk::Destroy() {
  if (frame_surface_) {
    if (wrap_) {
      wrap_->GetSkityCanvas()->Flush();
      wrap_.reset();
    }
    frame_surface_->Flush();

    auto presenter = native_window_->GetPresenter();
    if (presenter) {
      presenter->Present(std::move(frame_surface_));
    }
  }
}
}  // namespace animax
}  // namespace lynx
