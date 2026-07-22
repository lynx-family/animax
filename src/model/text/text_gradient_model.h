// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_MODEL_TEXT_TEXT_GRADIENT_MODEL_H_
#define ANIMAX_SRC_MODEL_TEXT_TEXT_GRADIENT_MODEL_H_

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "src/model/value/base_value.h"
#include "src/model/value/gradient_color.h"

namespace lynx {
namespace animax {

class TextGradientItemModel {
 public:
  TextGradientItemModel(GradientType type, std::unique_ptr<PointF> start_point,
                        std::unique_ptr<PointF> end_point,
                        std::unique_ptr<PointF> radius_xy,
                        std::unique_ptr<GradientColor> gradient_color,
                        std::optional<float> start_angle = std::nullopt)
      : type_(type),
        start_point_(std::move(start_point)),
        end_point_(std::move(end_point)),
        radius_xy_(std::move(radius_xy)),
        gradient_color_(std::move(gradient_color)),
        start_angle_(start_angle) {}

  GradientType GetType() const { return type_; }
  const PointF& GetStartPoint() const { return *start_point_; }
  const PointF* GetEndPoint() const { return end_point_.get(); }
  const PointF* GetRadiusXY() const { return radius_xy_.get(); }
  const GradientColor& GetGradientColor() const { return *gradient_color_; }
  std::optional<float> GetStartAngle() const { return start_angle_; }

 private:
  GradientType type_ = GradientType::kLinear;
  std::unique_ptr<PointF> start_point_;
  std::unique_ptr<PointF> end_point_;
  std::unique_ptr<PointF> radius_xy_;
  std::unique_ptr<GradientColor> gradient_color_;
  // CSS conic-gradient's `from` angle, in degrees. It is only used for conic
  // items; other gradient types keep the default value.
  std::optional<float> start_angle_;
};

class TextGradientModel {
 public:
  void AddItem(std::unique_ptr<TextGradientItemModel> item) {
    items_.push_back(std::move(item));
  }

  void Append(TextGradientModel&& model) {
    for (auto& item : model.items_) {
      items_.push_back(std::move(item));
    }
  }

  const std::vector<std::unique_ptr<TextGradientItemModel>>& GetItems() const {
    return items_;
  }

 private:
  std::vector<std::unique_ptr<TextGradientItemModel>> items_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_TEXT_TEXT_GRADIENT_MODEL_H_
