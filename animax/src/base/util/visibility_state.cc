// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/visibility_state.h"

namespace lynx {
namespace animax {

const char* StringifyVisibilityState(VisibilityState state) {
  switch (state) {
    case VisibilityState::kBackground:
      return "kBackground";
    case VisibilityState::kVisible:
      return "kVisible";
    case VisibilityState::kOpacity:
      return "kOpacity";
    case VisibilityState::kSize:
      return "kSize";
    case VisibilityState::kAttach:
      return "kAttach";
    case VisibilityState::kAggregated:
      return "kAggregated";
    default:
      return "Unknown";
  }
}

std::string StringifyVisibilityStateFlag(uint8_t state_flag) {
  std::string result;
  for (const auto& state : GetAllVisibilityStates()) {
    if (state_flag & static_cast<uint8_t>(state)) {
      if (!result.empty()) {
        result += "|";
      }
      result += StringifyVisibilityState(state);
    }
  }
  return result.empty() ? "None" : result;
}

}  // namespace animax
}  // namespace lynx
