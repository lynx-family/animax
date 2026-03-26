// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PROPERTY_ANIMAX_VALUE_PARAM_H_
#define ANIMAX_INCLUDE_PROPERTY_ANIMAX_VALUE_PARAM_H_

#include <memory>
#include <optional>
#include <string>

#include "include/base/macros.h"
#include "src/model/value/base_value.h"
#include "src/property/property_type.h"

namespace lynx {
namespace animax {

/**
 * A value parameter class that can hold different types of values (String,
 * Number, Boolean, Point) for animation parameters. Supports optional target
 * frame for keyframe animations.
 */
class ANIMAX_EXPORT AnimaXValueParam {
 public:
  constexpr static int32_t kFrameIndexAll = -1;

  enum class Type {
    // String type, used for properties like kTextValue.
    kString,
    // Numeric type, used for a wide range of numerical properties.
    kNumber,
    // Boolean type, primarily used for layer visibility.
    kBoolean,
    // Coordinate type, supports both 2D and 3D, convertible to ScaleF and
    // PointF.
    kCoordinate,
    // Color type, convertible to a Color value for color-related properties.
    kColor,
    // ColorFilter type, defaults to PorterDuff, includes color and mode.
    kColorFilter,
    // Null type, represents an empty or invalid value.
    kNull
  };

  struct ColorFilterParam {
    int32_t color;
    int32_t mode;
    ColorFilterParam() : color(0), mode(0) {}

    /**
     * Constructor with color and mode values
     * @param color_value The color value (ARGB format)
     * @param mode_value The filter mode value
     */
    ColorFilterParam(int32_t color_value, int32_t mode_value)
        : color(color_value), mode(mode_value) {}
  };

  /**
   * Creates a AnimaXValueParam from a String value
   * @param value String value
   * @param target_frame Frame number at which this value should be applied
   */
  explicit AnimaXValueParam(const std::string& value,
                            int32_t target_frame = kFrameIndexAll);

  /**
   * Creates a AnimaXValueParam from a numeric value
   * @param value Numeric value
   * @param target_frame Frame number at which this value should be applied
   */
  explicit AnimaXValueParam(double value,
                            int32_t target_frame = kFrameIndexAll);

  /**
   * Creates a AnimaXValueParam from a boolean value
   * @param value Boolean value
   * @param target_frame Frame number at which this value should be applied
   */
  explicit AnimaXValueParam(bool value, int32_t target_frame = kFrameIndexAll);

  /**
   * Creates a AnimaXValueParam from a point (x,y coordinates)
   * @param x X coordinate
   * @param y Y coordinate
   * @param target_frame Frame number at which this value should be applied
   */
  explicit AnimaXValueParam(double x, double y,
                            int32_t target_frame = kFrameIndexAll);

  /**
   * Creates a AnimaXValueParam from a coordinate (x,y,z coordinates)
   * @param x X coordinate
   * @param y Y coordinate
   * @param z Z coordinate
   * @param target_frame Frame number at which this value should be applied
   */
  explicit AnimaXValueParam(double x, double y, double z,
                            int32_t target_frame = kFrameIndexAll);

  /**
   * Creates a AnimaXValueParam from a color value
   * @param color Color integer value
   * @param target_frame Frame number at which this value should be applied
   */
  explicit AnimaXValueParam(int32_t color,
                            int32_t target_frame = kFrameIndexAll);

  /**
   * Creates a AnimaXValueParam for a color filter with color and mode
   * @param filter_params Bundle containing color and mode values
   * @param target_frame Frame number at which this value should be applied
   */
  explicit AnimaXValueParam(const ColorFilterParam& filter_params,
                            int32_t target_frame = kFrameIndexAll);

  /**
   * Creates a null AnimaXValueParam
   */
  AnimaXValueParam();

  AnimaXValueParam(AnimaXValueParam&& other) noexcept = default;

  AnimaXValueParam& operator=(AnimaXValueParam&& other) noexcept;

  AnimaXValueParam(const AnimaXValueParam&) = delete;

