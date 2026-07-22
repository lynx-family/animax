// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/parser/custom/custom_feature_parser.h"

#include <string.h>

#include "src/model/composition_model.h"
#include "src/parser/custom/text_gradient_parser.h"

namespace lynx {
namespace animax {

void CustomFeatureParser::Parse(rapidjson::Value& value,
                                CompositionModel& composition) {
  if (!value.IsObject()) {
    return;
  }

  const auto& object = value.GetObject();
  auto custom_it = object.FindMember("custom");
  if (custom_it == object.MemberEnd() || !custom_it->value.IsObject()) {
    return;
  }

  const auto& custom = custom_it->value.GetObject();
  auto animax_it = custom.FindMember("animax");
  if (animax_it == custom.MemberEnd() || !animax_it->value.IsObject()) {
    return;
  }

  const auto& animax = animax_it->value.GetObject();
  auto version_it = animax.FindMember("v");
  if (version_it == animax.MemberEnd() || !version_it->value.IsInt() ||
      version_it->value.GetInt() != 1) {
    return;
  }

  auto features_it = animax.FindMember("fs");
  if (features_it == animax.MemberEnd() || !features_it->value.IsObject()) {
    return;
  }

  const auto& features = features_it->value.GetObject();
  for (auto feature_it = features.MemberBegin();
       feature_it != features.MemberEnd(); feature_it++) {
    const auto& feature_key = feature_it->name.GetString();
    if (strcmp(feature_key, "tg") == 0) {
      TextGradientParser::Parse(feature_it->value, composition);
    }
  }
}

}  // namespace animax
}  // namespace lynx
