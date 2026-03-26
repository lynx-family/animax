// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_HARMONY_ANIMAX_ABILITY_HARMONY_H_
#define ANIMAX_SRC_PLAYER_HARMONY_ANIMAX_ABILITY_HARMONY_H_

#include <cstdint>

#include "src/player/animax_ability.h"

namespace lynx {
namespace animax {

class AnimaXAbilityHarmony : public AnimaXAbility {
 public:
  AnimaXAbilityHarmony() = default;
  ~AnimaXAbilityHarmony() override = default;

  void SetVideoFrameTimeout(int32_t timeout_ms) {
    video_frame_timeout_ms_ = timeout_ms;
  }

  int32_t GetVideoFrameTimeout() const { return video_frame_timeout_ms_; }

 private:
  // Video frame timeout in milliseconds. 0 means use default behavior.
  int32_t video_frame_timeout_ms_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_HARMONY_ANIMAX_ABILITY_HARMONY_H_
