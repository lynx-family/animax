// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/resource/bitmap.h"

namespace lynx {
namespace animax {
Bitmap::Bitmap(uint32_t width, uint32_t height, const void* pixels,
               BitmapDeleter deleter, BitmapDeleterContext context,
               BitmapFormat format, BitmapAlphaType alpha)
    : width_{width},
      height_{height},
      pixels_{pixels},
      deleter_{deleter},
      context_{context},
      format_{format},
      alpha_type_{alpha} {}

Bitmap::~Bitmap() {
  if (deleter_) {
    deleter_(context_);
  }
}

std::unique_ptr<Bitmap> Bitmap::MakeRGBA(uint32_t width, uint32_t height,
                                         const void* pixels,
                                         BitmapDeleter deleter,
                                         BitmapDeleterContext context) {
  return std::unique_ptr<Bitmap>(
      new Bitmap(width, height, pixels, deleter, context));
}

std::unique_ptr<Bitmap> Bitmap::Make(uint32_t width, uint32_t height,
                                     const void* pixels, BitmapDeleter deleter,
                                     BitmapDeleterContext context,
                                     BitmapFormat format,
                                     BitmapAlphaType alpha) {
  return std::unique_ptr<Bitmap>(
      new Bitmap(width, height, pixels, deleter, context, format, alpha));
}

uint32_t Bitmap::Width() const { return width_; }

uint32_t Bitmap::Height() const { return height_; }

const void* Bitmap::Pixels() const { return pixels_; }

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

}  // namespace animax
}  // namespace lynx
