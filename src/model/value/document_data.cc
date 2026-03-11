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

#include "src/model/value/document_data.h"

#include <cmath>
#include <utility>

namespace lynx {
namespace animax {

DocumentData::DocumentData()
    : text_(),
      font_name_(),
      size_(std::nullopt),
      justification_(DocumentJustification::kLeftAlign),
      tracking_(0),
      line_height_(0),
      baseline_shift_(0),
      color_(0),
      stroke_color_(0),
      stroke_width_(0),
      stroke_overfill_(false),
      box_position_(nullptr),
      box_size_(nullptr) {}

DocumentData::DocumentData(std::string text, std::string font_name, float size,
                           DocumentJustification justification,
                           uint32_t tracking, float line_height,
                           float baseline_shift, int32_t color,
                           int32_t stroke_color, float stroke_width,
                           bool stroke_overfill,
                           std::unique_ptr<PointF> box_position,
                           std::unique_ptr<PointF> box_size)
    : text_(std::move(text)),
      font_name_(std::move(font_name)),
      size_(size),
      justification_(justification),
      tracking_(tracking),
      line_height_(line_height),
      baseline_shift_(baseline_shift),
      color_(color),
      stroke_color_(stroke_color),
      stroke_width_(stroke_width),
      stroke_overfill_(stroke_overfill),
      box_position_(std::move(box_position)),
      box_size_(std::move(box_size)) {}

ValueType DocumentData::GetValueType() const { return ValueType::kDocument; }

std::unique_ptr<Value> DocumentData::Copy() const {
  auto model = std::unique_ptr<DocumentData>(new DocumentData());
  model->text_ = text_;
  model->font_name_ = font_name_;
  model->size_ = size_;
  model->justification_ = justification_;
  model->tracking_ = tracking_;
  model->line_height_ = line_height_;
  model->baseline_shift_ = baseline_shift_;
  model->color_ = color_;
  model->stroke_color_ = stroke_color_;
  model->stroke_width_ = stroke_width_;
  model->stroke_overfill_ = stroke_overfill_;
  if (box_position_) {
    model->box_position_ = ValueFactory::Make<PointF>(box_position_->GetX(),
                                                      box_position_->GetY());
  } else {
    model->box_position_ = nullptr;
  }
  if (box_size_) {
    model->box_size_ =
        ValueFactory::Make<PointF>(box_size_->GetX(), box_size_->GetY());
  } else {
    model->box_size_ = nullptr;
  }
  return model;
}

bool DocumentData::IsEmpty() const { return !size_.has_value(); }

}  // namespace animax
}  // namespace lynx
