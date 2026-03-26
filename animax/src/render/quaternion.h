// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_QUATERNION_H_
#define ANIMAX_SRC_RENDER_QUATERNION_H_

#include <memory>

#include "src/model/value/base_value.h"
#include "src/render/matrix.h"

namespace skity {
class Quaternion;
}
namespace lynx {
namespace animax {

class Quaternion : public Value {
 public:
  Quaternion();
  ~Quaternion();

  Quaternion(float alpha_degree, float beta_degree, float gamma_degree);
  Quaternion(Quaternion&& q);

  bool SphericalLinearInterpolation(const Quaternion& end, float progress,
                                    Quaternion& result) const;

  std::unique_ptr<Matrix> ToMatrix() const;

  bool AutoAdjustEndForInterpolation(const Quaternion& start);

  bool IsEmpty() const override;

  std::unique_ptr<Value> Copy() const override;

  ValueType GetValueType() const override { return ValueType::kOrientation; }

  void Reset();

 private:
  std::unique_ptr<skity::Quaternion> quaternion_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_QUATERNION_H_
