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

#include "src/parser/keyframe/keyframe_parser.h"

#include "src/parser/value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<KeyframeModel> KeyframeParser::Parse(
    rapidjson::Value& value, CompositionModel& composition, float scale,
    bool animated, bool multi_dimen, ValueType type) {
  if (animated && multi_dimen) {
    return ParseMultiDimensionalKeyframe(value, composition, scale, type);
  } else if (animated) {
    return ParseKeyframe(value, composition, scale, type);
  } else {
    return ParseStaticValue(value, composition, scale, type);
  }
}

std::unique_ptr<KeyframeModel> KeyframeParser::ParseMultiDimensionalKeyframe(
    rapidjson::Value& value, CompositionModel& composition, float scale,
    ValueType type) {
  auto cp1 = ValueFactory::Make<PointF>();
  auto cp2 = ValueFactory::Make<PointF>();

  auto x_cp1 = PointF();
  auto x_cp2 = PointF();
  auto y_cp1 = PointF();
  auto y_cp2 = PointF();

  float start_frame = 0;
  std::unique_ptr<Value> start_value;
  std::unique_ptr<Value> end_value;
  bool hold = false;

  std::unique_ptr<Interpolator> interpolator;
  std::unique_ptr<Interpolator> x_interpolator;
  std::unique_ptr<Interpolator> y_interpolator;

  auto path_cp1 = ValueFactory::Make<PointF>();
  auto path_cp2 = ValueFactory::Make<PointF>();

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "t") == 0) {
      start_frame = it->value.GetFloat();
    } else if (strcmp(key, "s") == 0) {
      end_value = ValueParser::Parse(it->value, scale,
                                     composition.parse_context_, type);
    } else if (strcmp(key, "e") == 0) {
      end_value = ValueParser::Parse(it->value, scale,
                                     composition.parse_context_, type);
    } else if (strcmp(key, "o") == 0) {
      if (it->value.IsObject()) {
        ParseControlPoints(it->value, x_cp1, y_cp1);
      } else {
        cp1 = JsonParser::JsonToPoint(it->value, scale);
      }
    } else if (strcmp(key, "i") == 0) {
      if (it->value.IsObject()) {
        ParseControlPoints(it->value, y_cp1, y_cp2);
      } else {
        cp2 = JsonParser::JsonToPoint(it->value, scale);
      }
    } else if (strcmp(key, "h") == 0) {
      hold = it->value.GetInt() == 1;
    } else if (strcmp(key, "to") == 0) {
      path_cp1 = JsonParser::JsonToPoint(it->value, scale);
    } else if (strcmp(key, "ti") == 0) {
      path_cp2 = JsonParser::JsonToPoint(it->value, scale);
    }
  }

  if (hold) {
    end_value = start_value->Copy();
    interpolator = LinearInterpolator::Make();
  } else if (!cp1->IsEmpty() && !cp2->IsEmpty()) {
    interpolator = PathInterpolator::Make(*cp1, *cp2);
  } else if (!x_cp1.IsEmpty() && !y_cp1.IsEmpty() && !x_cp2.IsEmpty() &&
             !y_cp2.IsEmpty()) {
    x_interpolator = PathInterpolator::Make(x_cp1, x_cp2);
    y_interpolator = PathInterpolator::Make(y_cp1, y_cp2);
  } else {
    interpolator = LinearInterpolator::Make();
  }

  std::unique_ptr<KeyframeModel> keyframe;
  if (x_interpolator && y_interpolator) {
    keyframe = std::make_unique<KeyframeModel>(
        composition, std::move(start_value), std::move(end_value),
        std::move(x_interpolator), std::move(y_interpolator), start_frame,
        Float::kMin);
  } else {
    keyframe = std::make_unique<KeyframeModel>(
        composition, std::move(start_value), std::move(end_value),
        std::move(interpolator), start_frame, Float::kMin);
  }

  keyframe->SetPathCps(std::move(path_cp1), std::move(path_cp2));
  return keyframe;
}

