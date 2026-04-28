// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_EMPTY_SURFACE_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_EMPTY_SURFACE_H_

#include <cstdint>
#include <memory>

namespace lynx {
namespace animax {

class AnimaXSurface;

std::unique_ptr<AnimaXSurface> MakeEmptySurface(int32_t width, int32_t height);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_EMPTY_SURFACE_H_
