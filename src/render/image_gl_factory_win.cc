// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "skity/graphic/image.hpp"
#include "src/render/image_gl_factory.h"
#include "src/render/skity_promise_texture.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

namespace {

using PixmapPromiseTexture =
    SkityPromiseTexture<std::shared_ptr<skity::Pixmap>>;
using TextureGLPromiseTexture = SkityPromiseTexture<TextureInfoGL>;

std::shared_ptr<skity::Texture> MakeTextureFromPixmap(
    skity::GPUContext *context, std::shared_ptr<skity::Pixmap> pixmap) {
  if (!context || !pixmap) {
    return {};
  }

  skity::TextureDescriptor desc{};
  desc.format = skity::Texture::FormatFromColorType(pixmap->GetColorType());
  desc.width = pixmap->Width();
  desc.height = pixmap->Height();
  desc.alpha_type = pixmap->GetAlphaType();
  desc.mipmapped = true;

  auto texture = context->CreateTextureWithDesc(&desc);
  if (!texture) {
    return {};
  }
  texture->UploadImage(pixmap);
  return texture;
}

}  // namespace

std::shared_ptr<skity::Image> CreateGLPixmapImage(
    std::shared_ptr<skity::Pixmap> pixmap, RealContext *real_context) {
  if (!pixmap) {
    return {};
  }

  auto promise = new PixmapPromiseTexture(pixmap, MakeTextureFromPixmap);
  return skity::Image::MakePromiseTextureImage2(
      skity::Texture::FormatFromColorType(pixmap->GetColorType()),
      pixmap->Width(), pixmap->Height(), pixmap->GetAlphaType(),
      &PixmapPromiseTexture::GeneratePromiseTexture,
      &PixmapPromiseTexture::ReleasePromiseContext, promise);
}

std::shared_ptr<skity::Image> CreateGLTextureImage(TextureInfoGL *texture_info,
                                                   RealContext *real_context) {
  if (!texture_info) {
    return {};
  }

  auto texture = TextureInfoGL(texture_info->ID(), texture_info->Width(),
                               texture_info->Height(), texture_info->Target());
  auto promise = new TextureGLPromiseTexture(texture, MakeTextureFromGLTexture);
  return skity::Image::MakePromiseTextureImage2(
      skity::TextureFormat::kRGBA, texture_info->Width(),
      texture_info->Height(), skity::AlphaType::kPremul_AlphaType,
      &TextureGLPromiseTexture::GeneratePromiseTexture,
      &TextureGLPromiseTexture::ReleasePromiseContext, promise);
}

}  // namespace animax
}  // namespace lynx
