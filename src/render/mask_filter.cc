// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/mask_filter.h"

#include "skity/effect/mask_filter.hpp"

namespace lynx {
namespace animax {

MaskFilter::MaskFilter(std::shared_ptr<skity::MaskFilter> filter)
    : mask_filter_(std::move(filter)) {}

MaskFilter::~MaskFilter() = default;

std::unique_ptr<MaskFilter> MaskFilter::MakeBlur(float radius) {
  return std::make_unique<MaskFilter>(
      skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal, radius));
}

}  // namespace animax
}  // namespace lynx
