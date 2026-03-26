// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Copyright 2018 Airbnb, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/parser/animatable/animatable_transform_parser.h"

#include <memory>

#include "src/parser/animatable/animatable_path_value_parser.h"
#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<AnimatableTransformModel> AnimatableTransformParser::Parse(
    rapidjson::Value& value, CompositionModel& composition, int32_t options) {
  std::unique_ptr<AnimatablePathValue> anchor_point;
  std::unique_ptr<AnimatablePointValue> position;
  std::unique_ptr<AnimatableScaleValue> scale;
  std::unique_ptr<AnimatableFloatValue> rotation;
  std::unique_ptr<AnimatableIntegerValue> opacity;
  std::unique_ptr<AnimatableFloatValue> start_opacity;
  std::unique_ptr<AnimatableFloatValue> end_opacity;
  std::unique_ptr<AnimatableFloatValue> skew;
  std::unique_ptr<AnimatableFloatValue> skew_angle;
  std::unique_ptr<AnimatableFloatValue> x_rotation;
  std::unique_ptr<AnimatableFloatValue> y_rotation;
  std::unique_ptr<AnimatableFloatValue> z_rotation;
  std::unique_ptr<AnimatableOrientationValue> orientation;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "a") == 0) {
      const auto& anchor_object = it->value.GetObject();
      for (auto anchor_it = anchor_object.MemberBegin();
           anchor_it != anchor_object.MemberEnd(); anchor_it++) {
        const auto& anchor_key = anchor_it->name.GetString();
        if (strcmp(anchor_key, "k") == 0) {
          anchor_point =
              AnimatablePathValueParser::Parse(anchor_it->value, composition);
        }
      }
    } else if (strcmp(key, "p") == 0) {
      position =
          AnimatablePathValueParser::ParseSplitPath(it->value, composition);
    } else if (strcmp(key, "s") == 0) {
      scale = AnimatableValueParser::ParseScale(it->value, composition);
    } else if (strcmp(key, "r") == 0) {
      rotation =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
      auto& keyframes = rotation->GetKeyframes();
      if (keyframes.empty()) {
        keyframes.emplace_back(std::make_unique<KeyframeModel>(
            composition, ValueFactory::Make<Float>(0),
            ValueFactory::Make<Float>(0), nullptr, 0,
            composition.GetEndFrame()));
      } else if (keyframes[0]->IsStartValueEmpty()) {
        keyframes.at(0) = std::make_unique<KeyframeModel>(
            composition, ValueFactory::Make<Float>(0),
            ValueFactory::Make<Float>(0), nullptr, 0,
            composition.GetEndFrame());
      }
    } else if (strcmp(key, "o") == 0) {
      opacity = AnimatableValueParser::ParseInteger(it->value, composition);
    } else if (strcmp(key, "so") == 0) {
      start_opacity =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "eo") == 0) {
      end_opacity =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "sk") == 0) {
      skew = AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "sa") == 0) {
      skew_angle =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "rx") == 0) {
      x_rotation =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "ry") == 0) {
      y_rotation =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "rz") == 0) {
      z_rotation =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "or") == 0) {
      orientation = std::unique_ptr<AnimatableOrientationValue>(
          new AnimatableOrientationValue());
      auto type = (options & Options::kForCamera)
                      ? ValueType::kCameraOrientation
                      : ValueType::kOrientation;
      AnimatableValueParser::Parse(it->value, composition,
                                   orientation->GetKeyframes(), type);
    }
  }

  // When should_reset_identity_ is false, it indicates that dynamic resources
  // are needed, providing a model for subsequent dynamic modifications of the
  // default property values.
  if (!composition.should_reset_identity_) {
    return std::make_unique<AnimatableTransformModel>(
        std::move(anchor_point), std::move(position), std::move(scale),
        std::move(rotation), std::move(opacity), std::move(skew),
        std::move(skew_angle), std::move(start_opacity), std::move(end_opacity),
        std::move(x_rotation), std::move(y_rotation), std::move(z_rotation),
        std::move(orientation));
  }

  if (IsAnchorPointIdentity(anchor_point.get())) {
    anchor_point.reset();
  }
  if (IsPositionIdentity(position.get())) {
    position.reset();
  }
  if (IsFloatValueIdentity(rotation.get())) {
    rotation.reset();
  }
  if (IsScaleIdentity(scale.get())) {
    scale.reset();
  }
  if (IsFloatValueIdentity(skew.get())) {
    skew.reset();
  }
  if (IsFloatValueIdentity(skew_angle.get())) {
    skew_angle.reset();
  }
  if (IsFloatValueIdentity(x_rotation.get())) {
    x_rotation.reset();
  }
  if (IsFloatValueIdentity(y_rotation.get())) {
    y_rotation.reset();
  }
  if (IsFloatValueIdentity(z_rotation.get())) {
    z_rotation.reset();
  }

  return std::make_unique<AnimatableTransformModel>(
      std::move(anchor_point), std::move(position), std::move(scale),
      std::move(rotation), std::move(opacity), std::move(skew),
      std::move(skew_angle), std::move(start_opacity), std::move(end_opacity),
      std::move(x_rotation), std::move(y_rotation), std::move(z_rotation),
      std::move(orientation));
}

bool AnimatableTransformParser::IsAnchorPointIdentity(
    AnimatablePathValue* anchor) {
  return anchor == nullptr ||
         (anchor->IsStatic() &&
          anchor->GetKeyframes()[0]->GetStartValue<PointF>()->Equals(0, 0, 0));
}

bool AnimatableTransformParser::IsPositionIdentity(
    AnimatablePointValue* position) {
  if (position == nullptr) {
    return true;
  }
  return position->Type() != ValueType::kSplitPath && position->IsStatic() &&
         position->GetKeyframes()[0]->GetStartValue<PointF>()->Equals(0, 0, 0);
}

bool AnimatableTransformParser::IsFloatValueIdentity(
    AnimatableFloatValue* float_value) {
  return float_value == nullptr ||
         (float_value->IsStatic() &&
          float_value->GetKeyframes()[0]->GetStartValue<Float>()->Get() == 0);
}

bool AnimatableTransformParser::IsScaleIdentity(AnimatableScaleValue* scale) {
  return scale == nullptr ||
         (scale->IsStatic() &&
          scale->GetKeyframes()[0]->GetStartValue<ScaleF>()->Equals(1, 1, 1));
}

}  // namespace animax
}  // namespace lynx
