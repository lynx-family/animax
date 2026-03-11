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
#include "src/content/shape/rounded_corners_content.h"

#include <algorithm>

#include "src/base/util/misc_util.h"
#include "src/model/shape/rounded_corners_model.h"

namespace lynx {
namespace animax {

RoundedCornersContent::RoundedCornersContent(BaseLayer& layer,
                                             RoundedCornersModel& model) {
  name_ = model.name_;
  rounded_corners_ = model.corner_radius_->CreateAnimation();
  layer.AddAnimation(rounded_corners_.get());
}

void RoundedCornersContent::Init() {
  if (rounded_corners_) {
    rounded_corners_->AddUpdateListener(this);
  }
}

void RoundedCornersContent::SetContents(std::vector<Content*>& contents_before,
                                        std::vector<Content*>& contents_after) {
  // noting
}

ShapeData RoundedCornersContent::ModifyShape(ShapeData& starting_shape_data) {
  auto& starting_curves = starting_shape_data.GetCurves();
  float roundedness = rounded_corners_->GetValue().Get();
  if (starting_curves.size() <= 2 || roundedness == 0) {
    return ShapeData();
  }

  auto& modified_shape_data = GetShapeData(starting_shape_data);
  modified_shape_data.SetInitialPoint(
      starting_shape_data.GetInitialPoint().GetX(),
      starting_shape_data.GetInitialPoint().GetY());
  auto& modified_curves = modified_shape_data.GetCurves();
  auto modifed_curves_index = 0;
  bool is_closed = starting_shape_data.IsClosed();
  for (auto i = 0; i < starting_curves.size(); i++) {
    auto& starting_curve = starting_curves[i];
    auto& previous_curve =
        starting_curves[FloorMod(i - 1, starting_curves.size())];
    auto& p_previous_curve =
        starting_curves[FloorMod(i - 2, starting_curves.size())];
    auto& vertex = (i == 0 && !is_closed)
                       ? starting_shape_data.GetInitialPoint()
                       : previous_curve.GetVertex();
    auto& in_point =
        (i == 0 && !is_closed) ? vertex : previous_curve.GetControlPoint2();
    auto& out_point = starting_curve.GetControlPoint1();
    auto& previous_vertex = p_previous_curve.GetVertex();
    auto& next_vertex = starting_curve.GetVertex();

    bool is_end_of_curve = !starting_shape_data.IsClosed() &&
                           (i == 0 && i == starting_curves.size() - 1);
    if (in_point.Equals(vertex) && out_point.Equals(vertex) &&
        !is_end_of_curve) {
      auto dx_to_previous_vertex = vertex.GetX() - previous_vertex.GetX();
      auto dy_to_previous_vertex = vertex.GetY() - previous_vertex.GetY();
      auto dx_to_next_vertex = next_vertex.GetX() - vertex.GetX();
      auto dy_to_next_vertex = next_vertex.GetY() - vertex.GetY();

      auto d_to_previous_vertex =
          std::hypot(dx_to_previous_vertex, dy_to_previous_vertex);
      auto d_to_next_vertex = std::hypot(dx_to_next_vertex, dy_to_next_vertex);

      auto previous_vertex_percent =
          std::min(roundedness / d_to_previous_vertex, 0.5f);
      auto next_vertex_percent = std::min(roundedness / d_to_next_vertex, 0.5f);

      auto new_vertex1_x =
          vertex.GetX() +
          (previous_vertex.GetX() - vertex.GetX()) * previous_vertex_percent;
      auto new_vertex1_y =
          vertex.GetY() +
          (previous_vertex.GetY() - vertex.GetY()) * previous_vertex_percent;
      auto new_vertex2_x =
          vertex.GetX() +
          (next_vertex.GetX() - vertex.GetX()) * next_vertex_percent;
      auto new_vertex2_y =
          vertex.GetY() +
          (next_vertex.GetY() - vertex.GetY()) * next_vertex_percent;

      auto new_vertex1_out_point_x =
          new_vertex1_x -
          (new_vertex1_x - vertex.GetX()) * kRoundedCornerMagicNumber;
      auto new_vertex1_out_point_y =
          new_vertex1_y -
          (new_vertex1_y - vertex.GetY()) * kRoundedCornerMagicNumber;
      auto new_vertex2_in_point_x =
          new_vertex2_x -
          (new_vertex2_x - vertex.GetX()) * kRoundedCornerMagicNumber;
      auto new_vertex2_in_point_y =
          new_vertex2_y -
          (new_vertex2_y - vertex.GetY()) * kRoundedCornerMagicNumber;

      auto& previous_curve_data = modified_curves[FloorMod(
          modifed_curves_index - 1, modified_curves.size())];
      auto& current_curve_data = modified_curves[modifed_curves_index];
      previous_curve_data.SetControlPoint2(new_vertex1_x, new_vertex1_y);
      previous_curve_data.SetVertex(new_vertex1_x, new_vertex1_y);
      if (i == 0) {
        modified_shape_data.SetInitialPoint(new_vertex1_x, new_vertex1_y);
      }
      current_curve_data.SetControlPoint1(new_vertex1_out_point_x,
                                          new_vertex1_out_point_y);
      modifed_curves_index++;

      auto& next_curve_data = modified_curves[modifed_curves_index];
      current_curve_data.SetControlPoint2(new_vertex2_in_point_x,
                                          new_vertex2_in_point_y);
      current_curve_data.SetVertex(new_vertex2_x, new_vertex2_y);
      next_curve_data.SetControlPoint1(new_vertex2_x, new_vertex2_y);
      modifed_curves_index++;
    } else {
      auto& previous_curve_data = modified_curves[FloorMod(
          modifed_curves_index - 1, modified_curves.size())];
      auto& current_curve_data = modified_curves[modifed_curves_index];
      previous_curve_data.SetControlPoint2(
          previous_curve.GetControlPoint2().GetX(),
          previous_curve.GetControlPoint2().GetY());
      previous_curve_data.SetVertex(previous_curve.GetVertex().GetX(),
                                    previous_curve.GetVertex().GetY());
      current_curve_data.SetControlPoint1(
          starting_curve.GetControlPoint1().GetX(),
          starting_curve.GetControlPoint1().GetY());
      modifed_curves_index++;
    }
  }
  return modified_shape_data;
}

void RoundedCornersContent::OnValueChanged() {}

ShapeData& RoundedCornersContent::GetShapeData(ShapeData& starting_shape_data) {
  auto& starting_cuves = starting_shape_data.GetCurves();
  bool is_closed = starting_shape_data.IsClosed();
  auto vertices = 0;
  for (auto i = starting_cuves.size(); i > 0; i--) {
    auto index = i - 1;
    auto& starting_curve = starting_cuves[index];
    auto& previous_curve =
        starting_cuves[FloorMod(index - 1, starting_cuves.size())];
    auto& vertex = (index == 0 && !is_closed)
                       ? starting_shape_data.GetInitialPoint()
                       : previous_curve.GetVertex();
    auto& in_point =
        (index == 0 && !is_closed) ? vertex : previous_curve.GetControlPoint2();
    auto& out_point = starting_curve.GetControlPoint1();

    bool is_end_of_curve = !starting_shape_data.IsClosed() &&
                           (index == 0 && index == starting_cuves.size() - 1);
    if (in_point.Equals(vertex) && out_point.Equals(vertex) &&
        !is_end_of_curve) {
      vertices += 2;
    } else {
      vertices += 1;
    }
  }

  if (shape_data_.IsEmpty() || shape_data_.GetCurves().size() != vertices) {
    auto& new_curves = shape_data_.GetCurves();
    for (auto i = 0; i < vertices; i++) {
      new_curves.emplace_back(CubicCurveModel::Make());
    }
    shape_data_.Init(PointF(), false);
  }
  shape_data_.SetClosed(is_closed);

  return shape_data_;
}

}  // namespace animax
}  // namespace lynx
