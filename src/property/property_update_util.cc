// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/property_update_util.h"

#include "include/property/animax_value_param.h"
#include "include/property/property_update_response.h"
#include "src/animation/base_keyframe_animation.h"
#include "src/animation/text_keyframe_animation.h"
#include "src/model/keyframe/keyframe_model.h"

namespace lynx {
namespace animax {

PropertyUpdateResult PropertyUpdateUtil::HandleValueCallback(
    PropertyUpdateContext& context, KeyframeAnimation* animation) {
  if (!animation) {
    return PropertyUpdateResult::kPropertyAnimationNull;
  }

  auto callback = context.GetValueCallback();

  // Process based on animation type
  switch (animation->Type()) {
    case ValueType::kFloat:
      return AddLayerPropertyCallback<FloatKeyframeAnimation>(
          context, animation, callback);

    case ValueType::kInteger:
      return AddLayerPropertyCallback<IntegerKeyframeAnimation>(
          context, animation, callback);

    case ValueType::kPoint:
      return AddLayerPropertyCallback<PointKeyframeAnimation>(
          context, animation, callback);

    case ValueType::kScale:
      return AddLayerPropertyCallback<ScaleKeyframeAnimation>(
          context, animation, callback);

    case ValueType::kColor:
      return AddLayerPropertyCallback<ColorKeyframeAnimation>(
          context, animation, callback);

    case ValueType::kColorFilter:
      return AddLayerPropertyCallback<ColorFilterKeyframeAnimation>(
          context, animation, callback);

    case ValueType::kDocument:
      return AddLayerPropertyCallback<TextKeyframeAnimation>(context, animation,
                                                             callback);

    default:
      return PropertyUpdateResult::kValueInvalid;
  }
}

PropertyUpdateResult PropertyUpdateUtil::UpdateLayerProperty(
    PropertyUpdateContext& context, KeyframeAnimation* animation) {
  if (!animation) {
    return PropertyUpdateResult::kPropertyAnimationNull;
  }

  const auto frame_index = context.GetValue().GetTargetFrame();
  if (frame_index == AnimaXValueParam::kFrameIndexAll) {
    for (auto index = 0; index <= animation->GetSize(); index++) {
      auto result = UpdateKeyframe(context, animation, index);
      if (result != PropertyUpdateResult::kSuccess) {
        return result;
      }
    }
    return PropertyUpdateResult::kSuccess;
  } else if (frame_index >= 0) {
    return UpdateKeyframe(context, animation, frame_index);
  } else {
    return PropertyUpdateResult::kFrameIndexInvalid;
  }
}

PropertyUpdateResult PropertyUpdateUtil::UpdateKeyframe(
    PropertyUpdateContext& context, KeyframeAnimation* animation,
    int32_t frame_index) {
  switch (animation->Type()) {
    case ValueType::kInteger: {
      return UpdateKeyframeAnimation<IntegerKeyframeAnimation, double, Integer>(
          context, animation, frame_index,
          [](auto value, auto src_ptr) { value->Set(*src_ptr); });
    }
    case ValueType::kFloat: {
      return UpdateKeyframeAnimation<FloatKeyframeAnimation, double, Float>(
          context, animation, frame_index,
          [](auto value, auto src_ptr) { value->Set(*src_ptr); });
    }
    case ValueType::kPoint: {
      return UpdateKeyframeAnimation<PointKeyframeAnimation, PointF, PointF>(
          context, animation, frame_index, [](auto value, auto src_ptr) {
            value->Set(src_ptr->GetX(), src_ptr->GetY(), src_ptr->GetZ());
          });
    }
    case ValueType::kScale: {
      return UpdateKeyframeAnimation<ScaleKeyframeAnimation, PointF, ScaleF>(
          context, animation, frame_index, [](auto value, auto src_ptr) {
            value->Set(src_ptr->GetX(), src_ptr->GetY(), src_ptr->GetZ());
          });
    }
    case ValueType::kColor: {
      return UpdateKeyframeAnimation<ColorKeyframeAnimation, Color, Color>(
          context, animation, frame_index,
          [](auto value, auto src_ptr) { value->Set(src_ptr->GetInt()); });
    }
    case ValueType::kDocument: {
      return UpdateKeyframeAnimation<TextKeyframeAnimation, std::string,
                                     DocumentData>(
          context, animation, frame_index,
          [](auto value, auto src_ptr) { value->SetText(*src_ptr); });
    }
    default: {
      break;
    }
  }
  return PropertyUpdateResult::kValueInvalid;
}

template <typename Animation, typename SrcType, typename TargetType>
PropertyUpdateResult PropertyUpdateUtil::UpdateKeyframeAnimation(
    PropertyUpdateContext& context, KeyframeAnimation* animation,
    int32_t frame_index,
    base::MoveOnlyClosure<void, TargetType*, SrcType*> set_src) {
  auto func = [set_src = std::move(set_src)](
                  SrcType* src_ptr, KeyframeModel* frame, bool update_start) {
    if (update_start) {
      auto* start_value = frame->template GetStartValue<TargetType>();
      if (start_value) {
        set_src(start_value, src_ptr);
      }
    } else {
      auto* end_value = frame->template GetEndValue<TargetType>();
      if (end_value) {
        set_src(end_value, src_ptr);
      }
    }
    frame->OnValueChanged();
  };

  return UpdateKeyframeAnimationInternal<Animation, SrcType, TargetType>(
      context, animation, frame_index, std::move(func));
}

template <typename Animation, typename SrcType, typename TargetType>
PropertyUpdateResult PropertyUpdateUtil::UpdateKeyframeAnimationInternal(
    PropertyUpdateContext& context, KeyframeAnimation* animation,
    int32_t frame_index,
    base::MoveOnlyClosure<void, SrcType*, KeyframeModel*, bool> func) {
  auto* target_animation = static_cast<Animation*>(animation);
  auto* src_ptr = static_cast<SrcType*>(context.GetValue().GetValuePtr());
  if (!target_animation || !src_ptr) {
    return PropertyUpdateResult::kPropertyAnimationNull;
  }

  auto* current_frame = target_animation->GetKeyframe(frame_index);
  auto* prev_frame = target_animation->GetKeyframe(frame_index - 1);

  if (frame_index == 0) {
    // Only update the first key frame.
    if (current_frame) {
      func(src_ptr, current_frame, true);
    } else {
      return PropertyUpdateResult::kFrameIndexInvalid;
    }
  } else if (frame_index == target_animation->GetSize()) {
    // Only update the last key frame.
    if (prev_frame) {
      func(src_ptr, prev_frame, false);
    } else {
      return PropertyUpdateResult::kFrameIndexInvalid;
    }
  } else {
    if (current_frame && prev_frame) {
      func(src_ptr, current_frame, true);
      func(src_ptr, prev_frame, false);
    } else {
      return PropertyUpdateResult::kFrameIndexInvalid;
    }
  }
  target_animation->OnValueChanged();
  return PropertyUpdateResult::kSuccess;
}

}  // namespace animax
}  // namespace lynx
