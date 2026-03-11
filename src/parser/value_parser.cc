// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
//
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

#include "src/parser/value_parser.h"

#include <algorithm>
#include <cmath>

#include "src/base/log/log.h"
#include "src/base/util/misc_util.h"
#include "src/model/composition_model.h"
#include "src/model/value/document_data.h"
#include "src/model/value/gradient_color.h"
#include "src/model/value/shape_data.h"
#include "src/parser/keyframe/json_parser.h"

namespace lynx {
namespace animax {
namespace {
template <typename T>
struct GradientItem {
  float position;
  T value;
};

float Lerp(const float& a, const float& b, float percentage) {
  return a + percentage * (b - a);
}

Color Lerp(const Color& a, const Color& b, float percentage) {
  Color out;
  GammaEvaluate(a, b, percentage, &out);
  return out;
}

template <typename T>
T Interpolate(const std::vector<GradientItem<T>>& array,
              typename std::vector<GradientItem<T>>::const_iterator it,
              float position, float epsilon) {
  DCHECK(!array.empty());
  if (it == array.begin()) {
    return it->value;
  }
  auto it_prev = it - 1;
  if (it == array.end()) {
    return it_prev->value;
  }
  if (std::abs(it->position - it_prev->position) <= epsilon) {
    return it_prev->value;
  }
  float percentage = std::clamp(
      (position - it_prev->position) / (it->position - it_prev->position), 0.f,
      1.f);
  return Lerp(it_prev->value, it->value, percentage);
}

std::vector<GradientItem<Color>> MergeArrays(
    std::vector<GradientItem<Color>>& color_array,
    std::vector<GradientItem<float>>& opacity_array, float epsilon) {
  DCHECK(!color_array.empty());
  if (opacity_array.empty()) {
    return color_array;
  }

  std::vector<GradientItem<Color>> result_array;
  result_array.reserve(color_array.size() + opacity_array.size());
  auto it_color = color_array.begin();
  auto it_opacity = opacity_array.begin();
  while (it_color != color_array.end() && it_opacity != opacity_array.end()) {
    const float position_difference = it_color->position - it_opacity->position;
    float position = it_color->position, opacity = it_opacity->value;
    Color color = it_color->value;
    if (std::fabs(position_difference) <= epsilon) {
      ++it_color;
      ++it_opacity;
    } else if (position_difference < -epsilon) {
      opacity =
          Interpolate<float>(opacity_array, it_opacity, position, epsilon);
      ++it_color;
    } else {
      position = it_opacity->position;
      color = Interpolate<Color>(color_array, it_color, position, epsilon);
      ++it_opacity;
    }
    color.SetA(opacity * 255.f);
    result_array.push_back(
        {.position = std::move(position), .value = std::move(color)});
  }
  for (; it_opacity != opacity_array.end(); ++it_opacity) {
    Color color = Interpolate<Color>(color_array, it_color,
                                     it_opacity->position, epsilon);
    color.SetA(it_opacity->value * 255.f);
    result_array.push_back(
        {.position = it_opacity->position, .value = std::move(color)});
  }
  for (; it_color != color_array.end(); ++it_color) {
    Color color = it_color->value;
    float opacity = Interpolate<float>(opacity_array, it_opacity,
                                       it_color->position, epsilon);
    color.SetA(opacity * 255.f);
    result_array.push_back(
        {.position = it_color->position, .value = std::move(color)});
  }
  return result_array;
}

}  // namespace
std::unique_ptr<Value> ValueParser::Parse(rapidjson::Value& value, float scale,
                                          ParseContext& context,
                                          ValueType type) {
  switch (type) {
    case ValueType::kFloat: {
      auto round = JsonParser::ValueFromObject(value) * scale;
      return ValueFactory::Make<Float>(round);
    }
    case ValueType::kInteger: {
      auto round = std::round(JsonParser::ValueFromObject(value) * scale);
      return ValueFactory::Make<Integer>(static_cast<int32_t>(round));
    }
    case ValueType::kPoint: {
      if (value.IsArray() || value.IsObject()) {
        return JsonParser::JsonToPoint(value, scale);
      } else if (value.IsNumber()) {
        // TODO(aiyongbiao): tmp impl p1
        ANIMAX_LOGI("warn, should not go to there basicvalueparser");
        auto v = value.GetFloat();
        return ValueFactory::Make<PointF>(v * scale, v * scale, v * scale);
      }
      return ValueFactory::Make<PointF>();
    }
    case ValueType::kScale: {
      auto it = value.GetArray().Begin(), it_end = value.GetArray().End();
      auto x = (it != it_end) ? it->GetFloat() : 1.f;
      it += 1;
      auto y = (it != it_end) ? it->GetFloat() : 1.f;
      it += 1;
      auto z = (it != it_end) ? it->GetFloat() : 1.f;
      return ValueFactory::Make<ScaleF>(x / 100 * scale, y / 100 * scale,
                                        z / 100 * scale);
    }
    case ValueType::kColor: {
      return ValueParser::ParseColor(value, scale);
    }
    case ValueType::kPath: {
      return JsonParser::JsonToPoint(value, scale);
    }
    case ValueType::kDocument: {
      return ValueParser::ParseDocument(value, scale);
    }
    case ValueType::kShape: {
      return ValueParser::ParseShape(value, scale);
    }
    case ValueType::kGradient: {
      return ValueParser::ParseGradient(value, context);
    }
    case ValueType::kOrientation:
    case ValueType::kCameraOrientation: {
      return ValueParser::ParseOrientation(value, type);
    }
    default: {
    }
  }
  return nullptr;
}

std::unique_ptr<Value> ValueParser::ParseOrientation(rapidjson::Value& value,
                                                     ValueType type) {
  if (!value.IsArray()) {
    return nullptr;
  }
  const auto& array = value.GetArray();
  size_t array_size = array.Size();
  if (array_size != 3) {
    return nullptr;
  }
  auto it = value.GetArray().Begin();
  auto alpha = it->GetFloat();
  it += 1;
  auto beta = it->GetFloat();
  it += 1;
  auto gamma = it->GetFloat();
  if (type == ValueType::kCameraOrientation) {
    // for the camera, the rotation direction around the z-axis is opposite
    gamma = -gamma;
  }
  return std::make_unique<Orientation>(alpha, beta, gamma);
}

std::unique_ptr<Value> ValueParser::ParseColor(rapidjson::Value& value,
                                               float scale) {
  const auto& array = value.GetArray();
  auto it = array.Begin();
  float r, g, b, a = 1;
  for (auto index = 0; index < array.Size(); index++) {
    if (index == 0) {
      r = (it + index)->GetFloat();
    } else if (index == 1) {
      g = (it + index)->GetFloat();
    } else if (index == 2) {
      b = (it + index)->GetFloat();
    } else if (index == 3) {
      a = (it + index)->GetFloat();
    }
  }

  if (r <= 1 && g <= 1 && b <= 1) {
    r *= 255;
    g *= 255;
    b *= 255;

    if (a <= 1) {
      a *= 255;
    }
  }

  return ValueFactory::Make<Color>(
      static_cast<uint8_t>(a), static_cast<uint8_t>(r), static_cast<uint8_t>(g),
      static_cast<uint8_t>(b));
}

std::unique_ptr<Value> ValueParser::ParseDocument(rapidjson::Value& value,
                                                  float scale) {
  std::string text;
  std::string font_name;
  float size = 0;
  DocumentJustification justification = DocumentJustification::kLeftAlign;
  int32_t tracking = 0;
  float line_height = 0;
  float baseline_shift = 0;
  int32_t fill_color = 0;
  int32_t stroke_color = 0;
  float stroke_width = 0;
  bool stroke_overfill = true;
  auto box_position = ValueFactory::Make<PointF>();
  auto box_size = ValueFactory::Make<PointF>();

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "t") == 0) {
      text = it->value.GetString();
    } else if (strcmp(key, "f") == 0) {
      font_name = it->value.GetString();
    } else if (strcmp(key, "s") == 0) {
      size = it->value.GetFloat() * scale;
    } else if (strcmp(key, "j") == 0) {
      auto justification_int = it->value.GetInt();
      if (justification_int >
              static_cast<int32_t>(
                  DocumentJustification::kJustifyWithLastLineFull) ||
          justification_int < 0) {
        justification = DocumentJustification::kLeftAlign;
      } else {
        justification = static_cast<DocumentJustification>(justification_int);
      }
    } else if (strcmp(key, "tr") == 0) {
      tracking = it->value.GetInt();
    } else if (strcmp(key, "lh") == 0) {
      line_height = it->value.GetFloat() * scale;
    } else if (strcmp(key, "ls") == 0) {
      baseline_shift = it->value.GetFloat() * scale;
    } else if (strcmp(key, "fc") == 0) {
      fill_color = JsonParser::JsonToColor(it->value);
    } else if (strcmp(key, "sc") == 0) {
      stroke_color = JsonParser::JsonToColor(it->value);
    } else if (strcmp(key, "sw") == 0) {
      stroke_width = it->value.GetFloat() * scale;
    } else if (strcmp(key, "of") == 0) {
      stroke_overfill = it->value.GetBool();
    } else if (strcmp(key, "ps") == 0) {
      box_position = JsonParser::JsonToPoint(it->value, scale);
    } else if (strcmp(key, "sz") == 0) {
      box_size = JsonParser::JsonToPoint(it->value, scale);
    }
  }
  return std::make_unique<DocumentData>(
      std::move(text), std::move(font_name), size, justification, tracking,
      line_height, baseline_shift, fill_color, stroke_color, stroke_width,
      stroke_overfill, std::move(box_position), std::move(box_size));
}

