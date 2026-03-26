// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image.h"

#include "skity/graphic/image.hpp"

namespace lynx {
namespace animax {

Image::~Image() = default;

float Image::GetWidth() const { return image_ ? image_->Width() : 0; }

float Image::GetHeight() const { return image_ ? image_->Height() : 0; }

std::shared_ptr<skity::Image> const &Image::GetImage() const { return image_; }

}  // namespace animax
}  // namespace lynx
