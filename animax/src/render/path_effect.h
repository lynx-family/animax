// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_PATH_EFFECT_H_
#define ANIMAX_SRC_RENDER_PATH_EFFECT_H_

#include <memory>

namespace skity {
class PathEffect;
}
namespace lynx {
namespace animax {

class DashPathEffect {
 public:
  static std::unique_ptr<DashPathEffect> Make(const float* values, size_t size,
                                              float offset);

  DashPathEffect(std::shared_ptr<skity::PathEffect> effect);
  ~DashPathEffect() = default;

  std::shared_ptr<skity::PathEffect> GetEffect() { return skity_effect_; }

 private:
  std::shared_ptr<skity::PathEffect> skity_effect_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_PATH_EFFECT_H_
