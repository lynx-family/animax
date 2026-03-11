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

#ifndef ANIMAX_SRC_PARSER_VALUE_PARSER_H_
#define ANIMAX_SRC_PARSER_VALUE_PARSER_H_

#include <memory>
#include <vector>

#include "base/include/no_destructor.h"
#include "src/model/value/base_value.h"
#include "third_party/rapidjson/document.h"

namespace lynx {
namespace animax {

struct ParseContext;
class GradientColor;

class ValueParser final {
 public:
  static std::unique_ptr<Value> Parse(rapidjson::Value& value, float scale,
                                      ParseContext& context, ValueType type);

 private:
  static std::unique_ptr<Value> ParseColor(rapidjson::Value& value,
                                           float scale);
  static std::unique_ptr<Value> ParseDocument(rapidjson::Value& value,
                                              float scale);
  static std::unique_ptr<Value> ParseShape(rapidjson::Value& value,
                                           float scale);
  static std::unique_ptr<Value> ParseGradient(rapidjson::Value& value,
                                              ParseContext& context);
  static std::unique_ptr<Value> ParseOrientation(rapidjson::Value& value,
                                                 ValueType type);

  static void AddOpacityStopsToGradientIfNeeded(GradientColor& model,
                                                std::vector<float>& array);

  static int32_t GetColorInBetweenColorStops(float position, float opacity,
                                             float* positions, int32_t* colors,
                                             int32_t length);

  static int32_t GetColorInBetweenOpacityStops(float position, int32_t color,
                                               float* positions,
                                               float* opacities,
                                               int32_t length);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PARSER_VALUE_PARSER_H_
