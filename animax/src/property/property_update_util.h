// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_UTIL_H_
#define ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_UTIL_H_

#include <memory>

#include "base/include/closure.h"
#include "src/property/property_update_context.h"

namespace lynx {
namespace animax {

class KeyframeAnimation;
class AnimaXValueCallback;
class KeyframeModel;

/**
 * Utility class for updating properties.
 * Provides static methods for updating different types of properties.
 */
class PropertyUpdateUtil {
 public:
  /**
   * Updates a keyframe animation based on the given context.
   * @param context The property update context containing update parameters
   * @param animation The target keyframe animation to update
   * @return PropertyUpdateResult
   */
  static PropertyUpdateResult UpdateLayerProperty(
      PropertyUpdateContext& context, KeyframeAnimation* animation);

  /**
   * Handles adding a value callback for a property.
   * Used by KeyPathElement implementations to process value callback requests.
   *
   * @param context The context containing callback information
   * @param animation The animation to attach the callback to
   * @return PropertyUpdateResult
   */
  static PropertyUpdateResult HandleValueCallback(
      PropertyUpdateContext& context, KeyframeAnimation* animation);

 private:
  /**
   * Adds a value callback to a keyframe animation.
   * @param AnimationType Type of the animation
   * @param animation The animation to add the callback to
   * @param callback The callback to add
   * @return PropertyUpdateResult
   */
  template <typename AnimationType>
  static PropertyUpdateResult AddLayerPropertyCallback(
      PropertyUpdateContext& context, KeyframeAnimation* animation,
      std::shared_ptr<AnimaXValueCallback> callback) {
    if (!animation) {
      return PropertyUpdateResult::kPropertyAnimationNull;
    }

    auto* typed_animation = static_cast<AnimationType*>(animation);
    typed_animation->SetValueCallback(callback);
    return PropertyUpdateResult::kSuccess;
  }

  /**
   * Updates a single keyframe in the animation.
   * @param context The property update context
   * @param animation The target animation
   * @param frame_index The index of the frame to update
   * @return PropertyUpdateResult
   */
  static PropertyUpdateResult UpdateKeyframe(PropertyUpdateContext& context,
                                             KeyframeAnimation* animation,
                                             int32_t frame_index);

  /**
   * Updates a keyframe animation with the given value type.
   * @param Animation The type of the animation
   * @param SrcType The type of the source value
   * @param TargetType The type of the target value
   * @param context The property update context
   * @param animation The target animation
   * @param frame_index The index of the frame to update
   * @param set_src The function to set the source value
   * @return PropertyUpdateResult
   */
  template <typename Animation, typename SrcType, typename TargetType>
  static PropertyUpdateResult UpdateKeyframeAnimation(
      PropertyUpdateContext& context, KeyframeAnimation* animation,
      int32_t frame_index,
      base::MoveOnlyClosure<void, TargetType*, SrcType*> set_src);

  /**
   * Internal implementation of keyframe animation update.
   * @param Animation The type of the animation
   * @param SrcType The type of the source value
   * @param TargetType The type of the target value
   * @param context The property update context
   * @param animation The target animation
   * @param frame_index The index of the frame to update
   * @param func The function to update the frame
   * @return PropertyUpdateResult
   */
  template <typename Animation, typename SrcType, typename TargetType>
  static PropertyUpdateResult UpdateKeyframeAnimationInternal(
      PropertyUpdateContext& context, KeyframeAnimation* animation,
      int32_t frame_index,
      base::MoveOnlyClosure<void, SrcType*, KeyframeModel*, bool> func);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_UTIL_H_
