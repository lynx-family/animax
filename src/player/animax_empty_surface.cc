// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/player/animax_surface.h"

namespace lynx {
namespace animax {
namespace {

class EmptySurface final : public AnimaXSurface {
 public:
  EmptySurface(int32_t width, int32_t height) : AnimaXSurface(width, height) {}

  void Flush() override {}

  animax::Canvas* Canvas() override { return nullptr; }

  AnimaXBackend Type() const override { return AnimaXBackend::kEmpty; }

  bool Valid() const override { return true; }
};

}  // namespace

std::unique_ptr<AnimaXSurface> MakeEmptySurface(int32_t width, int32_t height) {
  return std::make_unique<EmptySurface>(width, height);
}

}  // namespace animax
}  // namespace lynx
