// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_RESOURCE_BITMAP_H_
#define ANIMAX_INCLUDE_RESOURCE_BITMAP_H_

#include <cstdint>
#include <memory>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

enum class BitmapAlphaType {
  kUnknown,           // uninitialized
  kPremul_AlphaType,  // pixel with 8 bits for red, green, blue, alpha; in
                      // 32-bit word
  kUnpremul_AlphaType,
};

enum class BitmapFormat {
  kUnknown,  // uninitialized
  kRGBA,     // pixel with 8 bits for red, green, blue, alpha; in 32-bit word
  kRGB565,   // pixel with 5 bits red, 6 bits green, 5 bits blue, in 16-bit word
};

class ANIMAX_EXPORT Bitmap {
 public:
  using BitmapDeleterContext = const void*;
  using BitmapDeleter = void (*)(const void*);

  static std::unique_ptr<Bitmap> MakeRGBA(uint32_t width, uint32_t height,
                                          const void* pixels,
                                          BitmapDeleter deleter,
                                          BitmapDeleterContext context);
  static std::unique_ptr<Bitmap> Make(
      uint32_t width, uint32_t height, const void* pixels,
      BitmapDeleter deleter, BitmapDeleterContext context,
      BitmapFormat format = BitmapFormat::kRGBA,
      BitmapAlphaType alpha = BitmapAlphaType::kPremul_AlphaType);
  ~Bitmap();
  uint32_t Width() const;
  uint32_t Height() const;
  const void* Pixels() const;
  uint32_t PixelsLength() const;
  uint32_t BytesPerPixel() const;
  uint32_t BytesPerRow() const;
  BitmapFormat Format() const;
  BitmapAlphaType AlphaType() const;

 private:
  Bitmap(uint32_t width, uint32_t height, const void* pixels,
         BitmapDeleter deleter, BitmapDeleterContext context,
         BitmapFormat format = BitmapFormat::kRGBA,
         BitmapAlphaType alpha = BitmapAlphaType::kPremul_AlphaType);
  uint32_t width_;
  uint32_t height_;
  const void* pixels_;
  BitmapDeleter deleter_;
  BitmapDeleterContext context_;
  BitmapFormat format_{BitmapFormat::kRGBA};
  BitmapAlphaType alpha_type_{BitmapAlphaType::kPremul_AlphaType};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_RESOURCE_BITMAP_H_
