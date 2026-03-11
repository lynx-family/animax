// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
//
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

#ifndef ANIMAX_SRC_MODEL_VALUE_DOCUMENT_DATA_H_
#define ANIMAX_SRC_MODEL_VALUE_DOCUMENT_DATA_H_

#include <memory>
#include <optional>
#include <string>

#include "include/base/macros.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

enum class DocumentJustification : uint8_t {
  kLeftAlign = 0,
  kRightAlign,
  kCenter,
  kJustifyWithLastLineLeft,
  kJustifyWithLastLineRight,
  kJustifyWithLastLineCenter,
  kJustifyWithLastLineFull,
};

class ANIMAX_EXPORT DocumentData : public Value {
 public:
  DocumentData();
  explicit DocumentData(std::string text, std::string font_name, float size,
                        DocumentJustification justification, uint32_t tracking,
                        float line_height, float baseline_shift, int32_t color,
                        int32_t stroke_color, float stroke_width,
                        bool stroke_overfill,
                        std::unique_ptr<PointF> box_position,
                        std::unique_ptr<PointF> box_size);
  ~DocumentData() override = default;

  ValueType GetValueType() const override;
  std::unique_ptr<Value> Copy() const override;
  bool IsEmpty() const override;

  PointF* GetBoxPosition() const {
    return box_position_ ? box_position_.get() : nullptr;
  }
  PointF* GetBoxSize() const { return box_size_ ? box_size_.get() : nullptr; }
  const std::string& GetText() const { return text_; }
  const std::string& GetFontName() const { return font_name_; }
  float GetSize() const { return size_.value_or(0.0f); }
  DocumentJustification GetJustification() const { return justification_; }
  int32_t GetTracking() const { return tracking_; }
  float GetLineHeight() const { return line_height_; }
  float GetBaselineShift() const { return baseline_shift_; }
  int32_t GetColor() const { return color_; }
  int32_t GetStrokeColor() const { return stroke_color_; }
  float GetStrokeWidth() const { return stroke_width_; }
  bool GetStrokeOverfill() const { return stroke_overfill_; }

  void SetText(std::string text) { text_ = std::move(text); }
  void SetColor(int32_t color) { color_ = color; }
  void SetSize(float size) { size_ = size; }

 private:
  std::string text_;
  std::string font_name_;
  std::optional<float> size_;
  DocumentJustification justification_ = DocumentJustification::kLeftAlign;
  int32_t tracking_ = 0;
  float line_height_ = 0;
  float baseline_shift_ = 0;
  int32_t color_ = 0;
  int32_t stroke_color_ = 0;
  float stroke_width_ = 0;
  bool stroke_overfill_ = false;
  std::unique_ptr<PointF> box_position_;
  std::unique_ptr<PointF> box_size_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_VALUE_DOCUMENT_DATA_H_
