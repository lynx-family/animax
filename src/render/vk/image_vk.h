// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_VK_IMAGE_VK_H_
#define ANIMAX_SRC_RENDER_VK_IMAGE_VK_H_

#include <memory>

#include "src/render/image.h"

namespace skity {
class Texture;
}

namespace lynx {
namespace animax {

// Vulkan backend Image. Wraps a skity Texture imported from an
// AHardwareBuffer (GPUContext::WrapTexture with GPUBackendTextureExtInfoAHB),
// used to render GL-processed video frames under the Vulkan backend.
class ImageVK : public Image {
 public:
  explicit ImageVK(std::shared_ptr<skity::Texture> texture);
  ~ImageVK() override = default;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_VK_IMAGE_VK_H_
