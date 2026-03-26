// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_CAMERA_H_
#define ANIMAX_SRC_RENDER_CAMERA_H_

#include <memory>

#include "src/render/matrix.h"

namespace skity {
class Camera;
}
namespace lynx {
namespace animax {

class Camera {
 public:
  Camera(float viewport_width, float viewport_height, bool fixed);
  Camera() = delete;
  ~Camera();

  void SetPosition(float x, float y, float z);
  void LookAt(float x, float y, float z);
  void SetDistance(float distance);
  void SetRotation(const Matrix* rotate);

  std::unique_ptr<Matrix> GetMatrix();

 private:
  void UseFixedDistance(float width, float height);
  std::unique_ptr<skity::Camera> camera_;
  const bool fixed_ = true;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_CAMERA_H_
