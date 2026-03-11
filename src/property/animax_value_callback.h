// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANIMAX_VALUE_CALLBACK_H_
#define ANIMAX_SRC_PROPERTY_ANIMAX_VALUE_CALLBACK_H_

#include <memory>

#include "src/model/value/base_value.h"
#include "src/property/animax_frame_info.h"

namespace lynx {
namespace animax {
/**
 * Base value callback interface for dynamic property manipulation.
 * This is an abstract class that defines the interface for all value callbacks.
 *
 * Value callbacks allow dynamic modification of animation properties at runtime
 * without modifying the original animation data. They intercept property
 * values during the rendering process and can modify or override them.
 */
class AnimaXValueCallback {
 public:
  AnimaXValueCallback() = default;
  virtual ~AnimaXValueCallback() = default;

  /**
   * Gets the value for the given frame.
   *
   * @param original_value The original value from the animation
   * @param frame_info Info about the current animation frame
   * @return The modified value to use instead of original_value, or nullptr to
   * use original
   */
  virtual std::unique_ptr<Value> GetValue(
      const Value* original_value, const AnimaXFrameInfo& frame_info) = 0;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANIMAX_VALUE_CALLBACK_H_
