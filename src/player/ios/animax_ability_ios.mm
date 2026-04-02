// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/ios/animax_ability_ios.h"

namespace lynx {
namespace animax {

bool AnimaXAbilityIOS::IsDownsampleVideoEnabled() const {
  return context_ ? context_.enableDownsampleVideo : false;
}

}  // namespace animax
}  // namespace lynx
