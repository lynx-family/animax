// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/camera_layer.h"

#include <algorithm>

#include "src/render/matrix.h"
#include "src/render/quaternion.h"

namespace lynx {
namespace animax {

CameraLayer::CameraLayer(LayerModel& layer, CompositionModel& composition)
    : BaseLayer(layer, composition) {
  auto* perspective = layer.GetPerspective();
  if (perspective) {
    perspective_ = perspective->CreateAnimation();
    AddAnimation(perspective_.get());
  }
}

void CameraLayer::Init() {
  BaseLayer::Init();
  if (perspective_) {
    perspective_->AddUpdateListener(this);
  }
}

void CameraLayer::DrawLayer(Canvas& canvas, Matrix& parent_matrix,
                            int32_t parent_alpha) {
  // do nothing
}

bool CameraLayer::ConfigCamera(Camera* camera) {
  if (transform_ == nullptr || camera == nullptr) {
    return false;
  }

  auto* position = transform_->GetPosition();
  if (position == nullptr) {
    return false;
  }

  auto& value = position->GetValue();
  camera->SetPosition(value.GetX(), value.GetY(), value.GetZ());

  auto* anchor_point = transform_->GetAnchorPoint();
  if (anchor_point) {
    auto& value = anchor_point->GetValue();
    camera->LookAt(value.GetX(), value.GetY(), value.GetZ());
  } else {
    auto& value = position->GetValue();
    camera->LookAt(value.GetX(), value.GetY(), 0);
  }

  std::unique_ptr<Matrix> orientation_matrix = nullptr;
  auto* orientation = transform_->GetOrientation();
  if (orientation) {
    orientation_matrix = orientation->GetValue().ToMatrix();
  }

  float alpha = 0, beta = 0, gamma = 0;
  auto* x_rotation = transform_->GetXRotation();
  if (x_rotation) {
    alpha = x_rotation->GetValue().Get();
  }
  auto* y_rotation = transform_->GetYRotation();
  if (y_rotation) {
    beta = y_rotation->GetValue().Get();
  }
  auto* z_rotation = transform_->GetZRotation();
  if (z_rotation) {
    gamma = z_rotation->GetValue().Get();
  }

  if (alpha != 0 || beta != 0 || gamma != 0) {
    Matrix euler_matrix;
    // For the camera, the rotation direction around the z-axis is opposite.
    euler_matrix.PreRotateXYZ(alpha, beta, -gamma);
    if (orientation_matrix) {
      // If an orientation matrix already exists, pre-concat the euler matrix.
      // This effectively applies the euler rotation after the orientation.
      euler_matrix.PreConcat(*orientation_matrix);
    }
    camera->SetRotation(&euler_matrix);

  } else if (orientation_matrix) {
    camera->SetRotation(orientation_matrix.get());
  }

  float distance = 0;
  if (perspective_) {
    distance = perspective_->GetValue().Get();
  } else {
    distance = position->GetValue().GetZ();
  }
  camera->SetDistance(std::max(distance, 0.1f));
  return true;
}

}  // namespace animax
}  // namespace lynx
