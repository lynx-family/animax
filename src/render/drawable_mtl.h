// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_DRAWABLE_MTL_H_
#define ANIMAX_SRC_RENDER_DRAWABLE_MTL_H_

#import <Metal/Metal.h>

#include "src/render/drawable.h"

namespace lynx {
namespace animax {

enum class DrawableMTLBackendType { kInvalid = 0, kLayer, kTexture };

class DrawableMTL : public Drawable {
 public:
  DrawableMTL(DrawableMTLBackendType type, CAMetalLayer* layer,
              id<MTLTexture> texture, int32_t width, int32_t height)
      : Drawable{ContextBackend::kMetal, width, height},
        backend_type_{type},
        layer_{layer},
        texture_{texture} {}

  CAMetalLayer* Layer() const { return layer_; }
  id<MTLTexture> Texture() const { return texture_; }
  DrawableMTLBackendType GetType() const { return backend_type_; }

 private:
  DrawableMTLBackendType backend_type_;
  CAMetalLayer* layer_;
  id<MTLTexture> texture_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_DRAWABLE_MTL_H_
