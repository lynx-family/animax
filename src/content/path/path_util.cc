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

#include "src/content/path/path_util.h"

#include <algorithm>

#include "src/base/util/misc_util.h"
#include "src/content/path/trim_path_content.h"
#include "src/render/path_measure.h"

namespace lynx {
namespace animax {

void PathUtil::ApplyTrimPathIfNeeds(Path& path, TrimPathContent* trim_path) {
  if (trim_path == nullptr || trim_path->IsHidden()) {
    return;
  }

  auto start = trim_path->GetStart().GetValue().Get();
  auto end = trim_path->GetEnd().GetValue().Get();
  auto offset = trim_path->GetOffset().GetValue().Get();

  ApplyTrimPathIfNeeds(path, start / 100.0, end / 100.0, offset / 360.0);
}

void PathUtil::ApplyTrimPathIfNeeds(Path& path, float start_value,
                                    float end_value, float offset_value) {
  std::unique_ptr<Path> temp_path = std::make_unique<Path>();
  std::unique_ptr<Path> temp_path2 = std::make_unique<Path>();
  std::unique_ptr<PathMeasure> path_measure = std::make_unique<PathMeasure>();

  path_measure->SetPath(path, false);
  auto length = path_measure->GetLength();
  if (start_value == 1.0 && end_value == 0.0) {
    return;
  }

  if (length < 1.0 || std::abs(end_value - start_value - 1) < 0.01) {
    return;
  }

  auto start = length * start_value;
  auto end = length * end_value;
  auto new_start = std::min(start, end);
  auto new_end = std::max(start, end);

  auto offset = offset_value * length;
  new_start += offset;
  new_end += offset;

  if (new_start >= length && new_end >= length) {
    new_start = FloorMod(new_start, length);
    new_end = FloorMod(new_end, length);
  }

  if (new_start < 0) {
    new_start = FloorMod(new_start, length);
  }
  if (new_end < 0) {
    new_end = FloorMod(new_end, length);
  }

  if (new_start == new_end) {
    path.Reset();
    return;
  }

  if (new_start >= new_end) {
    new_start -= length;
  }

  temp_path->Reset();
  path_measure->GetSegment(new_start, new_end, *temp_path, true);
  if (new_end > length) {
    temp_path2->Reset();
    path_measure->GetSegment(0, fmod(new_end, length), *temp_path2, true);
    temp_path->AddPath(*temp_path2);
  } else if (new_start < 0) {
    temp_path2->Reset();
    path_measure->GetSegment(length + new_start, length, *temp_path2, true);
    temp_path->AddPath(*temp_path2);
  }
  path.Set(*temp_path);
}

float PathUtil::ToRadians(float ang_deg) { return ang_deg * kDegreesToRadians; }

float PathUtil::ToDegrees(float radians) { return radians * kRadiansToDegrees; }

}  // namespace animax
}  // namespace lynx
