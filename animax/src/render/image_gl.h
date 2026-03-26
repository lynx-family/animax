// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_IMAGE_GL_H_
#define ANIMAX_SRC_RENDER_IMAGE_GL_H_

#include "src/render/image.h"

namespace lynx {
namespace animax {

class Bitmap;
class RealContext;
class TextureInfo;

class ImageGL : public Image {
 public:
  explicit ImageGL(std::unique_ptr<Bitmap> bitmap, RealContext *real_context);
  explicit ImageGL(TextureInfo *texture, RealContext *real_context);

  ~ImageGL() override = default;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_IMAGE_GL_H_
