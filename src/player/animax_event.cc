// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_event.h"

namespace lynx {
namespace animax {

const char* StringifyEventWarning(EventWarning warning) {
  switch (warning) {
    case EventWarning::kNoWarning:
      return "kNoWarning";
    case EventWarning::kInvalidLottieFormat:
      return "kInvalidLottieFormat";
    case EventWarning::kIllegalAlphaVideoSize:
      return "kIllegalAlphaVideoSize";
    case EventWarning::kAssetCountOverLimit:
      return "kAssetCountOverLimit";
    case EventWarning::kDynamicResourceCannotBeLoaded:
      return "kDynamicResourceCannotBeLoaded";
    case EventWarning::kStartFrameGreaterThanEndFrame:
      return "kStartFrameGreaterThanEndFrame";
    case EventWarning::kExecuteBeforeReady:
      return "kExecuteBeforeReady";
    case EventWarning::kIllegalSubscribedStartFrame:
      return "kIllegalSubscribedStartFrame";
  }
}

}  // namespace animax
}  // namespace lynx
