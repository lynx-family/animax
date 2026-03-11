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

#include "src/layer/shape_layer.h"

#include <memory>
#include <vector>

#include "src/layer/composition_layer.h"

namespace lynx {
namespace animax {

ShapeLayer::ShapeLayer(LayerModel& layer_model, CompositionModel& composition)
    : BaseLayer(layer_model, composition) {}

void ShapeLayer::Init() {
  BaseLayer::Init();

  std::string name = "__container";
  shape_group_model_ = std::unique_ptr<ShapeGroupModel>(
      new ShapeGroupModel(name, layer_model_.GetShapes(), false));
  content_group_ = std::unique_ptr<ContentGroup>(
      new ContentGroup(*this, *shape_group_model_, composition_));
  content_group_->Init();

  auto empty_contents = std::vector<Content*>();
  content_group_->SetContents(empty_contents, empty_contents);
}

void ShapeLayer::DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) {
  if (!content_group_) {
    // TODO(aiyongbiao): need check at else
    return;
  }

  if (!GetLayerModel().GetEnable3D()) {
    content_group_->Draw(canvas, matrix, alpha);
  } else {
    // content_group_->Draw can not use 3d matrix, save and concat matrix first
    canvas.Save();
    canvas.Concat(matrix);
    Matrix local_matrix;
    content_group_->Draw(canvas, local_matrix, alpha);
    canvas.Restore();
  }
}

void ShapeLayer::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                           bool apply_parent) {
  BaseLayer::GetBounds(out_bounds, parent_matrix, apply_parent);
  if (content_group_) {
    content_group_->GetBounds(out_bounds, *bounds_matrix_, apply_parent);
  }
}

void ShapeLayer::ResolveChildKeyPath(
    const AnimaXKeyPath& path, int32_t depth,
    ResolvedKeyPathElements& match_elements,
    const AnimaXKeyPath& current_partial_path) {
  if (content_group_) {
    content_group_->ResolveKeyPath(path, depth, match_elements,
                                   current_partial_path);
  }
}

}  // namespace animax
}  // namespace lynx
