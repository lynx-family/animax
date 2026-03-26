// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_IMAGE_H_
#define ANIMAX_SRC_RENDER_IMAGE_H_

#include <memory>

namespace skity {
class Image;
}
namespace lynx {
namespace animax {

class Image {
 public:
  virtual ~Image();

  float GetWidth() const;

  float GetHeight() const;

  std::shared_ptr<skity::Image> const &GetImage() const;

 protected:
  std::shared_ptr<skity::Image> image_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_IMAGE_H_
