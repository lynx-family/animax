// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_VISIBILITY_STATE_H_
#define ANIMAX_SRC_BASE_UTIL_VISIBILITY_STATE_H_

#include <array>
#include <cstdint>
#include <string>

#include "include/base/visibility_state.h"

namespace lynx {
namespace animax {

// Total number of visibility states
static constexpr std::size_t kVisibilityStateCount = 6;

constexpr std::array<VisibilityState, kVisibilityStateCount>
GetAllVisibilityStates() {
  return {VisibilityState::kBackground, VisibilityState::kVisible,
          VisibilityState::kOpacity,    VisibilityState::kSize,
          VisibilityState::kAttach,     VisibilityState::kAggregated};
}

const char* StringifyVisibilityState(VisibilityState state);

std::string StringifyVisibilityStateFlag(uint8_t state_flag);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_VISIBILITY_STATE_H_
