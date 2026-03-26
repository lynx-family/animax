// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/camera.h"

#include <algorithm>

#include "skity/geometry/camera.hpp"
#include "src/content/path/path_util.h"

namespace lynx {
namespace animax {

Camera::Camera(float viewport_width, float viewport_height, bool fixed)
    : camera_(std::make_unique<skity::Camera>(viewport_width, viewport_height)),
      fixed_(fixed) {
  if (fixed) {
    UseFixedDistance(viewport_width, viewport_height);
  }
}
Camera::~Camera() = default;

void Camera::SetPosition(float x, float y, float z) {
  camera_->SetPosition(skity::Point{x, y, z, 1});
}
void Camera::LookAt(float x, float y, float z) {
  camera_->LookAt(skity::Point{x, y, z, 1});
}

void Camera::SetDistance(float distance) { camera_->SetCameraDist(distance); }

void Camera::SetRotation(const Matrix* rotate) {
  if (!rotate) {
    return;
  }
  camera_->SetRotation(rotate->GetMatrix());
}

std::unique_ptr<Matrix> Camera::GetMatrix() {
  if (fixed_) {
    return std::make_unique<Matrix>(camera_->GetFixedCamera());
  } else {
    return std::make_unique<Matrix>(camera_->GetCamera());
  }
}

void Camera::UseFixedDistance(float width, float height) {
  static constexpr float field_of_view_degrees = 39.6f;
  float half_field_of_view_rad =
      PathUtil::ToRadians(field_of_view_degrees / 2.0f);
  float distance =
      (std::max(width, height) / 2.0f) / std::tan(half_field_of_view_rad);
  camera_->SetCameraDist(distance);
}

}  // namespace animax
}  // namespace lynx
