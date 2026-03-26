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

#include "src/model/value/shape_data.h"

#include <algorithm>

#include "src/base/log/log.h"
#include "src/base/util/misc_util.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

ShapeData::ShapeData() : closed_(false) {}

void ShapeData::Init(const PointF& initial_point, bool closed) {
  initial_point_ = initial_point;
  closed_ = closed;
}

std::unique_ptr<Value> ShapeData::Copy() const {
  auto model = std::unique_ptr<ShapeData>(new ShapeData());
  model->initial_point_ = initial_point_;
  model->closed_ = closed_;
  model->curves_ = curves_;
  return model;
}

ValueType ShapeData::GetValueType() const { return ValueType::kShape; }

bool ShapeData::IsEmpty() const { return initial_point_.IsEmpty(); }

void ShapeData::InterpolateBetween(ShapeData& shape_data1,
                                   ShapeData& shape_data2, float percentage) {
  closed_ = shape_data1.IsClosed() || shape_data2.IsClosed();

  if (shape_data1.GetCurves().size() != shape_data2.GetCurves().size()) {
    ANIMAX_LOGI("curves not same size");
  }

  auto point_size =
      std::min(shape_data1.GetCurves().size(), shape_data2.GetCurves().size());
  if (curves_.size() < point_size) {
    for (auto i = curves_.size(); i < point_size; i++) {
      curves_.push_back(CubicCurveModel::Make());
    }
  } else if (curves_.size() > point_size) {
    curves_.resize(point_size);
  }

  auto& initial_point1 = shape_data1.GetInitialPoint();
  auto& initial_point2 = shape_data2.GetInitialPoint();

  SetInitialPoint(
      Lerp(initial_point1.GetX(), initial_point2.GetX(), percentage),
      Lerp(initial_point1.GetY(), initial_point2.GetY(), percentage));

  for (size_t i = 0; i < curves_.size(); i++) {
    auto& curve1 = shape_data1.GetCurves()[i];
    auto& curve2 = shape_data2.GetCurves()[i];

    auto& cp11 = curve1.GetControlPoint1();
    auto& cp21 = curve1.GetControlPoint2();
    auto& vertex1 = curve1.GetVertex();

    auto& cp12 = curve2.GetControlPoint1();
    auto& cp22 = curve2.GetControlPoint2();
    auto& vertex2 = curve2.GetVertex();

    curves_[i].SetControlPoint1(Lerp(cp11.GetX(), cp12.GetX(), percentage),
                                Lerp(cp11.GetY(), cp12.GetY(), percentage));
    curves_[i].SetControlPoint2(Lerp(cp21.GetX(), cp22.GetX(), percentage),
                                Lerp(cp21.GetY(), cp22.GetY(), percentage));
    curves_[i].SetVertex(Lerp(vertex1.GetX(), vertex2.GetX(), percentage),
                         Lerp(vertex1.GetY(), vertex2.GetY(), percentage));
  }
}

void ShapeData::SetInitialPoint(float x, float y) {
  initial_point_.Set(x, y, 0);
}

}  // namespace animax
}  // namespace lynx
