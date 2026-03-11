// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_COLOR_FILTER_H_
#define ANIMAX_SRC_RENDER_COLOR_FILTER_H_

#include <optional>

#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

/**
 * Color filter type for the animation system.
 */
class ColorFilter : public Value {
 public:
  /**
   * Constructs an empty ColorFilter.
   */
  ColorFilter() : color_(std::nullopt), mode_(std::nullopt) {}

  /**
   * Constructs a ColorFilter with specified color and blend mode.
   * @param color The color value in ARGB format
   * @param mode The blend mode to apply
   */
  explicit ColorFilter(int32_t color, int32_t mode)
      : color_(color), mode_(mode) {}

  ~ColorFilter() override = default;

  /**
   * Returns the type of the value.
   * @return The ValueType of this color filter (kColorFilter)
   */
  ValueType GetValueType() const override { return ValueType::kColorFilter; }

  /**
   * Checks if the color filter is in an empty state.
   * @return true if color or mode are not set, false otherwise
   */
  bool IsEmpty() const override {
    return !color_.has_value() || !mode_.has_value();
  }

  /**
   * Creates a deep copy of the color filter.
   * @return A unique pointer to the copied ColorFilter instance
   */
  std::unique_ptr<Value> Copy() const override {
    if (IsEmpty()) {
      return ValueFactory::Make<ColorFilter>();
    } else {
      return ValueFactory::Make<ColorFilter>(GetColor(), GetMode());
    }
  }

  /**
   * Gets the stored color value.
   * @return The color value in ARGB format, or -1 if not set
   */
  int32_t GetColor() const { return color_.value_or(-1); }

  /**
   * Gets the stored blend mode.
   * @return The blend mode value, or -1 if not set
   */
  int32_t GetMode() const { return mode_.value_or(-1); }

  /**
   * Sets new color and blend mode values.
   * @param color The color value in ARGB format
   * @param mode The blend mode to apply
   */
  void Set(int32_t color, int32_t mode) {
    color_ = color;
    mode_ = mode;
  }

 protected:
  std::optional<int32_t> color_;
  std::optional<int32_t> mode_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_COLOR_FILTER_H_
