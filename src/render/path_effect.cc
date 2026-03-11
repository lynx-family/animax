// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/path_effect.h"

#include "skity/effect/path_effect.hpp"

namespace lynx {
namespace animax {

std::unique_ptr<DashPathEffect> DashPathEffect::Make(const float* values,
                                                     size_t size,
                                                     float offset) {
  return std::make_unique<DashPathEffect>(skity::PathEffect::MakeDashPathEffect(
      values, static_cast<int>(size), offset));
}

DashPathEffect::DashPathEffect(std::shared_ptr<skity::PathEffect> effect)
    : skity_effect_(std::move(effect)) {}

}  // namespace animax
}  // namespace lynx
