// Copyright 2023 The Lynx Authors. All rights reserved.
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

#include "src/parser/content_model_parser.h"

#include "src/base/log/log.h"
#include "src/parser/animatable/animatable_transform_parser.h"
#include "src/parser/gradient/gradient_fill_parser.h"
#include "src/parser/gradient/gradient_stroke_parser.h"
#include "src/parser/path/merge_paths_parser.h"
#include "src/parser/path/shape_trim_path_parser.h"
#include "src/parser/shape/circle_shape_parser.h"
#include "src/parser/shape/polystar_shape_parser.h"
#include "src/parser/shape/rectangle_shape_parser.h"
#include "src/parser/shape/repeater_parser.h"
#include "src/parser/shape/rounded_corners_parser.h"
#include "src/parser/shape/shape_fill_parser.h"
#include "src/parser/shape/shape_group_parser.h"
#include "src/parser/shape/shape_path_parser.h"
#include "src/parser/shape/shape_stroke_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<ContentModel> ContentModelParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  std::string type;
  int32_t d = 2;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "ty") == 0) {
      type = it->value.GetString();
    } else if (strcmp(key, "d") == 0) {
      if (it->value.IsNumber()) {
        d = it->value.GetInt();
      }
    }
  }

  if (type.empty()) {
    ANIMAX_LOGI("type is null");
    return nullptr;
  }

  char* type_name = type.data();
  if (strcmp(type_name, "gr") == 0) {
    return ShapeGroupParser::Parse(value, composition);
  } else if (strcmp(type_name, "st") == 0) {
    return ShapeStrokeParser::Parse(value, composition);
  } else if (strcmp(type_name, "gs") == 0) {
    return GradientStrokeParser::Parse(value, composition);
  } else if (strcmp(type_name, "fl") == 0) {
    return ShapeFillParser::Parse(value, composition);
  } else if (strcmp(type_name, "gf") == 0) {
    return GradientFillParser::Parse(value, composition);
  } else if (strcmp(type_name, "tr") == 0) {
    return AnimatableTransformParser::Parse(value, composition);
  } else if (strcmp(type_name, "sh") == 0) {
    return ShapePathParser::Parse(value, composition);
  } else if (strcmp(type_name, "el") == 0) {
    return CircleShapeParser::Parse(value, composition, d);
  } else if (strcmp(type_name, "rc") == 0) {
    return RectangleShapeParser::Parse(value, composition);
  } else if (strcmp(type_name, "tm") == 0) {
    return ShapeTrimPathParser::Parse(value, composition);
  } else if (strcmp(type_name, "sr") == 0) {
    return PolystarShapeParser::Parse(value, composition, d);
  } else if (strcmp(type_name, "mm") == 0) {
    return MergePathsParser::Parse(value);
  } else if (strcmp(type_name, "rp") == 0) {
    return RepeaterParser::Parse(value, composition);
  } else if (strcmp(type_name, "rd") == 0) {
    return RoundedCornersParser::Parse(value, composition);
  } else {
    ANIMAX_LOGI("no valid type:" << type);
  }

  return nullptr;
}

}  // namespace animax
}  // namespace lynx
