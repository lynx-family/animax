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

#ifndef ANIMAX_SRC_MODEL_VALUE_BASE_VALUE_H_
#define ANIMAX_SRC_MODEL_VALUE_BASE_VALUE_H_

#include <cmath>
#include <limits>
#include <memory>
#include <optional>

#include "include/base/macros.h"
namespace lynx {
namespace animax {

enum class ValueType : uint8_t {
  kUnknown = 0,
  kPoint,
  kFloat,
  kScale,
  kInteger,
  kColor,
  kDocument,
  kShape,
  kGradient,
  kPath,
  kSplitPath,
  kColorFilter,
  kOrientation,
  kCameraOrientation,
};

/**
 * Base class for all value types in the animation system.
 *
 * This class provides the interface for all value types. Each derived class
 * must implement the pure virtual functions to provide type-specific behavior.
 */
class Value {
 public:
  virtual ~Value() = default;

  /**
   * Returns the type of the value.
   * @return The ValueType of this value.
   */
  virtual ValueType GetValueType() const { return ValueType::kUnknown; };

  /**
   * Checks if the value is in an empty state.
   * @return true if the value is empty, false otherwise.
   */
  virtual bool IsEmpty() const { return true; };

  /**
   * Creates a deep copy of the value.
   * @return A unique pointer to the copied value.
   */
  virtual std::unique_ptr<Value> Copy() const { return nullptr; };
};

/**
 * Factory class for creating value objects.
 */
class ValueFactory {
 public:
  template <typename T, typename... Args>
  static std::unique_ptr<T> Make(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }
};

/**
 * Integer value type for the animation system.
 */
class Integer : public Value {
 public:
  static constexpr int32_t kMax = std::numeric_limits<int32_t>::max();
  static constexpr int32_t kMin = std::numeric_limits<int32_t>::min();

  /**
   * Constructs an Integer with the specified value.
   * @param value The integer value to store.
   */
  explicit Integer(int32_t value);

  /**
   * Constructs an empty Integer.
   */
  Integer();

  ~Integer() override = default;

  ValueType GetValueType() const override { return ValueType::kInteger; }
  bool IsEmpty() const override;
  std::unique_ptr<Value> Copy() const override;

  /**
   * Gets the stored integer value.
   * @return The stored integer value, or 0 if empty.
   */
  int32_t Get() const { return value_.value_or(0); }

  /**
   * Resets the value to empty state.
   */
  void Reset() { value_.reset(); }

  /**
   * Sets a new integer value.
   * @param value The new integer value to store.
   */
  void Set(int32_t value) { value_ = value; }

 private:
  std::optional<int32_t> value_;
};

/**
 * Float value type for the animation system.
 */
class ANIMAX_EXPORT Float : public Value {
 public:
  static constexpr float kMax = std::numeric_limits<float>::max();
  static constexpr float kMin = std::numeric_limits<float>::min();

  /**
   * Constructs a Float with the specified value.
   * @param value The float value to store.
   */
  explicit Float(float value);

  /**
   * Constructs an empty Float.
   */
  Float();

  ~Float() override = default;

  ValueType GetValueType() const override { return ValueType::kFloat; }
  bool IsEmpty() const override;
  std::unique_ptr<Value> Copy() const override;

  /**
   * Gets the stored float value.
   * @return The stored float value, or 0.0f if empty.
   */
  float Get() const;

  /**
   * Resets the value to empty state.
   */
  void Reset();

  /**
   * Sets a new float value.
   * @param value The new float value to store.
   */
  void Set(float value);

 private:
  std::optional<float> value_;
};

/**
 * Coordinate base type for the animation system.
 */
class CoordinateF : public Value {
 public:
  /**
   * Gets the x coordinate.
   * @return The x coordinate, or default_ if empty.
   */
  float GetX() const { return x_.value_or(default_); }

  /**
   * Gets the y coordinate.
   * @return The y coordinate, or default_ if empty.
   */
  float GetY() const { return y_.value_or(default_); }

