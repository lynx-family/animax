// Copyright 2024 The Lynx Authors
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/animax_surface_android.h"

#include <cstring>

#include "base/include/platform/android/jni_utils.h"
#include "src/base/gl/android/egl_util.h"
#include "src/base/log/log.h"
#include "src/base/monitor/trace_event.h"
#include "src/base/thread/thread_assert.h"
#include "src/base/util/buffer_copy_helper.h"
#include "src/player/android/android_native_window.h"
#include "src/player/android/animax_egl_surface.h"
#include "src/player/android/bitmap_buffer_group.h"
#include "src/player/android/first_frame_aware_surface_texture.h"
#include "src/player/android/gl_context_android.h"
#include "src/render/drawable_gl.h"
#include "src/render/drawable_sw.h"
#include "src/render/surface_gl.h"
#include "src/render/surface_sw.h"

namespace lynx {
namespace animax {

namespace {

class AnimaXSurfaceAndroidGL : public AnimaXSurfaceAndroid {
 public:
  AnimaXSurfaceAndroidGL(
      std::unique_ptr<animax::SurfaceTextureAndroid> surface_texture,
      AndroidNativeWindow window, const SurfaceDrawableDescription& desc)
      : AnimaXSurfaceAndroid(desc.width, desc.height),
        surface_texture_{std::move(surface_texture)},
        window_{std::move(window)} {
    CreateDrawSurface(desc);
    DCHECK(AnimaXEGLContext::Instance().IsCurrent());
  }

  ~AnimaXSurfaceAndroidGL() override {
    ThreadAssert::Assert(ThreadAssert::Type::kGPU);
    DCHECK(AnimaXEGLContext::Instance().IsCurrent());
  }

  void Flush() override {
    if (egl_surface_.MakeCurrent()) {
      ANIMAX_TRACE_EVENT_BEGIN(kFlushFrame);
      draw_surface_->Flush();
      ANIMAX_TRACE_EVENT_END();
      ANIMAX_TRACE_EVENT_BEGIN(kSwapFrame);
      egl_surface_.Flush();
      ANIMAX_TRACE_EVENT_END();
    }
  }

  animax::Canvas* Canvas() override { return draw_surface_->GetCanvas(); }

  AnimaXBackend Type() const override { return AnimaXBackend::kGL; }

  bool Valid() const override {
    return window_ && egl_surface_ && draw_surface_;
  }

  void OnReconfigure(const SurfaceDrawableDescription& desc) override {
    if (desc.width <= 0 || desc.height <= 0) {
      return;
    }
    AnimaXSurface::Resize(desc.width, desc.height);
    egl_surface_.Destroy();
    DCHECK(!egl_surface_.Valid());
    window_.Resize(Width(), Height());
    draw_surface_->Destroy();
    CreateDrawSurface({Width(), Height(), desc.enable_anti_aliasing});
  }

 private:
  void CreateDrawSurface(const SurfaceDrawableDescription& desc) {
    egl_surface_ = AnimaXEGLSurface{window_, GetEGLConfigRGBA8()};

    auto drawable = DrawableGL(egl_surface_.Framebuffer(), desc.width,
                               desc.height, desc.enable_anti_aliasing);
    draw_surface_ = std::make_unique<SurfaceGL>(drawable);
    DCHECK(Valid());
  }
  std::unique_ptr<animax::SurfaceTextureAndroid> surface_texture_{};
  AndroidNativeWindow window_{};
  AnimaXEGLSurface egl_surface_{};
  std::unique_ptr<Surface> draw_surface_{};
};

class AnimaXSurfaceAndroidSW : public AnimaXSurfaceAndroid {
 public:
  AnimaXSurfaceAndroidSW(std::unique_ptr<SurfaceTextureAndroid> surface_texture,
                         AndroidNativeWindow window,
                         const SurfaceDrawableDescription& desc)
      : AnimaXSurfaceAndroid(desc.width, desc.height),
        surface_texture_(std::move(surface_texture)),
        window_(std::move(window)) {
    CreateDrawSurface();
  }

  ~AnimaXSurfaceAndroidSW() override {
    if (buffer_) {
      delete[] buffer_;
    }
  }

  void Flush() override {
    ANativeWindow_Buffer a_buffer;
    ANativeWindow_lock(window_, &a_buffer, nullptr);
    if (BufferCopyHelper::IsSizeMatch(Width(), Height(), a_buffer.width,
                                      a_buffer.height)) {
      BufferCopyHelper::CopyBuffer(buffer_,
                                   reinterpret_cast<uint8_t*>(a_buffer.bits),
                                   Height(), 4 * Width(), a_buffer.stride * 4);
    }

    ANativeWindow_unlockAndPost(window_);
    // clear bitmap for next frame
    draw_surface_->Clear();
  }

  animax::Canvas* Canvas() override { return draw_surface_->GetCanvas(); }

  AnimaXBackend Type() const override { return AnimaXBackend::kSoftware; }

  bool Valid() const override { return window_ && draw_surface_; }

 protected:
  void OnReconfigure(const SurfaceDrawableDescription& desc) override {
    if (Width() == desc.width && Height() == desc.height) {
      return;
    }

    AnimaXSurface::Resize(desc.width, desc.height);

    delete[] buffer_;
    window_.Resize(Width(), Height());

    CreateDrawSurface();
  }

 private:
  void CreateDrawSurface() {
    buffer_ = new uint8_t[Width() * Height() * 4];
    DrawableSW drawable{Width(), Height(), buffer_, Width() * 4};
    draw_surface_ = SurfaceSW::Create(drawable);
  }

