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

#include "src/model/value/gradient_color.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "src/base/log/log.h"
#include "src/base/util/misc_util.h"

namespace lynx {
namespace animax {
namespace {

struct MergedGradientPosition {
  float position;
  int32_t occurrence;
};

int32_t SampleGradientColor(const GradientColor& gradient, float position,
                            int32_t occurrence) {
  auto size = gradient.GetSize();
  auto* positions = gradient.GetPositions();
  auto* colors = gradient.GetColors();
  if (size <= 0 || positions == nullptr || colors == nullptr) {
    return 0;
  }

  auto exact_index = 0;
  while (exact_index < size && positions[exact_index] < position) {
    exact_index++;
  }
  if (exact_index < size && positions[exact_index] == position) {
    auto exact_count = 0;
    while (exact_index + exact_count < size &&
           positions[exact_index + exact_count] == position) {
      exact_count++;
    }
    auto sampled_occurrence = std::min(occurrence, exact_count - 1);
    return colors[exact_index + sampled_occurrence];
  }

  auto upper_index = 0;
  while (upper_index < size && positions[upper_index] < position) {
    upper_index++;
  }

  if (upper_index == 0) {
    return colors[0];
  }
  if (upper_index == size) {
    return colors[size - 1];
  }

  auto lower_index = upper_index - 1;
  auto start_position = positions[lower_index];
  auto end_position = positions[upper_index];
  if (end_position <= start_position) {
    return colors[lower_index];
  }

  auto segment_progress =
      (position - start_position) / (end_position - start_position);
  return GammaEvaluate(colors[lower_index], colors[upper_index],
                       segment_progress);
}

std::vector<MergedGradientPosition> MergeGradientPositions(
    const GradientColor& gc1, const GradientColor& gc2) {
  std::vector<MergedGradientPosition> positions;
  positions.reserve(gc1.GetSize() + gc2.GetSize());

  auto* positions1 = gc1.GetPositions();
  auto* positions2 = gc2.GetPositions();
  auto size1 = gc1.GetSize();
  auto size2 = gc2.GetSize();
  auto index1 = 0;
  auto index2 = 0;

  while (index1 < size1 || index2 < size2) {
    auto take_first =
        index2 >= size2 ||
        (index1 < size1 && positions1[index1] < positions2[index2]);
    auto position = take_first ? positions1[index1] : positions2[index2];

    auto count1 = 0;
    while (index1 < size1 && positions1[index1] == position) {
      count1++;
      index1++;
    }

    auto count2 = 0;
    while (index2 < size2 && positions2[index2] == position) {
      count2++;
      index2++;
    }

    auto count = std::max(count1, count2);
    for (auto i = 0; i < count; i++) {
      positions.push_back({position, i});
    }
  }

  return positions;
}

}  // namespace

GradientColor::GradientColor()
    : size_(0), positions_(nullptr), colors_(nullptr) {}

GradientColor::GradientColor(int32_t size) { Init(size); }

GradientColor::GradientColor(std::unique_ptr<float[]> positions,
                             std::unique_ptr<int32_t[]> colors, int32_t size)
    : size_(size),
      positions_(std::move(positions)),
      colors_(std::move(colors)) {}

bool GradientColor::IsEmpty() const { return size_ <= 0; }

std::unique_ptr<Value> GradientColor::Copy() const {
  auto model = std::unique_ptr<GradientColor>(new GradientColor());
  if (IsEmpty()) {
    return model;
  }
  model->Init(GetSize());
  std::copy(positions_.get(), positions_.get() + GetSize(),
            model->positions_.get());
  std::copy(colors_.get(), colors_.get() + GetSize(), model->colors_.get());
  return model;
}

void GradientColor::LerpColor(GradientColor& gc1, GradientColor& gc2,
                              float progress) {
  if (gc1.IsEmpty() || gc2.IsEmpty()) {
    ANIMAX_LOGE("gradient color is empty.");
    return;
  }

  if (gc1.size_ == gc2.size_) {
    // Ensure the current object is properly initialized to the correct size
    // before writing data.
    auto size = gc1.GetSize();
    Init(size);

    for (auto i = 0; i < size; i++) {
      positions_[i] = Lerp(gc1.positions_[i], gc2.positions_[i], progress);
      colors_[i] = GammaEvaluate(gc1.colors_[i], gc2.colors_[i], progress);
    }
    return;
  }

  auto merged_positions = MergeGradientPositions(gc1, gc2);
  Init(static_cast<int32_t>(merged_positions.size()));

  for (auto i = 0; i < static_cast<int32_t>(merged_positions.size()); i++) {
    auto position = merged_positions[i].position;
    auto occurrence = merged_positions[i].occurrence;
    positions_[i] = position;
    colors_[i] =
        GammaEvaluate(SampleGradientColor(gc1, position, occurrence),
                      SampleGradientColor(gc2, position, occurrence), progress);
  }
}

void GradientColor::Init(int32_t size) {
  if (size == GetSize() && positions_ && colors_) {
    return;
  }

  size_ = size;
  if (IsEmpty()) {
    return;
  }
  positions_ = std::make_unique<float[]>(size);
  colors_ = std::make_unique<int32_t[]>(size);
}

void GradientColor::UpdateColors(std::unique_ptr<int32_t[]> colors) {
  colors_ = std::move(colors);
}

}  // namespace animax
}  // namespace lynx
