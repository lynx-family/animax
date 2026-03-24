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

  explicit SkityPromiseContext(std::shared_ptr<skity::Pixmap> p)
      : pixmap(std::move(p)) {}

  ~SkityPromiseContext() = default;
};

std::shared_ptr<skity::Texture> GeneratePromiseTexture(void *data,
                                                       skity::GPUContext *ctx) {
  if (!ctx || !data) {
    return {};
  }

  auto promise_context = reinterpret_cast<SkityPromiseContext *>(data);

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
  if (gpu_ctx) {
    skity::TextureDescriptor desc{};
    desc.format = skity::Texture::FormatFromColorType(pixmap->GetColorType());
    desc.width = pixmap->Width();
    desc.height = pixmap->Height();
    desc.alpha_type = pixmap->GetAlphaType();
    // enable mipmap creation and let the engine calculate the mipmap level
    // count
    desc.mipmapped = true;

    auto texture = gpu_ctx->CreateTextureWithDesc(&desc);
    if (texture) {
      texture->DeferredUploadImage(std::move(pixmap));

      image_ = skity::Image::MakeHWImage(std::move(texture));
    }
  } else {
    auto promise = new SkityPromiseContext(pixmap);

    image_ = skity::Image::MakePromiseTextureImage2(
        skity::Texture::FormatFromColorType(pixmap->GetColorType()),
        pixmap->Width(), pixmap->Height(), pixmap->GetAlphaType(),
        &GeneratePromiseTexture, &ReleasePromiseContext, promise);
  }
#else
  image_ = skity::Image::MakeImage(pixmap, gpu_ctx);
#endif
}

ImageGL::ImageGL(TextureInfo *texture, RealContext *real_context) {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  DCHECK(real_context);
  auto texture_gl = static_cast<TextureInfoGL *>(texture);
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
}

}  // namespace animax
}  // namespace lynx
