// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BRIDGE_IOS_ANIMAX_ABILITY_IOS_H_
#define ANIMAX_SRC_BRIDGE_IOS_ANIMAX_ABILITY_IOS_H_

#include <AnimaX/AnimaXContext.h>
#include "src/base/log/log.h"
#include "src/player/animax_ability.h"

namespace lynx {
namespace animax {

class AnimaXAbilityIOS : public AnimaXAbility {
 public:
  explicit AnimaXAbilityIOS(AnimaXContext* context) : context_(context) { DCHECK(context); }

  bool EnableOptVtbErrorHandler() const { return [context_ enableOptVtbErrorHandler]; }

 private:
  AnimaXContext* context_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BRIDGE_IOS_ANIMAX_ABILITY_IOS_H_
