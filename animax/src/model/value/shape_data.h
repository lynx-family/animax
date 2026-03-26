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

#ifndef ANIMAX_SRC_MODEL_VALUE_SHAPE_DATA_H_
#define ANIMAX_SRC_MODEL_VALUE_SHAPE_DATA_H_

#include <memory>
#include <vector>

#include "src/model/shape/cubic_curve_model.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

/**
 * Shape data representing a collection of cubic curves forming a shape.
 *
 * This class stores the initial point and a list of cubic curves that define
 * a shape. It supports interpolation between shapes and can be marked as
 * closed.
 */
class ShapeData : public Value {
 public:
  /**
   * Constructs an empty ShapeData.
   */
  ShapeData();
  ~ShapeData() override = default;

  /**
   * Returns the type of the value.
   * @return The ValueType of this value, which is kShape.
   */
  ValueType GetValueType() const override;

  /**
   * Checks if the shape data is empty.
   * @return true if there are no curves and the initial point is not set, false
   * otherwise.
   */
  bool IsEmpty() const override;

  /**
   * Creates a deep copy of the ShapeData.
   * @return A unique pointer to the copied ShapeData instance.
   */
  std::unique_ptr<Value> Copy() const override;

  /**
   * Initializes the ShapeData with an initial point and closed state.
   * @param initial_point The starting point of the shape.
   * @param closed Whether the shape is closed (connects back to the initial
   * point).
   */
  void Init(const PointF& initial_point, bool closed);

  /**
   * Gets the list of cubic curves defining the shape.
   * @return A reference to the vector of CubicCurveModel objects.
   */
  std::vector<CubicCurveModel>& GetCurves() { return curves_; }

  /**
   * Gets the initial point of the shape.
   * @return A reference to the initial PointF object.
   */
  PointF& GetInitialPoint() { return initial_point_; }

  /**
   * Interpolates between two ShapeData objects based on a percentage.
   * @param shape_data1 The starting shape data.
   * @param shape_data2 The ending shape data.
   * @param percentage The interpolation percentage (0.0 to 1.0).
   */
  void InterpolateBetween(ShapeData& shape_data1, ShapeData& shape_data2,
                          float percentage);

  /**
   * Checks if the shape is closed.
   * @return true if the shape is closed, false otherwise.
   */
  bool IsClosed() const { return closed_; }

  /**
   * Sets the initial point coordinates.
   * @param x The x-coordinate of the initial point.
   * @param y The y-coordinate of the initial point.
   */
  void SetInitialPoint(float x, float y);

  /**
   * Sets whether the shape is closed.
   * @param closed true to close the shape, false to open it.
   */
  void SetClosed(bool closed) { closed_ = closed; }

 private:
  PointF initial_point_;
  bool closed_;
  std::vector<CubicCurveModel> curves_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_VALUE_SHAPE_DATA_H_