  std::unique_ptr<animax::SurfaceTextureAndroid> surface_texture_;
  AndroidNativeWindow window_;
  uint8_t* buffer_;
  std::unique_ptr<Surface> draw_surface_;
};

class AnimaXImageSurfaceAndroidSW : public AnimaXSurfaceAndroid {
 public:
  AnimaXImageSurfaceAndroidSW(
      std::unique_ptr<BitmapBufferGroup> bitmap_buffer_group,
      const SurfaceDrawableDescription& desc)
      : AnimaXSurfaceAndroid(desc.width, desc.height),
        bitmap_buffer_group_(std::move(bitmap_buffer_group)) {
    CreateDrawSurface();
  }

  ~AnimaXImageSurfaceAndroidSW() override {
    if (buffer_) {
      delete[] buffer_;
    }
  }

  void Flush() override {
    bitmap_buffer_group_->CopyTo(buffer_, Height(), Width());
    draw_surface_->Clear();
  }

  animax::Canvas* Canvas() override { return draw_surface_->GetCanvas(); }

  AnimaXBackend Type() const override { return AnimaXBackend::kSoftware; }

  bool Valid() const override { return bitmap_buffer_group_ && draw_surface_; }

 protected:
  void OnReconfigure(const SurfaceDrawableDescription& desc) override {
    if (Width() == desc.width && Height() == desc.height) {
      return;
    }

    AnimaXSurface::Resize(desc.width, desc.height);

    delete[] buffer_;

    CreateDrawSurface();
  }

 private:
  void CreateDrawSurface() {
    buffer_ = new uint8_t[Width() * Height() * 4];
    DrawableSW drawable{Width(), Height(), buffer_, Width() * 4};
    draw_surface_ = SurfaceSW::Create(drawable);
  }

  std::unique_ptr<BitmapBufferGroup> bitmap_buffer_group_;
  uint8_t* buffer_;
  std::unique_ptr<Surface> draw_surface_;
};
}  // namespace

AnimaXSurfaceAndroid::AnimaXSurfaceAndroid(float width, float height)
    : AnimaXSurface(width, height) {}

std::unique_ptr<AnimaXSurface> AnimaXSurfaceAndroid::Make(
    AnimaXSurfaceDrawableAndroid surface_drawable) {
  if (!surface_drawable.HasValidDrawable()) {
    return nullptr;
  }

  auto width = surface_drawable.GetWidth();
  auto height = surface_drawable.GetHeight();
  auto backend = surface_drawable.IsSoftwareRenderEnabled()
                     ? AnimaXBackend::kSoftware
                     : AnimaXBackend::kGL;
  JNIEnv* env = base::android::AttachCurrentThread();
  std::unique_ptr<AnimaXSurface> animax_surface{};
  auto desc = SurfaceDrawableDescription{
      width, height, surface_drawable.IsAntiAliasingEnabled()};

  switch (surface_drawable.GetType()) {
    case AnimaXSurfaceDrawableAndroid::Type::kBitmapBufferGroup: {
      auto bitmap_buffer_group =
          std::make_unique<lynx::animax::BitmapBufferGroup>(
              env, surface_drawable.GetBitmapBufferGroup().Get());
      if (backend == AnimaXBackend::kSoftware) {
        animax_surface.reset(new AnimaXImageSurfaceAndroidSW(
            std::move(bitmap_buffer_group), desc));
      }  // backend == AnimaXBackend::kGL
      // TODO(lixianruo) enable AnimaXImageView rendered with GL
      break;
    }
    case AnimaXSurfaceDrawableAndroid::Type::kTextureView:
    case AnimaXSurfaceDrawableAndroid::Type::kSurfaceView: {
      // native SurfaceTexture is a wrapper of Java_SurfaceTexture
      std::unique_ptr<lynx::animax::SurfaceTextureAndroid> surface_texture{};
      if (surface_drawable.GetType() ==
          AnimaXSurfaceDrawableAndroid::Type::kTextureView) {
        if (surface_drawable.IsTextureFirstFrameAware()) {
          surface_texture = std::make_unique<
              lynx::animax::FirstFrameAwareSurfaceTextureAndroid>(
              env, surface_drawable.GetSurfaceTexture().Get());
        } else {
          surface_texture =
              std::make_unique<lynx::animax::SurfaceTextureAndroid>(
                  env, surface_drawable.GetSurfaceTexture().Get());
        }
      }

      auto android_native_window = AndroidNativeWindow(
          surface_drawable.GetSurface().Get(), width, height);

      if (!android_native_window) {
        ANIMAX_LOGE("native_window is null");
        return nullptr;
      }

      if (backend == AnimaXBackend::kSoftware) {
        animax_surface.reset(
            new AnimaXSurfaceAndroidSW(std::move(surface_texture),
                                       std::move(android_native_window), desc));
      } else {  // backend == AnimaXBackend::kGL
        // Ensures the EGLContext is properly initialized and set as current.
        // This is crucial if it's the initial usage of an EGLContext within the
        // AnimaX_GPU thread.
        AnimaXEGLContext::Instance().init(
            surface_drawable.IsAutoDestroyEGLContextEnabled());
        AnimaXEGLContext::Instance().MakeCurrent();
        animax_surface.reset(
            new AnimaXSurfaceAndroidGL{std::move(surface_texture),
                                       std::move(android_native_window), desc});
      }
      break;
    }
    default:
      break;
  }

  return animax_surface;
}

std::unique_ptr<AnimaXSurface> AnimaXSurfaceAndroid::Reconfigure(
    std::unique_ptr<AnimaXSurface> old_surface,
    const SurfaceDrawableDescription& desc) {
  auto* old_surface_android =
      static_cast<AnimaXSurfaceAndroid*>(old_surface.get());
  if (old_surface_android) {
    old_surface_android->OnReconfigure(desc);
  }

  return old_surface;
}

}  // namespace animax
}  // namespace lynx