void KeyframeParser::ParseControlPoints(rapidjson::Value& value, PointF& x_cp,
                                        PointF& y_cp) {
  float x_cp_x = 0, x_cp_y = 0, y_cp_x = 0, y_cp_y = 0;
  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& name = it->name.GetString();
    if (strcmp(name, "x") == 0) {
      if (it->value.IsNumber()) {
        x_cp_x = it->value.GetFloat();
        y_cp_x = x_cp_x;
      } else {
        const auto& array = it->value.GetArray();
        for (size_t i = 0; i < array.Size(); i++) {
          auto point = (array.Begin() + i)->GetFloat();
          if (i == 0) {
            x_cp_x = point;
          } else if (i == 1) {
            y_cp_x = point;
          }
        }
        if (array.Size() <= 1) {
          y_cp_x = x_cp_x;
        }
      }
    } else if (strcmp(name, "y") == 0) {
      if (it->value.IsNumber()) {
        x_cp_y = it->value.GetFloat();
        y_cp_y = x_cp_y;
      } else {
        const auto& array = it->value.GetArray();
        for (size_t i = 0; i < array.Size(); i++) {
          auto point = (array.Begin() + i)->GetFloat();
          if (i == 0) {
            x_cp_y = point;
          } else if (i == 1) {
            y_cp_y = point;
          }
        }
        if (array.Size() <= 1) {
          y_cp_y = x_cp_y;
        }
      }
    }
  }
  x_cp.Set(x_cp_x, x_cp_y, 0);
  y_cp.Set(y_cp_x, y_cp_y, 0);
}

std::unique_ptr<KeyframeModel> KeyframeParser::ParseKeyframe(
    rapidjson::Value& value, CompositionModel& composition, float scale,
    ValueType type) {
  auto cp1 = ValueFactory::Make<PointF>();
  auto cp2 = ValueFactory::Make<PointF>();
  float start_frame = 0;
  std::unique_ptr<Value> start_value;
  std::unique_ptr<Value> end_value;
  bool hold = false;
  std::unique_ptr<Interpolator> interpolator;

  auto path_cp1 = ValueFactory::Make<PointF>();
  auto path_cp2 = ValueFactory::Make<PointF>();

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "t") == 0) {
      start_frame = it->value.GetFloat();
    } else if (strcmp(key, "s") == 0) {
      start_value = ValueParser::Parse(it->value, scale,
                                       composition.parse_context_, type);
    } else if (strcmp(key, "e") == 0) {
      end_value = ValueParser::Parse(it->value, scale,
                                     composition.parse_context_, type);
    } else if (strcmp(key, "o") == 0) {
      cp1 = JsonParser::JsonToPoint(it->value, 1);
    } else if (strcmp(key, "i") == 0) {
      cp2 = JsonParser::JsonToPoint(it->value, 1);
    } else if (strcmp(key, "h") == 0) {
      hold = it->value.GetInt() == 1;
    } else if (strcmp(key, "to") == 0) {
      path_cp1 = JsonParser::JsonToPoint(it->value, scale);
    } else if (strcmp(key, "ti") == 0) {
      path_cp2 = JsonParser::JsonToPoint(it->value, scale);
    }
  }

  if (hold) {
    end_value = start_value->Copy();
    interpolator =
        std::unique_ptr<LinearInterpolator>(new LinearInterpolator());
  } else if (!cp1->IsEmpty() && !cp2->IsEmpty()) {
    interpolator = PathInterpolator::Make(*cp1, *cp2);
  } else {
    interpolator =
        std::unique_ptr<LinearInterpolator>(new LinearInterpolator());
  }

  auto keyframe = std::make_unique<KeyframeModel>(
      composition, std::move(start_value), std::move(end_value),
      std::move(interpolator), start_frame, Float::kMin);
  keyframe->SetPathCps(std::move(path_cp1), std::move(path_cp2));

  return keyframe;
}

std::unique_ptr<KeyframeModel> KeyframeParser::ParseStaticValue(
    rapidjson::Value& value, CompositionModel& composition, float scale,
    ValueType type) {
  return std::make_unique<KeyframeModel>(
      composition,
      ValueParser::Parse(value, scale, composition.parse_context_, type));
}

}  // namespace animax
}  // namespace lynx
