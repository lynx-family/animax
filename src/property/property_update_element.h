// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_ELEMENT_H_
#define ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_ELEMENT_H_

#include <memory>

#include "src/animation/keyframe_animation.h"
#include "src/property/property_update_context.h"
#include "src/property/property_update_util.h"

namespace lynx {
namespace animax {

class ColorKeyframeAnimation;
class FloatKeyframeAnimation;

class PropertyUpdateElement {
 public:
  PropertyUpdateElement() = default;
  virtual ~PropertyUpdateElement() = default;

  /**
   * Updates properties based on the provided context.
   *
   * @param context The context containing property update information
   * @return PropertyUpdateResult
   */
  virtual PropertyUpdateResult UpdateLayerProperty(
      PropertyUpdateContext& context) {
    auto* animation = GetAnimationForProperty(context.GetLayerType());
    if (animation) {
      return PropertyUpdateUtil::UpdateLayerProperty(context, animation);
    } else {
      return PropertyUpdateResult::kSuccess;
    }
  }

  /**
   * Registers a value callback for dynamic property updates.
   *
   * @param context The context containing value callback information
   * @return PropertyUpdateResult
   */
  virtual PropertyUpdateResult AddLayerPropertyCallback(
      PropertyUpdateContext& context) {
    auto* animation = GetAnimationForProperty(context.GetLayerType());
    if (animation) {
      return PropertyUpdateUtil::HandleValueCallback(context, animation);
    } else {
      return PropertyUpdateResult::kSuccess;
    }
  }

  virtual KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) {
    return nullptr;
  }

  /**
   * Returns the animation for the given property type.
   * If the animation is not found, it creates a new one and returns it.
   *
   * @param animation_field The animation field to check
   * @return The animation for the given property type
   */
  template <typename Animation>
  KeyframeAnimation* GetOrCreateAnimation(
      std::unique_ptr<Animation>& animation, AnimationHost& host,
      AnimationListener* listener,
      LayerPropertyType type = LayerPropertyType::kUnknown) {
    if (animation == nullptr) {
      animation = MakeDefaultAnimation<Animation>(host, type);
      host.AddAnimation(animation.get());
      if (listener) {
        animation->AddUpdateListener(listener);
      }
    }
    if (listener) {
      listener->OnValueChanged();
    }
    return animation.get();
  }

 private:
  template <typename Animation>
  std::unique_ptr<Animation> MakeDefaultAnimation(AnimationHost& host,
                                                  LayerPropertyType type) {
    auto animation = MakeDocumentDefaultAnimation(
        host, type, static_cast<Animation*>(nullptr));
    return animation ? std::move(animation) : Animation::MakeDefault();
  }

  template <typename Animation>
  std::unique_ptr<Animation> MakeDocumentDefaultAnimation(AnimationHost&,
                                                          LayerPropertyType,
                                                          Animation*) {
    return nullptr;
  }

  std::unique_ptr<ColorKeyframeAnimation> MakeDocumentDefaultAnimation(
      AnimationHost& host, LayerPropertyType type, ColorKeyframeAnimation*);
  std::unique_ptr<FloatKeyframeAnimation> MakeDocumentDefaultAnimation(
      AnimationHost& host, LayerPropertyType type, FloatKeyframeAnimation*);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_ELEMENT_H_