  AnimaXValueParam& operator=(const AnimaXValueParam&) = delete;

  virtual ~AnimaXValueParam() = default;

  /**
   * Checks if this parameter has a target frame
   * @return true if target frame is specified
   */
  bool HasTargetFrame() const { return target_frame_ != kFrameIndexAll; }

  /**
   * Gets the target frame if specified
   * @return Target frame or -1 if not specified
   */
  int32_t GetTargetFrame() const { return target_frame_; }

  /**
   * Checks if the value is a String
   * @return true if value is a String
   */
  bool IsString() const { return type_ == Type::kString; }

  /**
   * Checks if the value is a Number
   * @return true if value is a Number
   */
  bool IsNumber() const { return type_ == Type::kNumber; }

  /**
   * Checks if the value is a Boolean
   * @return true if value is a Boolean
   */
  bool IsBoolean() const { return type_ == Type::kBoolean; }

  /**
   * Checks if the value is a Coordinate
   * @return true if value is a Coordinate
   */
  bool IsCoordinate() const { return type_ == Type::kCoordinate; }

  /**
   * Checks if the value is a Color
   * @return true if value is a Color
   */
  bool IsColor() const { return type_ == Type::kColor; }

  /**
   * Checks if the value is a ColorFilter
   * @return true if value is a ColorFilter
   */
  bool IsColorFilter() const { return type_ == Type::kColorFilter; }

  /**
   * Checks if the value is null
   * @return true if value is null
   */
  bool IsNull() const { return type_ == Type::kNull; }

  /**
   * Gets the value as a String
   * @return String value or empty string if not a String
   */
  const std::string& GetStringValue() const;

  /**
   * Gets the value as a Number
   * @return Number value or 0 if not a Number
   */
  double GetNumberValue() const;

  /**
   * Gets the value as a Boolean
   * @return Boolean value or false if not a Boolean
   */
  bool GetBooleanValue() const;

  /**
   * Gets the X coordinate if this is a Coordinate value
   * @return X coordinate or 0 if not a Coordinate
   */
  double GetX() const;

  /**
   * Gets the Y coordinate if this is a Coordinate value
   * @return Y coordinate or 0 if not a Coordinate
   */
  double GetY() const;

  /**
   * Gets the Z coordinate if this is a Coordinate value
   * @return Z coordinate or 0 if not a Coordinate
   */
  double GetZ() const;

  /**
   * Gets the color value
   * @return Color int value or 0 if not a Color or ColorFilter
   */
  int32_t GetColorValue() const;

  /**
   * Gets the value pointer based on the type.
   * @return Pointer to the value object, or nullptr if type is kNull
   */
  void* GetValuePtr() const;

  /**
   * Converts a Value to AnimaXValueParam based on value type
   * @param value The source Value to convert
   * @return A new AnimaXValueParam containing the converted value
   */
  static std::unique_ptr<AnimaXValueParam> FromValue(const Value* value);

  /**
   * Converts this AnimaXValueParam to a Value of the appropriate type based on
   * property type
   * @param property_type The property type to determine which Value subclass to
   * create
   * @return A new Value object containing the converted value
   */
  std::unique_ptr<Value> ToValue(LayerPropertyType property_type) const;

  /**
   * Gets the type of the value held by this parameter.
   * @return The Type enum value representing the underlying type.
   */
  Type GetType() const;

  /**
   * Copies the value to target variable if types are compatible
   * @param target Pointer to the target variable
   * @return true if copy succeeded, false otherwise
   */
  bool CopyTo(std::string* target) const;
  bool CopyTo(float* target) const;
  bool CopyTo(int32_t* target) const;
  bool CopyTo(double* target) const;
  bool CopyTo(bool* target) const;

 private:
  Type type_;
  std::optional<std::string> string_value_;
  std::optional<double> number_value_;
  std::optional<PointF> coordinate_value_;
  std::optional<Color> color_value_;
  int32_t target_frame_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PROPERTY_ANIMAX_VALUE_PARAM_H_
