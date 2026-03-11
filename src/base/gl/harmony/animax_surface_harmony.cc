// Copyright 2024 The Lynx Authors
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/harmony/animax_surface_harmony.h"

#include <sys/mman.h>

#include <cstring>

#include "src/base/gl/gl_util.h"
#include "src/base/gl/harmony/animax_egl_surface.h"
#include "src/base/gl/harmony/egl_util.h"
#include "src/base/gl/harmony/gl_context_harmony.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/render/drawable_gl.h"
#include "src/render/drawable_sw.h"
#include "src/render/surface_gl.h"
#include "src/render/surface_sw.h"

namespace lynx {
namespace animax {

namespace /* AnimaXSurfaceHarmonyGL */ {

class AnimaXSurfaceHarmonyGL : public AnimaXSurfaceHarmony {
 public:
  AnimaXSurfaceHarmonyGL(OHNativeWindow* window, AnimaXEGLSurface egl_surface,
                         std::unique_ptr<Surface> draw_surface, int32_t width,
                         int32_t height)
      : AnimaXSurfaceHarmony(width, height),
        window_{window},
        egl_surface_(std::move(egl_surface)),
        draw_surface_(std::move(draw_surface)) {
    DCHECK(AnimaXEGLContext::Instance().IsCurrent());
  }

  ~AnimaXSurfaceHarmonyGL() override {
    ThreadAssert::Assert(ThreadAssert::Type::kGPU);
    DCHECK(AnimaXEGLContext::Instance().IsCurrent());
  }

  void Flush() override {
    if (egl_surface_.MakeCurrent()) {
      draw_surface_->Flush();
      egl_surface_.Flush();
    }
  }

  animax::Canvas* Canvas() override { return draw_surface_->GetCanvas(); }

  AnimaXBackend Type() const override { return AnimaXBackend::kGL; }

  bool Valid() const override {
    return window_ && egl_surface_ && draw_surface_;
  }

  void OnReconfigure(const ReconfigureDescription& desc) override {
    if (desc.width <= 0 || desc.height <= 0) {
      return;
    }
    AnimaXSurface::Resize(desc.width, desc.height);
    egl_surface_.Destroy();
    DCHECK(!egl_surface_.Valid());

    // resize
    OH_NativeWindow_NativeWindowHandleOpt(window_, SET_BUFFER_GEOMETRY,
                                          desc.width, desc.height);

    egl_surface_ = AnimaXEGLSurface{window_, GetEGLConfigRGBA8()};
    draw_surface_->Destroy();
    auto drawable = DrawableGL(egl_surface_.Framebuffer(), Width(), Height(),
                               desc.enable_anti_aliasing);
    draw_surface_ = std::make_unique<SurfaceGL>(drawable);
    DCHECK(Valid());
  }

 private:
  OHNativeWindow* window_ = nullptr;
  AnimaXEGLSurface egl_surface_{};
  std::unique_ptr<Surface> draw_surface_{};
};

class AnimaXSurfaceHarmonySW : public AnimaXSurfaceHarmony {
 public:
  AnimaXSurfaceHarmonySW(OHNativeWindow* window, int32_t width, int32_t height,
                         uint8_t* buffer, std::unique_ptr<Surface> draw_surface)
      : AnimaXSurfaceHarmony(width, height),
        window_(window),
        buffer_(buffer),
        draw_surface_(std::move(draw_surface)) {}

  ~AnimaXSurfaceHarmonySW() override {
    if (buffer_) {
      delete[] buffer_;
    }
  }

