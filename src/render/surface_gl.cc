// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/surface_gl.h"

#include <cstring>

#include "skity/gpu/gpu_context_gl.hpp"
#include "skity/gpu/gpu_surface.hpp"
#include "skity/render/canvas.hpp"
#include "src/base/gl/gl_util.h"
#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/render/drawable_gl.h"
#include "src/render/error_callback.h"

namespace lynx {
namespace animax {

namespace {

static constexpr uint32_t kSampleCountAntiAliasing = 4;
static constexpr uint32_t kSampleCountNoAntiAliasing = 1;
static constexpr float kContentNoScale = 1.f;

const DrawableGL& GetDrawableGL(const Drawable& drawable) {
  DCHECK(drawable.BackendType() == ContextBackend::kOpenGL);
  return static_cast<const DrawableGL&>(drawable);
}

skity::GPUSurfaceDescriptorGL CreateGPUSurfaceDescriptorGL(
    const Drawable& drawable) {
  const auto& drawable_gl = GetDrawableGL(drawable);
  skity::GPUSurfaceDescriptorGL surface_desc{};
  // common info
  surface_desc.backend = skity::GPUBackendType::kOpenGL;
  surface_desc.sample_count = drawable_gl.EnableAntiAliasing()
                                  ? kSampleCountAntiAliasing
                                  : kSampleCountNoAntiAliasing;
  // do not need context scale
  surface_desc.content_scale = kContentNoScale;
  surface_desc.width = drawable_gl.Width();
  surface_desc.height = drawable_gl.Height();
  // gl info
  surface_desc.gl_id = drawable_gl.Framebuffer();
  surface_desc.surface_type = skity::GLSurfaceType::kFramebuffer;
  surface_desc.has_stencil_attachment = false;
  return surface_desc;
}

std::unique_ptr<skity::GPUSurface> CreateGPUSurfaceGL(
    const std::shared_ptr<skity::GPUContext>& gpu_context,
    const Drawable& drawable) {
  const auto& drawable_gl = GetDrawableGL(drawable);
  auto desc = CreateGPUSurfaceDescriptorGL(drawable_gl);
  return gpu_context->CreateSurface(&desc);
}

}  // namespace

SurfaceGL::SurfaceGL(const Drawable& drawable)
    : gpu_ctx_{GetGPUContext()},
      gpu_surface_{CreateGPUSurfaceGL(gpu_ctx_, drawable)},
      frame_canvas_{nullptr},
      wrap_{nullptr} {}

SurfaceGL::~SurfaceGL() = default;

Canvas* SurfaceGL::GetCanvas() {
  frame_canvas_ = gpu_surface_->LockCanvas();

  wrap_.reset(new Canvas(frame_canvas_, gpu_surface_->GetWidth(),
                         gpu_surface_->GetHeight(), gpu_ctx_.get(),
                         gpu_surface_.get()));

  return wrap_.get();
}

void SurfaceGL::Clear() {}

void SurfaceGL::Flush() {
  if (frame_canvas_ == nullptr || gpu_surface_ == nullptr) {
    return;
  }

  frame_canvas_->Flush();

  gpu_surface_->Flush();
}

void SurfaceGL::Destroy() {
  if (gpu_surface_ == nullptr) {
    return;
  }

  gpu_surface_.reset();
  wrap_.reset();
  frame_canvas_ = nullptr;
}

std::shared_ptr<skity::GPUContext> GetGPUContext() {
  static thread_local std::shared_ptr<skity::GPUContext> ctx =
      CreateGPUContext();
  return ctx;
}

std::shared_ptr<skity::GPUContext> CreateGPUContext() {
  auto ctx = skity::GLContextCreate(GetGLProcLoader());
  ctx->SetErrorCallback(&SkityErrorCallbackDefault, nullptr);
  ctx->SetEnableTextLinearFilter(true);
  return ctx;
}

}  // namespace animax
}  // namespace lynx
