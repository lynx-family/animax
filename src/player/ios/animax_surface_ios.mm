// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/ios/animax_surface_ios.h"

#include "src/base/log/log.h"
#include "src/base/util/buffer_copy_helper.h"
#include "src/base/util/ios/ca_util.h"
#include "src/render/drawable_mtl.h"
#include "src/render/surface_mtl.h"

#ifdef ENABLE_IOS_SOFTWARE_RENDER
#include "src/render/drawable_sw.h"
#include "src/render/surface_sw.h"
#endif

#import <AnimaX/AnimaXSurfaceDrawable.h>
#import "AnimaXSurfaceDrawable+Internal.h"

namespace lynx {
namespace animax {
#pragma mark - Surface: AnimaXSurfaceIOS
AnimaXSurfaceIOS::AnimaXSurfaceIOS(const ReconfigureDescription& desc)
    : AnimaXSurface(desc.size.width, desc.size.height) {}

#pragma mark - Surface: AnimaXLayerSurfaceIOSMetal

class AnimaXLayerSurfaceIOSMetal : public AnimaXSurfaceIOS {
 public:
  AnimaXLayerSurfaceIOSMetal(const ReconfigureDescription& desc, CAMetalLayer* layer)
      : AnimaXSurfaceIOS(desc), metal_layer_{layer} {
    auto drawable = DrawableMTL(DrawableMTLBackendType::kLayer, layer, nil, Width(), Height());
    skity_surface_mtl_ = std::make_unique<SkitySurfaceMTL>(drawable);
  }

  ~AnimaXLayerSurfaceIOSMetal() override {}

  animax::Canvas* Canvas() override { return skity_surface_mtl_->GetCanvas(); }

  void Flush() override { skity_surface_mtl_->Flush(); }

  AnimaXBackend Type() const override { return AnimaXBackend::kMetal; }

  bool Valid() const override { return metal_layer_ != nil && skity_surface_mtl_; }

  void OnReconfigure(const ReconfigureDescription& desc) override {
    AnimaXSurface::Resize(desc.size.width, desc.size.height);
    skity_surface_mtl_->Destroy();
    ResizeCAMetalLayerDrawable(metal_layer_, desc.size);
    auto drawable =
        DrawableMTL(DrawableMTLBackendType::kLayer, metal_layer_, nil, Width(), Height());
    skity_surface_mtl_ = std::make_unique<SkitySurfaceMTL>(drawable);
    DCHECK(Valid());
  }

  CAMetalLayer* metal_layer_ = nil;
  std::unique_ptr<Surface> skity_surface_mtl_ = nullptr;
};

#pragma mark - Surface: AnimaXPixelBufferSurfaceIOS

class AnimaXPixelBufferSurfaceIOS : public AnimaXSurfaceIOS {
 public:
  AnimaXPixelBufferSurfaceIOS(const ReconfigureDescription& desc, CVPixelBufferWrapper* buffer)
      : AnimaXSurfaceIOS(desc), pixel_buffer_wrapper_(buffer) {
    DCHECK(pixel_buffer_wrapper_);
    ResizeBufferWrapper();
  }

 protected:
  void ResizeBufferWrapper() {
    gen_ = pixel_buffer_wrapper_.generation;
    [pixel_buffer_wrapper_ resizePixelBufferWrapperWithWidth:Width() height:Height()];
  }

  NSUInteger gen_;
  CVPixelBufferWrapper* pixel_buffer_wrapper_;
};

#pragma mark - Surface: AnimaXPixelBufferSurfaceIOSMetal

class AnimaXPixelBufferSurfaceIOSMetal : public AnimaXPixelBufferSurfaceIOS {
 public:
  AnimaXPixelBufferSurfaceIOSMetal(const ReconfigureDescription& desc, CVPixelBufferWrapper* buffer)
      : AnimaXPixelBufferSurfaceIOS(desc, buffer) {
    CreateDrawSurface();
  }

  ~AnimaXPixelBufferSurfaceIOSMetal() override{};

  animax::Canvas* Canvas() override { return skity_surface_mtl_->GetCanvas(); }

  void Flush() override {
    skity_surface_mtl_->Flush();
    [pixel_buffer_wrapper_ notifyBufferUpdateWithGeneration:gen_];
  }

  AnimaXBackend Type() const override { return AnimaXBackend::kMetal; }

  bool Valid() const override { return pixel_buffer_wrapper_ && skity_surface_mtl_; }

