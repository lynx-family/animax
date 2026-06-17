// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/vk/image_vk.h"

#include "skity/gpu/texture.hpp"
#include "skity/graphic/image.hpp"

namespace lynx {
namespace animax {

ImageVK::ImageVK(std::shared_ptr<skity::Texture> texture) {
  image_ = skity::Image::MakeHWImage(std::move(texture));
}

}  // namespace animax
}  // namespace lynx
