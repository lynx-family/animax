// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/quaternion.h"

#include "skity/geometry/matrix.hpp"
#include "skity/geometry/quaternion.hpp"
#include "src/content/path/path_util.h"

namespace lynx {
namespace animax {
Quaternion::Quaternion()
    : quaternion_(std::make_unique<skity::Quaternion>(
          skity::Quaternion::FromXYZW(0, 0, 0, 1))) {}

Quaternion::Quaternion(float alpha_degree, float beta_degree,
                       float gamma_degree)
    : quaternion_(std::make_unique<skity::Quaternion>(
          skity::Quaternion::FromEuler(PathUtil::ToRadians(alpha_degree),
                                       PathUtil::ToRadians(beta_degree),
                                       PathUtil::ToRadians(gamma_degree)))) {}

Quaternion::Quaternion(Quaternion&& q)
    : quaternion_(std::move(q.quaternion_)) {}

Quaternion::~Quaternion() = default;

bool Quaternion::SphericalLinearInterpolation(const Quaternion& end,
                                              float progress,
                                              Quaternion& result) const {
  if (!quaternion_ || !end.quaternion_) {
    return false;
  }
  result.quaternion_ = std::make_unique<skity::Quaternion>(
      quaternion_->Slerp(*end.quaternion_, progress));
  return true;
}

bool Quaternion::AutoAdjustEndForInterpolation(const Quaternion& start) {
  if (!quaternion_ || !start.quaternion_) {
    return false;
  }

  auto &q1 = *start.quaternion_, &q2 = *quaternion_;
  auto q1_to_q2 = q2 * q1.Reciprocal();
  if (q1_to_q2.W() < 0) {
    quaternion_ = std::make_unique<skity::Quaternion>(q1_to_q2.Negative() * q1);
  }
  return true;
}

std::unique_ptr<Matrix> Quaternion::ToMatrix() const {
  if (!quaternion_) {
    return nullptr;
  }

  return std::make_unique<Matrix>(quaternion_->ToMatrix());
}

bool Quaternion::IsEmpty() const { return quaternion_ == nullptr; }

std::unique_ptr<Value> Quaternion::Copy() const {
  auto result = new Quaternion();
  if (quaternion_) {
    result->quaternion_ = std::make_unique<skity::Quaternion>(*quaternion_);
  }
  return std::unique_ptr<Value>(result);
}

void Quaternion::Reset() { quaternion_.reset(); }

}  // namespace animax
}  // namespace lynx
