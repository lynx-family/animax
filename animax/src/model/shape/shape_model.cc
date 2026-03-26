// Copyright 2023 The Lynx Authors. All rights reserved.
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

#include "src/content/content_group.h"
#include "src/content/shape/ellipse_content.h"
#include "src/content/shape/fill_content.h"
#include "src/content/shape/polystar_content.h"
#include "src/content/shape/rectangle_content.h"
#include "src/content/shape/repeater_content.h"
#include "src/content/shape/rounded_corners_content.h"
#include "src/content/shape/shape_content.h"
#include "src/content/shape/stroke_content.h"
#include "src/layer/base_layer.h"
#include "src/model/composition_model.h"
#include "src/model/shape/circle_shape_model.h"
#include "src/model/shape/polystar_shape_model.h"
#include "src/model/shape/rectangle_shape_model.h"
#include "src/model/shape/repeater_model.h"
#include "src/model/shape/rounded_corners_model.h"
#include "src/model/shape/shape_fill_model.h"
#include "src/model/shape/shape_group_model.h"
#include "src/model/shape/shape_path_model.h"
#include "src/model/shape/shape_stroke_model.h"

namespace lynx {
namespace animax {

std::unique_ptr<Content> ShapeGroupModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<ContentGroup>(
      new ContentGroup(layer, *this, composition));
}

std::unique_ptr<Content> ShapePathModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<ShapeContent>(new ShapeContent(layer, *this));
}

std::unique_ptr<Content> ShapeStrokeModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<StrokeContent>(new StrokeContent(layer, *this));
}

std::unique_ptr<Content> ShapeFillModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<FillContent>(
      new FillContent(layer, *this, composition));
}

std::unique_ptr<Content> RoundedCornersModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<RoundedCornersContent>(
      new RoundedCornersContent(layer, *this));
}

std::unique_ptr<Content> RepeaterModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<RepeaterContent>(new RepeaterContent(layer, *this));
}

std::unique_ptr<Content> RectangleShapeModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<RectangleContent>(new RectangleContent(layer, *this));
}

std::unique_ptr<Content> PolystarShapeModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<PolystarContent>(new PolystarContent(layer, *this));
}

std::unique_ptr<Content> CircleShapeModel::CreateContent(
    CompositionModel& composition, BaseLayer& layer) {
  return std::unique_ptr<EllipseContent>(new EllipseContent(layer, *this));
}

}  // namespace animax
}  // namespace lynx
