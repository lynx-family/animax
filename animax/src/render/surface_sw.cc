// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/surface_sw.h"

#include <cstring>

#include "skity/graphic/bitmap.hpp"
#include "skity/render/canvas.hpp"
#include "src/base/log/log.h"
#include "src/render/drawable_sw.h"

namespace lynx {
namespace animax {

SurfaceSW::SurfaceSW(std::unique_ptr<skity::Bitmap> bitmap)
    : bitmap_(std::move(bitmap)),
      canvas_(skity::Canvas::MakeSoftwareCanvas(bitmap_.get())),
      wrap_(new Canvas(canvas_.get(), bitmap_->Width(), bitmap_->Height(),
                       nullptr)) {}
SurfaceSW::~SurfaceSW() = default;

const DrawableSW &GetDrawableSW(const Drawable &drawable) {
  DCHECK(drawable.BackendType() == ContextBackend::kSoftware);
  return static_cast<const DrawableSW &>(drawable);
}

std::unique_ptr<SurfaceSW> SurfaceSW::Create(
    const lynx::animax::Drawable &drawable) {
  auto const &drawable_sw = GetDrawableSW(drawable);
  auto size = drawable_sw.Height() * drawable_sw.GetBytesPerRow();
  auto raw_data = skity::Data::MakeWithProc(drawable_sw.GetBuffer(), size,
                                            nullptr, nullptr);

  auto pixmap = std::make_shared<skity::Pixmap>(
      std::move(raw_data), drawable_sw.GetBytesPerRow(), drawable_sw.Width(),
      drawable_sw.Height(), skity::AlphaType::kPremul_AlphaType);

  return std::make_unique<SurfaceSW>(
      std::make_unique<skity::Bitmap>(std::move(pixmap), false));
}

void SurfaceSW::Clear() {
  std::memset(bitmap_->GetPixelAddr(), 0,
              bitmap_->Height() * bitmap_->RowBytes());
}

}  // namespace animax
}  // namespace lynx
