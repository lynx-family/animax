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

#ifndef ANIMAX_SRC_MODEL_VALUE_GRADIENT_COLOR_H_
#define ANIMAX_SRC_MODEL_VALUE_GRADIENT_COLOR_H_

#include <memory>

#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

enum class GradientType : uint8_t { kLinear = 0, kRadial };

/**
 * Represents a gradient color, defined by a set of colors and their positions.
 */
class GradientColor : public Value {
 public:
  /**
   * Constructs an empty GradientColor.
   */
  GradientColor();
  /**
   * Constructs a GradientColor with a specific size, initializing colors and
   * positions.
   * @param size The number of color stops in the gradient.
   */
  explicit GradientColor(int32_t size);
  /**
   * Constructs a GradientColor with predefined positions and colors.
   * @param positions An array of color stop positions (0.0 to 1.0).
   * @param colors An array of color values (ARGB format).
   * @param size The number of color stops.
   */
  explicit GradientColor(std::unique_ptr<float[]> positions,
                         std::unique_ptr<int32_t[]> colors, int32_t size);

  ~GradientColor() override = default;

  ValueType GetValueType() const override { return ValueType::kGradient; }

  /**
   * Checks if the gradient color is empty.
   * @return true if the size is 0, false otherwise.
   */
  bool IsEmpty() const override;
  /**
   * Creates a deep copy of the GradientColor.
   * @return A unique pointer to the copied GradientColor instance.
   */
  std::unique_ptr<Value> Copy() const override;

  /**
   * Initializes the gradient with a given size.
   * @param size The number of color stops.
   */
  void Init(int32_t size);
  /**
   * Linearly interpolates between two GradientColor objects.
   * @param gc1 The starting gradient.
   * @param gc2 The ending gradient.
   * @param progress The interpolation progress (0.0 to 1.0).
   */
  void LerpColor(GradientColor& gc1, GradientColor& gc2, float progress);
  /**
   * Updates the colors of the gradient.
   * @param colors A new array of color values.
   */
  void UpdateColors(std::unique_ptr<int32_t[]> colors);

  /**
   * Gets the array of color stop positions.
   * @return A raw pointer to the positions array.
   */
  float* GetPositions() const { return positions_.get(); }
  /**
   * Gets the array of color values.
   * @return A raw pointer to the colors array.
   */
  int32_t* GetColors() const { return colors_.get(); }
  /**
   * Gets the number of color stops in the gradient.
   * @return The size of the gradient.
   */
  int32_t GetSize() const { return size_; }

 private:
  int32_t size_;
  std::unique_ptr<float[]> positions_;
  std::unique_ptr<int32_t[]> colors_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_VALUE_GRADIENT_COLOR_H_
