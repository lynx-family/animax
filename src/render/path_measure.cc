// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/path_measure.h"

#include "skity/graphic/path_measure.hpp"
#include "src/model/value/base_value.h"
#include "src/render/path.h"

namespace lynx {
namespace animax {

PathMeasure::PathMeasure() : pm_(std::make_unique<skity::PathMeasure>()) {}

PathMeasure::~PathMeasure() = default;

void PathMeasure::SetPath(Path& path, bool force_close) {
  pm_->SetPath(&path.GetPath(), force_close);
}

bool PathMeasure::NextContour() { return pm_->NextContour(); }

float PathMeasure::GetLength() { return pm_->GetLength(); }

bool PathMeasure::GetPosTan(float distance, PointF* out_pos) {
  skity::Point pos{};

  bool result = pm_->GetPosTan(distance, &pos, nullptr);
  if (result) {
    out_pos->Set(pos.x, pos.y, pos.z);
  }
  return result;
}

bool PathMeasure::GetSegment(float start, float stop, Path& dst,
                             bool start_with_move_to) {
  return pm_->GetSegment(start, stop, &dst.GetPath(), start_with_move_to);
}

}  // namespace animax
}  // namespace lynx
