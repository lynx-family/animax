// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_gl.h"

#include "include/resource/bitmap.h"
#include "skity/gpu/gpu_context_gl.hpp"
#include "skity/graphic/image.hpp"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/render/canvas.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

#if !defined(ANIMAX_GL_USE_OSMESA) && (defined(OS_WIN) || defined(OS_OSX))
struct SkityPromiseContext {
  std::shared_ptr<skity::Pixmap> pixmap;
  std::shared_ptr<TextureInfoGL> texture_info;

  explicit SkityPromiseContext(std::shared_ptr<skity::Pixmap> p)
      : pixmap(std::move(p)) {}

  explicit SkityPromiseContext(std::shared_ptr<TextureInfoGL> t)
      : texture_info(std::move(t)) {}

  ~SkityPromiseContext() = default;
};

std::shared_ptr<skity::Texture> GeneratePromiseTexture(void *data,
                                                       skity::GPUContext *ctx) {
  if (!ctx || !data) {
    return {};
  }

  auto promise_context = reinterpret_cast<SkityPromiseContext *>(data);

  if (promise_context->pixmap) {
    skity::TextureDescriptor desc{};
    desc.format = skity::Texture::FormatFromColorType(
        promise_context->pixmap->GetColorType());
    desc.width = promise_context->pixmap->Width();
    desc.height = promise_context->pixmap->Height();
    desc.alpha_type = promise_context->pixmap->GetAlphaType();
    desc.mipmapped = true;

    auto texture = ctx->CreateTextureWithDesc(&desc);

    texture->UploadImage(promise_context->pixmap);

    return texture;
  } else if (promise_context->texture_info) {
    skity::GPUBackendTextureInfoGL sk_texture_info;
    sk_texture_info.backend = skity::GPUBackendType::kOpenGL;
    sk_texture_info.format = skity::TextureFormat::kRGBA;
    sk_texture_info.width = promise_context->texture_info->Width();
    sk_texture_info.height = promise_context->texture_info->Height();
    sk_texture_info.alpha_type = skity::AlphaType::kPremul_AlphaType;

    sk_texture_info.tex_id = promise_context->texture_info->ID();
    sk_texture_info.owned_by_engine = false;

    return ctx->WrapTexture(&sk_texture_info);
  }

  return {};
}

void ReleasePromiseContext(void *data) {
  if (!data) {
    return;
  }

  auto promise_context = reinterpret_cast<SkityPromiseContext *>(data);

  delete promise_context;
}
#endif

ImageGL::ImageGL(std::unique_ptr<Bitmap> bitmap, RealContext *real_context) {
  auto bitmap_ptr = bitmap.release();
  auto skity_raw_data = skity::Data::MakeWithProc(
      bitmap_ptr->Pixels(), bitmap_ptr->PixelsLength(),
      [](const void *ptr, void *context) {
        delete reinterpret_cast<Bitmap *>(context);
      },
      bitmap_ptr /*context*/);
  auto pixmap = std::make_shared<skity::Pixmap>(
      skity_raw_data, bitmap_ptr->BytesPerRow(), bitmap_ptr->Width(),
      bitmap_ptr->Height(),
      bitmap_ptr->AlphaType() == BitmapAlphaType::kPremul_AlphaType
          ? skity::AlphaType::kPremul_AlphaType
          : skity::AlphaType::kUnpremul_AlphaType);

  skity::GPUContext *gpu_ctx = nullptr;
  if (real_context) {
    gpu_ctx = real_context->Get();
  }
#if !defined(ANIMAX_GL_USE_OSMESA) && (defined(OS_WIN) || defined(OS_OSX))
  // Mipmap is helpful when image is scaled down to a smaller size.
  // But cost 30% more memory usage, so only use in special platform

  auto promise = new SkityPromiseContext(pixmap);

  image_ = skity::Image::MakePromiseTextureImage2(
      skity::Texture::FormatFromColorType(pixmap->GetColorType()),
      pixmap->Width(), pixmap->Height(), pixmap->GetAlphaType(),
      &GeneratePromiseTexture, &ReleasePromiseContext, promise);
#else
  image_ = skity::Image::MakeImage(pixmap, gpu_ctx);
#endif
}

ImageGL::ImageGL(TextureInfo *texture, RealContext *real_context) {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  auto texture_gl = static_cast<TextureInfoGL *>(texture);

#if !defined(ANIMAX_GL_USE_OSMESA) && (defined(OS_WIN) || defined(OS_OSX))
  // On Windows & macOS, context is null, so texture needs to be generated
  // based on promise.
  auto texture_gl_shared = std::make_shared<TextureInfoGL>(
      texture_gl->ID(), texture_gl->Width(), texture_gl->Height(), 0);
  auto promise = new SkityPromiseContext(texture_gl_shared);

  image_ = skity::Image::MakePromiseTextureImage2(
      skity::TextureFormat::kRGBA, texture_gl->Width(), texture_gl->Height(),
      skity::AlphaType::kPremul_AlphaType, &GeneratePromiseTexture,
      &ReleasePromiseContext, promise);
#else
  DCHECK(real_context);
  auto context = real_context->Get();
  skity::GPUBackendTextureInfoGL texture_info;
  texture_info.backend = skity::GPUBackendType::kOpenGL;
  texture_info.format = skity::TextureFormat::kRGBA;
  texture_info.width = texture_gl->Width();
  texture_info.height = texture_gl->Height();
  texture_info.alpha_type = skity::AlphaType::kPremul_AlphaType;

  texture_info.tex_id = texture_gl->ID();
  texture_info.owned_by_engine = false;

  auto skity_texture = context->WrapTexture(&texture_info);

  image_ = skity::Image::MakeHWImage(skity_texture);
#endif
}

}  // namespace animax
}  // namespace lynx