  void Flush() override {
    OHNativeWindowBuffer* a_buffer;
    int fence_id;
    OH_NativeWindow_NativeWindowRequestBuffer(window_, &a_buffer, &fence_id);
    BufferHandle* buffer_handle =
        OH_NativeWindow_GetBufferHandleFromNative(a_buffer);

    // todo(aiyongbiao.rick): not work for now
    void* map_addr =
        mmap(buffer_handle->virAddr, buffer_handle->size,
             PROT_READ | PROT_WRITE, MAP_SHARED, buffer_handle->fd, 0);
    if (map_addr == MAP_FAILED) {
      return;
    }

    uint8_t* src = buffer_;
    uint32_t* dst = static_cast<uint32_t*>(map_addr);
    uint32_t width = Width();
    uint32_t height = Height();
    for (uint32_t y = 0; y < height; y++) {
      std::memcpy(dst, src, width * 4);
      dst += width;
      src += width * 4;
    }

    Region region{nullptr, 0};
    OH_NativeWindow_NativeWindowFlushBuffer(window_, a_buffer, fence_id,
                                            region);

    int result = munmap(map_addr, buffer_handle->size);
    if (result == -1) {
      return;
    }

    // clear bitmap for next frame
    draw_surface_->Clear();
  }

  animax::Canvas* Canvas() override { return draw_surface_->GetCanvas(); }

  AnimaXBackend Type() const override { return AnimaXBackend::kSoftware; }

  bool Valid() const override { return window_ && draw_surface_; }

 protected:
  void OnReconfigure(const ReconfigureDescription& desc) override {
    if (Width() == desc.width && Height() == desc.height) {
      return;
    }

    AnimaXSurface::Resize(desc.width, desc.height);

    delete[] buffer_;

    buffer_ = new uint8_t[Width() * Height() * 4];

    DrawableSW drawable{Width(), Height(), buffer_, Width() * 4};

    draw_surface_ = SurfaceSW::Create(drawable);
  }

 private:
  OHNativeWindow* window_;
  uint8_t* buffer_;
  std::unique_ptr<Surface> draw_surface_;
};

// todo: software

}  // namespace

AnimaXSurfaceHarmony::AnimaXSurfaceHarmony(float width, float height)
    : AnimaXSurface(width, height) {}

std::unique_ptr<AnimaXSurface> AnimaXSurfaceHarmony::Make(
    const Description& desc) {
  auto [native_window, width, height, backend, enable_anti_aliasing] = desc;
  if (!native_window) {
    // You must either provide a surface or a native_window to create a
    // AnimaXSurface
    return nullptr;
  }

  if (width <= 0 || height <= 0) {
    // Invalid dimension
    return nullptr;
  }

  std::unique_ptr<AnimaXSurface> animax_surface{};
  if (backend == AnimaXBackend::kSoftware) {
    auto buffer = new uint8_t[width * height * 4];
    DrawableSW drawable{width, height, buffer, width * 4};

    auto draw_surface = SurfaceSW::Create(drawable);

    animax_surface.reset(new AnimaXSurfaceHarmonySW(
        native_window, width, height, buffer, std::move(draw_surface)));
  } else {
    // Ensures the EGLContext is properly initialized and set as current.
    // This is crucial if it's the initial usage of an EGLContext within the
    // AnimaX_GPU thread.
    AnimaXEGLContext::Instance().MakeCurrent();
    OH_NativeWindow_NativeWindowHandleOpt(native_window, SET_BUFFER_GEOMETRY,
                                          width, height);
    auto egl_surface = AnimaXEGLSurface{native_window, GetEGLConfigRGBA8()};
    auto drawable = DrawableGL(egl_surface.Framebuffer(), width, height,
                               enable_anti_aliasing);
    auto draw_surface = std::make_unique<SurfaceGL>(drawable);

    animax_surface.reset(
        new AnimaXSurfaceHarmonyGL{native_window, std::move(egl_surface),
                                   std::move(draw_surface), width, height});
  }

  return animax_surface;
}

std::unique_ptr<AnimaXSurface> AnimaXSurfaceHarmony::Reconfigure(
    std::unique_ptr<AnimaXSurface> old_surface,
    const ReconfigureDescription& desc) {
  auto* old_surface_harmony =
      static_cast<AnimaXSurfaceHarmony*>(old_surface.get());

  old_surface_harmony->OnReconfigure(desc);

  return old_surface;
}

}  // namespace animax
}  // namespace lynx
