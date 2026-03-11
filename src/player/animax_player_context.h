// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_PLAYER_CONTEXT_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_PLAYER_CONTEXT_H_

#include <memory>

#include "base/include/lynx_actor.h"

namespace lynx {
namespace animax {
class AnimaXRenderer;
class AnimaXMainController;
class AnimaXAbility;
struct AnimaXPlayerContext
    : public std::enable_shared_from_this<AnimaXPlayerContext> {
  std::weak_ptr<shell::LynxActor<AnimaXRenderer>> weak_renderer_actor;
  std::weak_ptr<shell::LynxActor<AnimaXMainController>> weak_main_controller;
  std::weak_ptr<AnimaXAbility> weak_ability;

  ~AnimaXPlayerContext() = default;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_PLAYER_CONTEXT_H_
