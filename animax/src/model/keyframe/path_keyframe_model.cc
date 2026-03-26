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

#include "src/model/keyframe/path_keyframe_model.h"
namespace lynx {
namespace animax {
template <PathPlane>
struct PathPoint;
template <>
struct PathPoint<PathPlane::kXY> {
  float first, second;
  PathPoint(PointF& p) : first(p.GetX()), second(p.GetY()){};
};
template <>
struct PathPoint<PathPlane::kYZ> {
  float first, second;
  PathPoint(PointF& p) : first(p.GetY()), second(p.GetZ()){};
};

PathKeyframeModel::PathKeyframeModel(CompositionModel& composition,
                                     KeyframeModel& keyframe)
    : KeyframeModel(composition, keyframe.CopyStartValue(),
                    keyframe.CopyEndValue(),
                    std::move(keyframe.GetInterpolator()),
                    keyframe.GetStartFrame(), keyframe.GetEndFrame()),
      path_() {
  InitControlPoints(keyframe);
}

void PathKeyframeModel::InitControlPoints(KeyframeModel& keyframe) {
  auto* cp1 = keyframe.GetPathCp1();
  auto new_cp1 =
      cp1 ? ValueFactory::Make<PointF>(cp1->GetX(), cp1->GetY(), cp1->GetZ())
          : ValueFactory::Make<PointF>();
  auto* cp2 = keyframe.GetPathCp2();
  auto new_cp2 =
      cp2 ? ValueFactory::Make<PointF>(cp2->GetX(), cp2->GetY(), cp2->GetZ())
          : ValueFactory::Make<PointF>();
  SetPathCps(std::move(new_cp1), std::move(new_cp2));
}

void PathKeyframeModel::OnValueChanged() { is_path_valid_ = false; }

template <PathPlane plan>
void PathKeyframeModel::CreatePath(std::unique_ptr<Path>& path) {
  if (!IsStartValueEmpty() && !IsEndValueEmpty()) {
    auto* start_value = GetStartValue<PointF>();
    auto* end_value = GetEndValue<PointF>();
    if (!start_value || !end_value) {
      return;
    }

    bool equals = start_value->Equals(*end_value);
    if (equals) {
      return;
    }

    if (!path) {
      path = std::make_unique<Path>();
    }

    PathPoint<plan> start(*start_value), end(*end_value);
    path->Reset();
    path->MoveTo(start.first, start.second);

    if (IsPathCpNotEmpty()) {
      auto cp1 = GetPathCp1() ? *GetPathCp1() : PointF();
      auto cp2 = GetPathCp2() ? *GetPathCp2() : PointF();
      if (cp1.Length() != 0 || cp2.Length() != 0) {
        PathPoint<plan> cpp1(cp1), cpp2(cp2);
        path->CubicTo(start.first + cpp1.first, start.second + cpp1.second,
                      end.first + cpp2.first, end.second + cpp2.second,
                      end.first, end.second);
      } else {
        path->LineTo(end.first, end.second);
      }
    } else {
      path->LineTo(end.first, end.second);
    }
    is_path_valid_ = true;
  }
}

template <PathPlane plan>
Path* PathKeyframeModel::GetOrCreatePath() {
  auto& path = GetPath(plan);
  if (!is_path_valid_ || !path) {
    CreatePath<plan>(path);
  }
  return path.get();
}
template Path* PathKeyframeModel::GetOrCreatePath<PathPlane::kXY>();
template Path* PathKeyframeModel::GetOrCreatePath<PathPlane::kYZ>();

}  // namespace animax
}  // namespace lynx
