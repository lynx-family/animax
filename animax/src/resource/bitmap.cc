// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/resource/bitmap.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {
Bitmap::Bitmap(uint32_t width, uint32_t height, BitmapFormat format,
               BitmapAlphaType alpha, std::optional<BitmapPixels> pixels,
               std::optional<uint64_t> texture)
    : width_{width},
      height_{height},
      format_{format},
      alpha_type_{alpha},
      pixels_{std::move(pixels)},
      texture_{std::move(texture)} {}

Bitmap::~Bitmap() {
  if (pixels_.has_value() && pixels_->deleter) {
    pixels_->deleter(pixels_->context);
  }
}

std::unique_ptr<Bitmap> Bitmap::MakeRGBA(uint32_t width, uint32_t height,
                                         const void* pixels,
                                         BitmapDeleter deleter,
                                         BitmapDeleterContext context) {
  return std::unique_ptr<Bitmap>(new Bitmap(
      width, height, BitmapFormat::kRGBA, BitmapAlphaType::kPremul_AlphaType,
      BitmapPixels{pixels, deleter, context}, std::nullopt));
}

std::unique_ptr<Bitmap> Bitmap::Make(uint32_t width, uint32_t height,
                                     const void* pixels, BitmapDeleter deleter,
                                     BitmapDeleterContext context,
                                     BitmapFormat format,
                                     BitmapAlphaType alpha) {
  return std::unique_ptr<Bitmap>(
      new Bitmap(width, height, format, alpha,
                 BitmapPixels{pixels, deleter, context}, std::nullopt));
}

std::unique_ptr<Bitmap> Bitmap::MakeFromTexture(uint32_t width, uint32_t height,
                                                uint64_t texture,
                                                BitmapFormat format,
                                                BitmapAlphaType alpha) {
  return std::unique_ptr<Bitmap>(
      new Bitmap(width, height, format, alpha, std::nullopt, texture));
}

uint32_t Bitmap::Width() const { return width_; }

uint32_t Bitmap::Height() const { return height_; }

const void* Bitmap::Pixels() const {
  return pixels_.has_value() ? pixels_->data : nullptr;
}

uint32_t Bitmap::PixelsLength() const {
  return Width() * Height() * BytesPerPixel();
}

uint32_t Bitmap::BytesPerPixel() const {
  switch (format_) {
    case BitmapFormat::kRGB565:
      return 2;
    case BitmapFormat::kRGBA:
    default:
      return 4;
  }
}

uint32_t Bitmap::BytesPerRow() const { return Width() * BytesPerPixel(); }

BitmapFormat Bitmap::Format() const { return format_; }

BitmapAlphaType Bitmap::AlphaType() const { return alpha_type_; }

bool Bitmap::HasTexture() const { return texture_.has_value(); }

uint64_t Bitmap::TextureID() const {
  DCHECK(texture_.has_value());
  return *texture_;
}

}  // namespace animax
}  // namespace lynx
