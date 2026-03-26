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

#include "src/base/log/log.h"
#include "src/base/util/misc_util.h"

namespace lynx {
namespace animax {

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
  if (gc1.size_ != gc2.size_) {
    ANIMAX_LOGE("gradient color size not same.");
    return;
  }

  // Ensure the current object is properly initialized to the correct size
  // before writing data.
  auto size = gc1.GetSize();
  Init(size);

  for (auto i = 0; i < size; i++) {
    positions_[i] = Lerp(gc1.positions_[i], gc2.positions_[i], progress);
    colors_[i] = GammaEvaluate(gc1.colors_[i], gc2.colors_[i], progress);
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
