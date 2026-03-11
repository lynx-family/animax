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

#include "src/parser/keyframe/json_parser.h"

#include "src/base/log/log.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

void JsonParser::JsonToPoints(rapidjson::Value& value, float scale,
                              std::vector<std::unique_ptr<PointF>>& points) {
  const auto& array = value.GetArray();
  if ((array.Size() == 2 || array.Size() == 3) && array[0].IsNumber()) {
    auto point = JsonToPoint(value, scale);
    if (!point->IsEmpty()) {
      points.push_back(std::move(point));
    }
  } else {
    for (auto it = array.Begin(); it != array.End(); it++) {
      auto point = JsonToPoint(it->Move(), scale);
      if (!point->IsEmpty()) {
        points.push_back(std::move(point));
      }
    }
  }
}

std::unique_ptr<PointF> JsonParser::JsonToPoint(rapidjson::Value& value,
                                                float scale) {
  if (value.IsNumber()) {
    return JsonNumbersToPoint(value, scale);
  } else if (value.IsArray()) {
    return JsonArrayToPoint(value, scale);
  } else if (value.IsObject()) {
    return JsonObjectToPoint(value, scale);
  } else {
    ANIMAX_LOGI("unknown points");
  }
  return ValueFactory::Make<PointF>();
}

std::unique_ptr<PointF> JsonParser::JsonNumbersToPoint(rapidjson::Value& value,
                                                       float scale) {
  // TODO(aiyongbiao): tmp impl p1
  ANIMAX_LOGI("JsonNumbersToPoint should not be here, pls check!");
  auto v = value.GetFloat();
  return ValueFactory::Make<PointF>(v * scale, v * scale, v * scale);
}

std::unique_ptr<PointF> JsonParser::JsonArrayToPoint(rapidjson::Value& value,
                                                     float scale) {
  auto it = value.GetArray().Begin(), it_end = value.GetArray().End();
  float x = (it != it_end) ? it->GetFloat() : 0.f;
  it += 1;
  float y = (it != it_end) ? it->GetFloat() : 0.f;
  it += 1;
  float z = (it != it_end) ? it->GetFloat() : 0.f;

  return ValueFactory::Make<PointF>(x * scale, y * scale, z * scale);
}

std::unique_ptr<PointF> JsonParser::JsonObjectToPoint(rapidjson::Value& value,
                                                      float scale) {
  const auto& object = value.GetObject();
  float x = 0, y = 0, z = 0;
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "x") == 0) {
      x = ValueFromObject(it->value);
    } else if (strcmp(key, "y") == 0) {
      y = ValueFromObject(it->value);
    } else if (strcmp(key, "z") == 0) {
      z = ValueFromObject(it->value);
    }
  }
  return ValueFactory::Make<PointF>(x * scale, y * scale, z * scale);
}

float JsonParser::ValueFromObject(rapidjson::Value& value) {
  if (value.IsNumber()) {
    return value.GetFloat();
  } else if (value.IsArray()) {
    if (value.GetArray().Empty()) {
      ANIMAX_LOGI("Point array is empty");
      return 0;
    }
    return value.GetArray().Begin()->GetFloat();
  }
  return 0;
}

int32_t JsonParser::JsonToColor(rapidjson::Value& value) {
  const auto& array = value.GetArray();
  int32_t r = 255, g = 255, b = 255;
  for (auto i = 0; i < array.Size(); i++) {
    auto color_value = array[i].GetFloat() * 255;
    if (i == 0) {
      r = color_value;
    } else if (i == 1) {
      g = color_value;
    } else if (i == 2) {
      b = color_value;
    }
  }
  return Color::ToInt(255, r, g, b);
}

const std::unique_ptr<PointF> JsonParser::AddPoints(const PointF& point1,
                                                    const PointF& point2) {
  return ValueFactory::Make<PointF>(point1.GetX() + point2.GetX(),
                                    point1.GetY() + point2.GetY(),
                                    point1.GetZ() + point2.GetZ());
}

}  // namespace animax
}  // namespace lynx
