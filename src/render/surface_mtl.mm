// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/surface_mtl.h"

#include "skity/gpu/gpu_context_mtl.h"
#include "skity/gpu/gpu_surface.hpp"
#include "src/base/log/log.h"
#include "src/base/monitor/trace_event.h"
#include "src/render/drawable_mtl.h"
#include "src/render/error_callback.h"

namespace lynx {
namespace animax {

namespace {

static constexpr uint32_t kSampleCountDefault = 4;
static constexpr float kContentNoScale = 1.f;

std::shared_ptr<skity::GPUContext> GetGPUContext(id<MTLDevice> device) {
  static thread_local std::weak_ptr<skity::GPUContext> g_gpu_context;

  std::shared_ptr<skity::GPUContext> ctx = g_gpu_context.lock();

  if (ctx) {
    return ctx;
  }

  if (device == nil) {
    device = MTLCreateSystemDefaultDevice();
  }

  ctx = skity::MTLContextCreate(device, [device newCommandQueue]);

  ctx->SetErrorCallback(&SkityErrorCallbackDefault, nullptr);

  g_gpu_context = ctx;

  return ctx;
}

const DrawableMTL& GetDrawableMTL(const Drawable& drawable) {
  DCHECK(drawable.BackendType() == ContextBackend::kMetal);
  return static_cast<const DrawableMTL&>(drawable);
}

skity::GPUSurfaceDescriptorMTL CreateGPUSurfaceDescriptorMTL(const Drawable& drawable) {
  const auto& drawable_mtl = GetDrawableMTL(drawable);
  auto desc = skity::GPUSurfaceDescriptorMTL{};
  desc.backend = skity::GPUBackendType::kMetal;
  desc.content_scale = kContentNoScale;
  desc.width = drawable_mtl.Width();
  desc.height = drawable_mtl.Height();
  // metal always use msaa
  desc.sample_count = kSampleCountDefault;

  if (drawable_mtl.GetType() == DrawableMTLBackendType::kLayer) {
    desc.surface_type = skity::MTLSurfaceType::kLayer;
    desc.layer = drawable_mtl.Layer();
  } else if (drawable_mtl.GetType() == DrawableMTLBackendType::kTexture) {
    desc.surface_type = skity::MTLSurfaceType::kTexture;
    desc.texture = drawable_mtl.Texture();
  } else {
    desc.surface_type = skity::MTLSurfaceType::kInvalid;
  }
  return desc;
}

std::shared_ptr<skity::GPUContext> GetGPUContext(const Drawable& drawable) {
  const auto& drawable_mtl = GetDrawableMTL(drawable);

  if (drawable_mtl.GetType() == DrawableMTLBackendType::kTexture) {
    return GetGPUContext(drawable_mtl.Texture().device);
  }
  return GetGPUContext(drawable_mtl.Layer().device);
}

std::unique_ptr<skity::GPUSurface> CreateGPUSurfaceMTL(
    const std::shared_ptr<skity::GPUContext>& gpu_context, const Drawable& drawable) {
  auto desc = CreateGPUSurfaceDescriptorMTL(drawable);
  return gpu_context->CreateSurface(&desc);
}

}  // namespace

SkitySurfaceMTL::SkitySurfaceMTL(const Drawable& drawable)
    : gpu_ctx_{GetGPUContext(drawable)},
      gpu_surface_{CreateGPUSurfaceMTL(gpu_ctx_, drawable)},
      canvas_{nullptr},
      frame_canvas_{nullptr} {}

Canvas* SkitySurfaceMTL::GetCanvas() {
  // gpu_surface_->LockCanvas() must be called every frame to begin next render pass
  // TODO(liuyufeng.0716): add BeginFrame/EndFrame API for Surface
  auto skity_canvas = gpu_surface_->LockCanvas();
  if (skity_canvas != frame_canvas_) {
    frame_canvas_ = skity_canvas;
    canvas_.reset(new Canvas(frame_canvas_, gpu_surface_->GetWidth(), gpu_surface_->GetHeight(),
                             gpu_ctx_.get()));
  }
  return canvas_.get();
}

SkitySurfaceMTL::~SkitySurfaceMTL() = default;

void SkitySurfaceMTL::Clear() {}

void SkitySurfaceMTL::Flush() {
  if (canvas_ == nullptr || gpu_surface_ == nullptr) {
    return;
  }

  ANIMAX_TRACE_EVENT_BEGIN(kFlushFrame);
  frame_canvas_->Flush();
  ANIMAX_TRACE_EVENT_END();

  ANIMAX_TRACE_EVENT_BEGIN(kSwapFrame);
  gpu_surface_->Flush();
  ANIMAX_TRACE_EVENT_END();
}

void SkitySurfaceMTL::Destroy() {
  gpu_surface_.reset();
  canvas_.reset();
  frame_canvas_ = nullptr;
}

}  // namespace animax
}  // namespace lynx
