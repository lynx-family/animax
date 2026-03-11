// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/web/animax_surface_web.h"

#include <EGL/egl.h>
#include <emscripten/html5_webgl.h>

#include <cstring>

#include "skity/gpu/gpu_context_gl.hpp"
#include "skity/gpu/gpu_context_web.hpp"
#include "src/base/log/log.h"
#include "src/player/web/animax_web_gpu_context.h"
#include "src/render/canvas.h"

namespace lynx {
namespace animax {

// Wrapper for WGPUSurface, implements skity::GPUSurface
class AnimaxWGPUSurface : public skity::GPUSurface {
 public:
  AnimaxWGPUSurface(uint32_t width, uint32_t height, WGPUSurface surface,
                    WGPUDevice device, WGPUQueue queue,
                    skity::GPUContext* gpu_ctx)
      : width_(width),
        height_(height),
        web_surface_(surface),
        web_device_(device),
        web_queue_(queue),
        gpu_ctx_(gpu_ctx),
        current_surface_(nullptr) {}
  ~AnimaxWGPUSurface() override { wgpuSurfaceUnconfigure(web_surface_); }
  uint32_t GetWidth() const override { return width_; }
  uint32_t GetHeight() const override { return height_; }
  float ContentScale() const override { return 1.f; }
  std::shared_ptr<skity::Pixmap> ReadPixels(const skity::Rect& rect) override {
    return {};
  }
  skity::Canvas* LockCanvas(bool clear = true) override {
    if (current_surface_) {
      current_surface_->Flush();
      current_surface_ = nullptr;
    }
    WGPUSurfaceTexture texture{};
    wgpuSurfaceGetCurrentTexture(web_surface_, &texture);
    if (texture.texture == nullptr) {
      return nullptr;
    }
    skity::GPUSurfaceDescriptorWEB desc{};
    desc.backend = skity::GPUBackendType::kWebGPU;
    desc.width = wgpuTextureGetWidth(texture.texture);
    desc.height = wgpuTextureGetHeight(texture.texture);
    desc.content_scale = 1.f;
    desc.sample_count = 4;
    desc.texture = texture.texture;
    current_surface_ = gpu_ctx_->CreateSurface(&desc);
    if (!current_surface_) {
      return nullptr;
    }
    return current_surface_->LockCanvas(clear);
  }
  void Flush() override {
    if (current_surface_) {
      current_surface_->Flush();
      current_surface_ = nullptr;
    }
  }

