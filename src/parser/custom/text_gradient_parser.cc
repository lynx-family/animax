// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/parser/custom/text_gradient_parser.h"

#include <string.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/model/text/text_gradient_model.h"
#include "src/parser/value_parser.h"

namespace lynx {
namespace animax {
namespace {

enum class TextGradientType : int32_t {
  kLinear = 1,
  kRadial = 2,
  kConic = 3,
};

// Text-gradient coordinates are normalized to the text bounds and gradient
// stops are concrete values; no keyframe animations or composition scaling are
// applied while parsing this extension.
std::unique_ptr<TextGradientItemModel> ParseItem(rapidjson::Value& value,
                                                 int32_t& layer_id,
                                                 std::string& ref_id,
                                                 ParseContext& context) {
  if (!value.IsObject()) {
    return nullptr;
  }

  int32_t type_value = 0;
  std::unique_ptr<PointF> start_point;
  std::unique_ptr<PointF> end_point;
  std::unique_ptr<PointF> radius_xy;
  std::optional<float> start_angle;
  std::unique_ptr<GradientColor> gradient_color;
  rapidjson::Value* gradient_value = nullptr;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "ind") == 0) {
      if (!it->value.IsInt()) {
        return nullptr;
      }
      layer_id = it->value.GetInt();
    } else if (strcmp(key, "refId") == 0) {
      if (!it->value.IsString()) {
        return nullptr;
      }
      ref_id.assign(it->value.GetString(), it->value.GetStringLength());
    } else if (strcmp(key, "t") == 0) {
      if (!it->value.IsInt()) {
        return nullptr;
      }
      type_value = it->value.GetInt();
    } else if (strcmp(key, "s") == 0) {
      start_point = ValueParser::ParsePoint(it->value, 1.f);
      if (!start_point) {
        return nullptr;
      }
    } else if (strcmp(key, "e") == 0) {
      end_point = ValueParser::ParsePoint(it->value, 1.f);
    } else if (strcmp(key, "rxy") == 0) {
      radius_xy = ValueParser::ParsePoint(it->value, 1.f);
    } else if (strcmp(key, "a") == 0) {
      if (!it->value.IsNumber()) {
        return nullptr;
      }
      start_angle = it->value.GetFloat();
    } else if (strcmp(key, "g") == 0) {
      gradient_value = &it->value;
    }
  }

  if (gradient_value == nullptr || !gradient_value->IsObject()) {
    return nullptr;
  }

  int32_t color_points = -1;
  rapidjson::Value* gradient_values = nullptr;
  const auto& gradient_object = gradient_value->GetObject();
  for (auto gradient_it = gradient_object.MemberBegin();
       gradient_it != gradient_object.MemberEnd(); gradient_it++) {
    const auto& gradient_key = gradient_it->name.GetString();
    if (strcmp(gradient_key, "p") == 0) {
      if (!gradient_it->value.IsInt()) {
        return nullptr;
      }
      color_points = gradient_it->value.GetInt();
    } else if (strcmp(gradient_key, "k") == 0) {
      gradient_values = &gradient_it->value;
    }
  }

  if (color_points < 0 || gradient_values == nullptr) {
    return nullptr;
  }

  ParseContext gradient_context{context.enable_audio_, color_points};
  gradient_color =
      ValueParser::ParseGradient(*gradient_values, gradient_context);
  if (!gradient_color) {
    return nullptr;
  }

  if (layer_id < 0 || !start_point || !gradient_color ||
      gradient_color->IsEmpty()) {
    return nullptr;
  }

  GradientType gradient_type;
  switch (static_cast<TextGradientType>(type_value)) {
    case TextGradientType::kLinear:
      if (!end_point) {
        return nullptr;
      }
      gradient_type = GradientType::kLinear;
      break;
    case TextGradientType::kRadial:
      if (radius_xy) {
        if (radius_xy->GetX() <= 0 || radius_xy->GetY() <= 0) {
          return nullptr;
        }
      } else if (!end_point) {
        return nullptr;
      }
      gradient_type = GradientType::kRadial;
      break;
    case TextGradientType::kConic:
      if (!start_angle.has_value()) {
        return nullptr;
      }
      gradient_type = GradientType::kConic;
      break;
    default:
      return nullptr;
  }
  return std::make_unique<TextGradientItemModel>(
      gradient_type, std::move(start_point), std::move(end_point),
      std::move(radius_xy), std::move(gradient_color), std::move(start_angle));
}

}  // namespace

void TextGradientParser::Parse(rapidjson::Value& value,
                               CompositionModel& composition) {
  if (!value.IsArray()) {
    return;
  }

  const auto& array = value.GetArray();
  auto& context = composition.parse_context_;
  for (auto array_it = array.Begin(); array_it != array.End(); array_it++) {
    int32_t layer_id = -1;
    std::string ref_id;
    auto item = ParseItem(*array_it, layer_id, ref_id, context);
    if (!item) {
      continue;
    }

    auto* layer = composition.FindLayer(layer_id, ref_id);
    if (layer == nullptr || layer->GetLayerType() != LayerType::kText) {
      continue;
    }

    auto model = std::make_unique<TextGradientModel>();
    model->AddItem(std::move(item));
    layer->SetTextGradientModel(std::move(model));
  }
}

}  // namespace animax
}  // namespace lynx