  void OnReconfigure(const ReconfigureDescription& desc) override {
    // Step1: Resize the Surface
    AnimaXSurface::Resize(desc.size.width, desc.size.height);

    // Step2: Destroy the Skia / Skity Surface While Maintaining the GPU Context
    if (skity_surface_mtl_) {
      skity_surface_mtl_->Destroy();
    }

    // Step3: Update
    ResizeBufferWrapper();
    CreateDrawSurface();
  }

 private:
  void CreateDrawSurface() {
    texture_mtl_ = pixel_buffer_wrapper_.metalTexture;
    if (!texture_mtl_ || texture_mtl_.width != Width() || texture_mtl_.height != Height()) {
      texture_mtl_ = nil;
      skity_surface_mtl_.reset();
      return;
    }
    auto drawable =
        DrawableMTL(DrawableMTLBackendType::kTexture, nil, texture_mtl_, Width(), Height());
    skity_surface_mtl_ = std::make_unique<SkitySurfaceMTL>(drawable);
    DCHECK(Valid());
  }
  id<MTLTexture> texture_mtl_;
  std::unique_ptr<Surface> skity_surface_mtl_{nullptr};
};

#pragma mark - Surface: AnimaXImageSurfaceIOSSoftware

#ifdef ENABLE_IOS_SOFTWARE_RENDER
class AnimaXPixelBufferSurfaceIOSSoftware : public AnimaXPixelBufferSurfaceIOS {
 public:
  AnimaXPixelBufferSurfaceIOSSoftware(const ReconfigureDescription& desc,
                                      CVPixelBufferWrapper* buffer)
      : AnimaXPixelBufferSurfaceIOS(desc, buffer) {
    CreateDrawSurface();
  }

  ~AnimaXPixelBufferSurfaceIOSSoftware() override {
    if (buffer_) {
      delete[] buffer_;
    }
  }

  animax::Canvas* Canvas() override { return surface_sw_->GetCanvas(); }

  void Flush() override {
    [pixel_buffer_wrapper_ notifyBufferUpdateWithGeneration:gen_
                                                  srcPixels:buffer_
                                                      width:Width()
                                                     height:Height()
                                                     stride:4 * Width()];
    surface_sw_->Clear();
  }

  AnimaXBackend Type() const override { return AnimaXBackend::kSoftware; }

  bool Valid() const override { return pixel_buffer_wrapper_ && surface_sw_; }

  void OnReconfigure(const ReconfigureDescription& desc) override {
    AnimaXSurface::Resize(desc.size.width, desc.size.height);

    delete[] buffer_;

    CreateDrawSurface();
    DCHECK(Valid());
  }

 private:
  void CreateDrawSurface() {
    buffer_ = new uint8_t[Width() * Height() * 4];
    DrawableSW drawable{Width(), Height(), buffer_, Width() * 4};
    surface_sw_ = SurfaceSW::Create(drawable);
  }

  uint8_t* buffer_;
  std::unique_ptr<Surface> surface_sw_{nullptr};
};
#endif

#pragma mark - Surface Factory

std::unique_ptr<AnimaXSurface> AnimaXSurfaceIOS::Make(AnimaXSurfaceDrawable* drawable) {
  DCHECK(drawable);
  const ReconfigureDescription desc{.size = drawable.size, .target = drawable.target};
  if (drawable.target == Layer && drawable.backend == AnimaXMetal) {
    CALayer* layer = drawable.layer;
    if (!layer) {
      return nullptr;
    }
    return std::make_unique<AnimaXLayerSurfaceIOSMetal>(desc, layer);
  }

  if (drawable.target == Buffer || drawable.target == View) {
    CVPixelBufferWrapper* bufferWrapper = drawable.bufferWrapper;
    if (!bufferWrapper) {
      return nullptr;
    }
#ifdef ENABLE_IOS_SOFTWARE_RENDER
    if (drawable.backend == AnimaXSoftware) {
      return std::make_unique<AnimaXPixelBufferSurfaceIOSSoftware>(desc, bufferWrapper);
    }
#endif
    if (drawable.backend == AnimaXMetal) {
      return std::make_unique<AnimaXPixelBufferSurfaceIOSMetal>(desc, bufferWrapper);
    }
  }
  return nullptr;
}

std::unique_ptr<AnimaXSurface> AnimaXSurfaceIOS::Reconfigure(
    std::unique_ptr<AnimaXSurface> animax_surface, const ReconfigureDescription& desc) {
  auto* surface = static_cast<AnimaXSurfaceIOS*>(animax_surface.get());
  if (surface) {
    surface->OnReconfigure(desc);
  }
  if (!surface || !surface->Valid()) {
    ANIMAX_LOGE("Failed to reconfigure AnimaXSurfaceIOS");
    animax_surface.reset();
  }
  return animax_surface;
}

}  // namespace animax
}  // namespace lynx