  /**
   * Get the z coordinate.
   * @return The z coordinate, or default_ if empty.
   */
  float GetZ() const { return z_.value_or(default_); }

  /**
   * Calculates the length of the point vector.
   * @return The length of the point vector.
   */
  float Length() const {
    return std::hypot(x_.value_or(default_), y_.value_or(default_),
                      z_.value_or(default_));
  }

  /**
   * Checks if this point equals the specified coordinates.
   * @param x The x coordinate to compare.
   * @param y The y coordinate to compare.
   * @param z The z coordinate to compare.
   * @return true if the coordinates match, false otherwise.
   */
  bool Equals(float x, float y, float z) const {
    return x_ == x && y_ == y && z_ == z;
  }

  /**
   * Checks if this point equals another point.
   * @param other The point to compare with.
   * @return true if the points are equal, false otherwise.
   */
  bool Equals(const CoordinateF& other) const {
    return Equals(other.x_.value_or(default_), other.y_.value_or(default_),
                  other.z_.value_or(default_));
  }

  /**
   * Sets new coordinates.
   * @param x The new x coordinate.
   * @param y The new y coordinate.
   * @param z The new z coordinate.
   */
  void Set(float x, float y, float z) {
    x_ = x;
    y_ = y;
    z_ = z;
  }

  /**
   * Resets the point to empty state.
   */
  void Reset() {
    x_.reset();
    y_.reset();
    z_.reset();
  }

  bool IsEmpty() const override {
    return !x_.has_value() || !y_.has_value() || !z_.has_value();
  }

 protected:
  CoordinateF(float default_value, std::optional<float> x = std::nullopt,
              std::optional<float> y = std::nullopt,
              std::optional<float> z = std::nullopt)
      : x_(std::move(x)),
        y_(std::move(y)),
        z_(std::move(z)),
        default_(default_value) {}

 private:
  std::optional<float> x_;
  std::optional<float> y_;
  std::optional<float> z_;
  float default_;
};

class PointF : public CoordinateF {
 public:
  /**
   * Constructs a 2D PointF with the specified coordinates.
   * @param x The x coordinate.
   * @param y The y coordinate.
   */
  PointF(float x, float y) : CoordinateF(0, x, y, 0) {}

  /**
   * Constructs a 3D PointF with the specified coordinates.
   * @param x The x coordinate.
   * @param y The y coordinate.
   * @param z The z coordinate.
   */
  PointF(float x, float y, float z) : CoordinateF(0, x, y, z) {}

  /**
   * Constructs an empty PointF.
   */
  PointF() : CoordinateF(0) {}

  ~PointF() override = default;

  ValueType GetValueType() const override { return ValueType::kPoint; }

  std::unique_ptr<Value> Copy() const override {
    return std::unique_ptr<Value>(new PointF(*this));
  }
};

/**
 * 3D scale value type for the animation system.
 */
class ScaleF : public CoordinateF {
 public:
  /**
   * Constructs a 2D ScaleF with the specified scale factors.
   * @param scale_x The x scale factor.
   * @param scale_y The y scale factor.
   */
  ScaleF(float scale_x, float scale_y) : CoordinateF(1, scale_x, scale_y, 1) {}

  /**
   * Constructs a 3D ScaleF with the specified scale factors.
   * @param scale_x The x scale factor.
   * @param scale_y The y scale factor.
   * @param scale_z The z scale factor.
   */
  ScaleF(float scale_x, float scale_y, float scale_z)
      : CoordinateF(1, scale_x, scale_y, scale_z) {}

  /**
   * Constructs an empty ScaleF.
   */
  ScaleF() : CoordinateF(1) {}

  ~ScaleF() override = default;

  ValueType GetValueType() const override { return ValueType::kScale; }

  std::unique_ptr<Value> Copy() const override {
    return std::unique_ptr<Value>(new ScaleF(*this));
  }
};

/**
 * RGBA color value type for the animation system.
 */
class ANIMAX_EXPORT Color : public Value {
 public:
  /**
   * Creates a new empty Color.
   */
  Color();

