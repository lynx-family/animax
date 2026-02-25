// Copyright 2023 The Lynx Authors. All rights reserved.
// Copyright 2018 Airbnb, Inc. All rights reserved.

// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/parser/mask_parser.h"

#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<MaskModel> MaskParser::Parse(rapidjson::Value& value,
                                             CompositionModel& composition) {
  MaskMode mask_mode;
  std::unique_ptr<AnimatableShapeValue> mask_path;
  std::unique_ptr<AnimatableIntegerValue> opacity;
  bool inverted = false;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& name = it->name.GetString();
    if (strcmp(name, "mode") == 0) {
      const auto& mode_name = it->value.GetString();
      if (strcmp(mode_name, "a") == 0) {
        mask_mode = MaskMode::kAdd;
      } else if (strcmp(mode_name, "s") == 0) {
        mask_mode = MaskMode::kSubtract;
      } else if (strcmp(mode_name, "n") == 0) {
        mask_mode = MaskMode::kNone;
      } else if (strcmp(mode_name, "i") == 0) {
        mask_mode = MaskMode::kIntersect;
      } else {
        mask_mode = MaskMode::kAdd;
      }
    } else if (strcmp(name, "pt") == 0) {
      mask_path = AnimatableValueParser::ParseShapeData(it->value, composition);
    } else if (strcmp(name, "o") == 0) {
      opacity = AnimatableValueParser::ParseInteger(it->value, composition);
    } else if (strcmp(name, "inv") == 0) {
      inverted = it->value.GetBool();
    }
  }

  return std::unique_ptr<MaskModel>(new MaskModel(
      mask_mode, std::move(mask_path), std::move(opacity), inverted));
}

}  // namespace animax
}  // namespace lynx
