// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/property_update_element.h"

#include <memory>
#include <utility>

#include "src/animation/base_keyframe_animation.h"
#include "src/animation/text_keyframe_animation.h"
#include "src/model/composition_model.h"
#include "src/model/keyframe/keyframe_model.h"
#include "src/model/value/document_data.h"

namespace lynx {
namespace animax {
namespace {

TextKeyframeAnimation* FindTextKeyframeAnimation(AnimationHost& host) {
  for (auto* animation : host.GetAnimations()) {
    if (animation && animation->GetSize() > 0 &&
        animation->Type() == ValueType::kDocument) {
      return static_cast<TextKeyframeAnimation*>(animation);
    }
  }
  return nullptr;
}

std::unique_ptr<Value> MakeDefaultDocumentPropertyValue(
    LayerPropertyType type) {
  switch (type) {
    case LayerPropertyType::kTextSize:
    case LayerPropertyType::kTextTracking:
    case LayerPropertyType::kStrokeWidth:
      return ValueFactory::Make<Float>(0.f);
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kColor:
    case LayerPropertyType::kStrokeColor:
      return ValueFactory::Make<Color>(0);
    default:
      return nullptr;
  }
}

std::unique_ptr<Value> MakeDocumentPropertyValue(DocumentData* document,
                                                 LayerPropertyType type) {
  if (!document) {
    return MakeDefaultDocumentPropertyValue(type);
  }

  switch (type) {
    case LayerPropertyType::kTextSize:
      return ValueFactory::Make<Float>(document->GetSize());
    case LayerPropertyType::kTextTracking:
      return MakeDefaultDocumentPropertyValue(type);
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kColor:
      return ValueFactory::Make<Color>(document->GetColor());
    case LayerPropertyType::kStrokeColor:
      return ValueFactory::Make<Color>(document->GetStrokeColor());
    case LayerPropertyType::kStrokeWidth:
      return ValueFactory::Make<Float>(document->GetStrokeWidth());
    default:
      return nullptr;
  }
}

std::unique_ptr<KeyframeModel> MakeDocumentPropertyKeyframe(
    KeyframeModel& document_keyframe, LayerPropertyType type) {
  auto* start_document = document_keyframe.GetStartValue<DocumentData>();
  auto* end_document = document_keyframe.GetEndValue<DocumentData>();
  auto start_value = MakeDocumentPropertyValue(start_document, type);
  auto end_value = MakeDocumentPropertyValue(
      end_document ? end_document : start_document, type);
  if (!start_value) {
    return nullptr;
  }
  if (!end_value) {
    end_value = start_value->Copy();
  }

  auto* composition = document_keyframe.GetComposition();
  if (composition) {
    return std::make_unique<KeyframeModel>(
        *composition, std::move(start_value), std::move(end_value),
        std::unique_ptr<Interpolator>(), document_keyframe.GetStartFrame(),
        document_keyframe.GetEndFrame());
  }

  auto keyframe = std::make_unique<KeyframeModel>(std::move(start_value));
  keyframe->SetEndValue(std::move(end_value));
  keyframe->SetStartFrame(document_keyframe.GetStartFrame());
  keyframe->SetEndFrame(document_keyframe.GetEndFrame());
  return keyframe;
}

std::unique_ptr<KeyframeModel> MakeTerminalDocumentPropertyKeyframe(
    KeyframeModel& document_keyframe, LayerPropertyType type) {
  // TextKeyframeAnimation can render the final DocumentData end value at the
  // last boundary. Document-derived typed animations are discrete, so expose
  // that final value as a terminal start value that seek/update can reach.
  auto* composition = document_keyframe.GetComposition();
  auto* end_document = document_keyframe.GetEndValue<DocumentData>();
  if (!composition || !end_document ||
      document_keyframe.GetEndFrame() == Float::kMax) {
    return nullptr;
  }

  auto start_value = MakeDocumentPropertyValue(end_document, type);
  if (!start_value) {
    return nullptr;
  }
  auto end_value = start_value->Copy();
  auto start_frame = document_keyframe.GetEndFrame();
  if (start_frame == Float::kMin) {
    start_frame = composition->GetEndFrame();
  }

  return std::make_unique<KeyframeModel>(
      *composition, std::move(start_value), std::move(end_value),
      std::unique_ptr<Interpolator>(), start_frame, Float::kMin);
}

std::shared_ptr<KeyframeModelList> MakeDocumentPropertyKeyframes(
    AnimationHost& host, LayerPropertyType type) {
  auto* text_keyframe_animation = FindTextKeyframeAnimation(host);
  if (!text_keyframe_animation) {
    return nullptr;
  }

  auto frames = std::make_shared<KeyframeModelList>();
  const auto size = text_keyframe_animation->GetSize();
  for (auto index = 0; index < size; ++index) {
    auto* document_keyframe = text_keyframe_animation->GetKeyframe(index);
    if (!document_keyframe) {
      continue;
    }
    auto keyframe = MakeDocumentPropertyKeyframe(*document_keyframe, type);
    if (keyframe) {
      frames->push_back(std::move(keyframe));
    }
  }
  if (auto* last_document_keyframe = text_keyframe_animation->GetKeyframe(
          text_keyframe_animation->GetSize() - 1)) {
    auto terminal_keyframe =
        MakeTerminalDocumentPropertyKeyframe(*last_document_keyframe, type);
    if (terminal_keyframe) {
      frames->push_back(std::move(terminal_keyframe));
    }
  }
  if (frames->empty()) {
    return nullptr;
  }
  return frames;
}

}  // namespace

std::unique_ptr<ColorKeyframeAnimation>
PropertyUpdateElement::MakeDocumentDefaultAnimation(AnimationHost& host,
                                                    LayerPropertyType type,
                                                    ColorKeyframeAnimation*) {
  switch (type) {
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kColor:
    case LayerPropertyType::kStrokeColor: {
      auto frames = MakeDocumentPropertyKeyframes(host, type);
      if (!frames) {
        return nullptr;
      }
      auto animation =
          std::make_unique<ColorKeyframeAnimation>(std::move(frames));
      animation->SetIsDiscrete();
      return animation;
    }
    default:
      return nullptr;
  }
}

std::unique_ptr<FloatKeyframeAnimation>
PropertyUpdateElement::MakeDocumentDefaultAnimation(AnimationHost& host,
                                                    LayerPropertyType type,
                                                    FloatKeyframeAnimation*) {
  switch (type) {
    case LayerPropertyType::kTextSize:
    case LayerPropertyType::kTextTracking:
    case LayerPropertyType::kStrokeWidth: {
      auto frames = MakeDocumentPropertyKeyframes(host, type);
      if (!frames) {
        return nullptr;
      }
      auto animation =
          std::make_unique<FloatKeyframeAnimation>(std::move(frames));
      animation->SetIsDiscrete();
      return animation;
    }
    default:
      return nullptr;
  }
}

}  // namespace animax
}  // namespace lynx