  /**
   * Creates a new Color with the specified components.
   * @param a The alpha component (0-255).
   * @param r The red component (0-255).
   * @param g The green component (0-255).
   * @param b The blue component (0-255).
   */
  Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b);

  /**
   * Creates a new Color from a 32-bit integer value.
   * @param value The color value in ARGB format.
   */
  explicit Color(int32_t value);

  /**
   * Creates a new Color from a color string.
   * @param color The color string to parse (e.g., "#FF0000" or "rgb(255,0,0)").
   */
  explicit Color(const std::string& color);

  /**
   * Converts color components to a 32-bit integer.
   * @param a The alpha component (0-255).
   * @param r The red component (0-255).
   * @param g The green component (0-255).
   * @param b The blue component (0-255).
   * @return The color as a 32-bit integer.
   */
  static int32_t ToInt(uint8_t a, uint8_t r, uint8_t g, uint8_t b);

  ~Color() override = default;

  ValueType GetValueType() const override { return ValueType::kColor; }
  bool IsEmpty() const override;
  std::unique_ptr<Value> Copy() const override;

  /**
   * Gets the alpha component.
   * @return The alpha component (0-255), or 0 if empty.
   */
  uint8_t GetA() const { return a_.value_or(0); }

  /**
   * Gets the red component.
   * @return The red component (0-255), or 0 if empty.
   */
  uint8_t GetR() const { return r_.value_or(0); }

  /**
   * Gets the green component.
   * @return The green component (0-255), or 0 if empty.
   */
  uint8_t GetG() const { return g_.value_or(0); }

  /**
   * Gets the blue component.
   * @return The blue component (0-255), or 0 if empty.
   */
  uint8_t GetB() const { return b_.value_or(0); }

  /**
   * Gets the color as a 32-bit integer.
   * @return The color in ARGB format.
   */
  int32_t GetInt() const {
    return ToInt(a_.value_or(0), r_.value_or(0), g_.value_or(0),
                 b_.value_or(0));
  }

  /**
   * Resets the color to empty state.
   */
  void Reset();

  /**
   * Sets the alpha component.
   * @param a The new alpha value (0-255).
   */
  void SetA(uint8_t a) { a_ = a; }

  /**
   * Sets the color from a 32-bit integer value.
   * @param value The color value in ARGB format.
   */
  void Set(int32_t value);

  /**
   * Sets the color components.
   * @param a The alpha component (0-255).
   * @param r The red component (0-255).
   * @param g The green component (0-255).
   * @param b The blue component (0-255).
   */
  void Set(uint8_t a, uint8_t r, uint8_t g, uint8_t b);

  /**
   * Sets the color from a string.
   * @param color The color string to parse.
   */
  void Set(const std::string& color);

 private:
  std::optional<uint8_t> a_;
  std::optional<uint8_t> r_;
  std::optional<uint8_t> g_;
  std::optional<uint8_t> b_;
};

class Orientation : public CoordinateF {
 public:
  /**
   * Constructs an orientation from Euler angles.
   * The Euler angles represent rotations around the coordinate axes.
   * @param alpha_degree Rotation angle around the X-axis in radians.
   * @param beta_degree Rotation angle around the Y-axis in radians.
   * @param gamma_degree Rotation angle around the Z-axis in radians.
   */
  Orientation(float alpha_degree, float beta_degree, float gamma_degree)
      : CoordinateF(0, alpha_degree, beta_degree, gamma_degree) {}

  /**
   * Constructs an empty Orientation.
   */
  Orientation() : CoordinateF(0) {}

  ~Orientation() override = default;

  ValueType GetValueType() const override { return ValueType::kOrientation; }

  std::unique_ptr<Value> Copy() const override {
    return std::unique_ptr<Value>(new Orientation(*this));
  }
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_VALUE_BASE_VALUE_H_