std::unique_ptr<Value> ValueParser::ParseShape(rapidjson::Value& value,
                                               float scale) {
  rapidjson::Value object;
  if (value.IsArray()) {
    object = value.GetArray().Begin()->GetObject();
  } else {
    object = value.GetObject();
  }

  bool closed;
  std::vector<std::unique_ptr<PointF>> points_array;
  std::vector<std::unique_ptr<PointF>> in_tangents;
  std::vector<std::unique_ptr<PointF>> out_tangents;
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    auto key = it->name.GetString();
    if (strcmp(key, "c") == 0) {
      closed = it->value.GetBool();
    } else if (strcmp(key, "v") == 0) {
      JsonParser::JsonToPoints(it->value, scale, points_array);
    } else if (strcmp(key, "i") == 0) {
      JsonParser::JsonToPoints(it->value, scale, in_tangents);
    } else if (strcmp(key, "o") == 0) {
      JsonParser::JsonToPoints(it->value, scale, out_tangents);
    }
  }

  auto shape_model = ValueFactory::Make<ShapeData>();
  if (points_array.empty()) {
    return shape_model;
  }

  auto& curves = shape_model->GetCurves();
  int32_t length = points_array.size();
  auto& initial_point = *points_array[0];
  for (auto i = 1; i < length; i++) {
    auto& vertex = *points_array[i];
    auto& previous_vertex = *points_array[i - 1];
    auto& cp1 = *out_tangents[i - 1];
    auto& cp2 = *in_tangents[i];
    curves.emplace_back(
        CubicCurveModel(*JsonParser::AddPoints(previous_vertex, cp1),
                        *JsonParser::AddPoints(vertex, cp2), vertex));
  }

  if (closed) {
    auto& vertex = *points_array[0];
    auto& previous_vertex = *points_array[length - 1];
    auto& cp1 = *out_tangents[length - 1];
    auto& cp2 = *in_tangents[0];
    curves.emplace_back(
        CubicCurveModel(*JsonParser::AddPoints(previous_vertex, cp1),
                        *JsonParser::AddPoints(vertex, cp2), vertex));
  }

  shape_model->Init(initial_point, closed);
  return shape_model;
}