 private:
  uint32_t width_;
  uint32_t height_;
  WGPUSurface web_surface_;
  WGPUDevice web_device_;
  WGPUQueue web_queue_;
  skity::GPUContext* gpu_ctx_;
  std::unique_ptr<skity::GPUSurface> current_surface_;
};

AnimaXSurfaceWeb::AnimaXSurfaceWeb(
    const Description& desc,
    const std::shared_ptr<AnimaXWebGPUContext>& web_gpu_ctx)
    : AnimaXSurface(desc.width, desc.height),
      backend_type_(web_gpu_ctx ? AnimaXBackend::kWebGPU : AnimaXBackend::kGL),
      web_gpu_ctx_(web_gpu_ctx),
      canvas_(nullptr),
      gl_context_(0),
      gl_gpu_ctx_(nullptr),
      web_gpu_surface_(nullptr),
      gpu_surface_(nullptr) {
  switch (backend_type_) {
    case AnimaXBackend::kGL:
      InitWebGLSurface(desc);
      break;
    case AnimaXBackend::kWebGPU:
      InitWebGPUSurface(desc);
      break;
    default:
      DCHECK(false) << "Invalid backend type!";
      break;
  }
}
AnimaXSurfaceWeb::~AnimaXSurfaceWeb() {
  canvas_.reset();
  gpu_surface_.reset();
  switch (backend_type_) {
    case AnimaXBackend::kGL:
      gl_gpu_ctx_.reset();
      if (gl_context_) {
        emscripten_webgl_make_context_current(0);
        emscripten_webgl_destroy_context(gl_context_);
        gl_context_ = 0;
      }
      break;
    case AnimaXBackend::kWebGPU:
      if (web_gpu_surface_) {
        wgpuSurfaceRelease(reinterpret_cast<WGPUSurface>(web_gpu_surface_));
        web_gpu_surface_ = nullptr;
      }
      web_gpu_ctx_.reset();
      break;
    default:
      DCHECK(false) << "Invalid backend type!";
      break;
  }
}

void AnimaXSurfaceWeb::Flush() {
  if (canvas_) {
    canvas_->GetSkityCanvas()->Flush();
    canvas_.reset();
  }
  if (gpu_surface_) {
    gpu_surface_->Flush();
  }
}
lynx::animax::Canvas* AnimaXSurfaceWeb::Canvas() {
  if (canvas_) {
    return canvas_.get();
  }
  if (!gpu_surface_) {
    return nullptr;
  }
  skity::GPUContext* gpu_ctx = nullptr;
  if (gl_gpu_ctx_) {
    gpu_ctx = gl_gpu_ctx_.get();
  } else if (web_gpu_ctx_) {
    gpu_ctx = web_gpu_ctx_->Context();
  } else {
    DCHECK(false) << "Invalid GPU context!";
    return nullptr;
  }
  canvas_ = std::make_unique<lynx::animax::Canvas>(
      gpu_surface_->LockCanvas(), gpu_surface_->GetWidth(),
      gpu_surface_->GetHeight(), gpu_ctx);
  return canvas_.get();
}

void AnimaXSurfaceWeb::Reconfigure(const Description& desc) {
  canvas_.reset();
  gpu_surface_.reset();
  Resize(desc.width, desc.height);
  switch (backend_type_) {
    case AnimaXBackend::kGL:
      if (gl_context_ > 0) {
        UpdateWebGLSurface(desc);
      } else {
        ANIMAX_LOGI("WebGL context handle is invalid! " << desc.canvas_id);
      }
      break;
    case AnimaXBackend::kWebGPU:
      if (web_gpu_surface_ && web_gpu_ctx_) {
        UpdateWebGPUSurface(desc);
      } else {
        ANIMAX_LOGI("WebGPU surface or context is invalid! " << desc.canvas_id);
      }
      break;
    default:
      DCHECK(false) << "Invalid backend type!";
      break;
  }
}
void AnimaXSurfaceWeb::InitWebGLSurface(const Description& desc) {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;
  attrs.stencil = 1;
  attrs.antialias = 0;
  attrs.enableExtensionsByDefault = 1;
  attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
  gl_context_ = emscripten_webgl_create_context(desc.canvas_id.c_str(), &attrs);
  if (gl_context_ <= 0) {
    ANIMAX_LOGI("Failed to create WebGL context!");
    return;
  }
  emscripten_webgl_make_context_current(gl_context_);
  gl_gpu_ctx_ = skity::GLContextCreate(
      reinterpret_cast<void*>(emscripten_webgl_get_proc_address));
  UpdateWebGLSurface(desc);
}
void AnimaXSurfaceWeb::UpdateWebGLSurface(const Description& desc) {
  DCHECK(gl_context_);
  gpu_surface_.reset();
  skity::GPUSurfaceDescriptorGL skity_desc{};
  skity_desc.backend = skity::GPUBackendType::kOpenGL;
  skity_desc.width = desc.width;
  skity_desc.height = desc.height;
  skity_desc.content_scale = 1;
  skity_desc.sample_count = 4;
  skity_desc.surface_type = skity::GLSurfaceType::kFramebuffer;
  skity_desc.gl_id = 0;
  skity_desc.has_stencil_attachment = false;
  gpu_surface_ = gl_gpu_ctx_->CreateSurface(&skity_desc);
}
void AnimaXSurfaceWeb::InitWebGPUSurface(const Description& desc) {
  WGPUEmscriptenSurfaceSourceCanvasHTMLSelector selector =
      WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
  selector.selector.data = desc.canvas_id.c_str();
  selector.selector.length = desc.canvas_id.size();
  WGPUSurfaceDescriptor surface_desc{};
  surface_desc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&selector);
  surface_desc.label.data = "AnimaXSurfaceWeb";
  surface_desc.label.length = std::strlen("AnimaXSurfaceWeb");
  DCHECK(web_gpu_ctx_);
  auto instance = web_gpu_ctx_->GetInstance();
  web_gpu_surface_ = wgpuInstanceCreateSurface(instance, &surface_desc);
  if (web_gpu_surface_) {
    UpdateWebGPUSurface(desc);
  } else {
    ANIMAX_LOGI("Failed to create WebGPU surface!");
  }
}
void AnimaXSurfaceWeb::UpdateWebGPUSurface(const Description& desc) {
  DCHECK(web_gpu_ctx_ && web_gpu_surface_);
  auto web_surface = reinterpret_cast<WGPUSurface>(web_gpu_surface_);
  auto device = web_gpu_ctx_->GetDevice();
  WGPUSurfaceConfiguration config{
      .device = device,
      .format = WGPUTextureFormat_BGRA8Unorm,
      .usage = WGPUTextureUsage_RenderAttachment,
      .width = static_cast<uint32_t>(desc.width),
      .height = static_cast<uint32_t>(desc.height),
      .alphaMode = WGPUCompositeAlphaMode_Premultiplied,
      .presentMode = WGPUPresentMode_Fifo};
  wgpuSurfaceConfigure(web_surface, &config);
  auto queue = web_gpu_ctx_->GetQueue();
  gpu_surface_ = std::make_unique<AnimaxWGPUSurface>(desc.width, desc.height,
                                                     web_surface, device, queue,
                                                     web_gpu_ctx_->Context());
}
}  // namespace animax
}  // namespace lynx
