// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_BASE_VISIBILITY_STATE_H_
#define ANIMAX_INCLUDE_BASE_VISIBILITY_STATE_H_

#include <cstdint>

namespace lynx {
namespace animax {

enum class VisibilityState : uint8_t {
  // Changed when app enter foreground or background
  kBackground = 1,
  // Changed when platform view's setVisibility called
  kVisible = 1 << 1,
  // Changed when platform view's setAlpha called
  kOpacity = 1 << 2,
  // Changed when platform view's width or height update
  kSize = 1 << 3,
  // Changed when platform view's attach or detach from window
  kAttach = 1 << 4,
  // Changed when platform view's parent visibility is changed, affecting this
  // view's
  // aggregated visibility state. Represents the true visibility of a view based
  // on
  // the entire hierarchy - the view is only considered visible when both itself
  // and
  // all ancestors are visible and within screen bounds.
  kAggregated = 1 << 5
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_BASE_VISIBILITY_STATE_H_