std::unique_ptr<Value> ValueParser::ParseGradient(rapidjson::Value& value,
                                                  ParseContext& context) {
  if (!value.IsArray()) {
    return nullptr;
  }

  const int32_t value_array_size = value.GetArray().Size();
  auto color_points = (context.color_points_ >= 0 ? context.color_points_
                                                  : value_array_size / 4);
  if (color_points <= 0 || value_array_size < color_points * 4) {
    return nullptr;
  }

  std::vector<GradientItem<Color>> color_array;
  std::vector<GradientItem<float>> opacity_array;
  auto it = value.GetArray().Begin();
  constexpr float epsilon = 1e-3;
  float last_position = 0;
  color_array.reserve(color_points);
  for (int32_t i = 0; i < color_points; ++i) {
    GradientItem<Color> item;
    item.position = std::clamp((it++)->GetFloat(), last_position, 1.f);
    last_position = item.position;
    float r = std::clamp((it++)->GetFloat(), 0.f, 1.f) * 255.f;
    float g = std::clamp((it++)->GetFloat(), 0.f, 1.f) * 255.f;
    float b = std::clamp((it++)->GetFloat(), 0.f, 1.f) * 255.f;
    item.value = Color(255, r, g, b);
    color_array.emplace_back(std::move(item));
  }
  last_position = 0;
  const int32_t opacity_points =
      (value.GetArray().Size() - color_points * 4) / 2;
  opacity_array.reserve(opacity_points);
  for (int32_t i = 0; i < opacity_points; ++i) {
    GradientItem<float> item;
    item.position = std::clamp((it++)->GetFloat(), last_position, 1.f);
    last_position = item.position;
    item.value = std::clamp((it++)->GetFloat(), 0.f, 1.f);
    opacity_array.emplace_back(std::move(item));
  }

  // auto add color position 0 and 1 if not exist
  auto it_color = color_array.begin();
  if (color_array.size() == 1) {
    it_color->position = 0.f;
    it_color = color_array.insert(color_array.end(), *it_color);
    it_color->position = 1.f;
  } else {
    if (it_color->position > epsilon) {
      it_color = color_array.insert(color_array.begin(), *it_color);
      it_color->position = 0.f;
    }
    it_color = color_array.end() - 1;
    if (it_color->position < 1.f - epsilon) {
      it_color = color_array.insert(color_array.end(), *it_color);
      it_color->position = 1.f;
    }
  }

  std::vector<GradientItem<Color>> merged_array =
      MergeArrays(color_array, opacity_array, epsilon);
  auto merged_size = merged_array.size();
  auto positions = std::make_unique<float[]>(merged_size);
  auto colors = std::make_unique<int32_t[]>(merged_size);
  for (size_t j = 0; j < merged_size; ++j) {
    auto& item = merged_array[j];
    positions[j] = item.position;
    colors[j] = item.value.GetInt();
  }
  return ValueFactory::Make<GradientColor>(std::move(positions),
                                           std::move(colors),
                                           static_cast<int32_t>(merged_size));
}

}  // namespace animax
}  // namespace lynx
