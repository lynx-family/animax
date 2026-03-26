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

#ifndef ANIMAX_SRC_CONTENT_SHAPE_REPEATER_CONTENT_H_
#define ANIMAX_SRC_CONTENT_SHAPE_REPEATER_CONTENT_H_

#include "src/content/content.h"
#include "src/content/content_group.h"
#include "src/layer/base_layer.h"
#include "src/render/matrix.h"

namespace lynx {
namespace animax {

class RepeaterModel;

class RepeaterContent : public AnimationListener, public Content {
 public:
  RepeaterContent(BaseLayer& layer, RepeaterModel& model);

  void Draw(Canvas& canvas, Matrix& parent_matrix,
            int32_t parent_alpha) override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parents) override;

  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;
  void Init() override;

  Path* GetPath() override;
  void OnValueChanged() override;
  void AbsorbContent(std::vector<std::unique_ptr<Content>>& contents) override;

  bool SubDrawingType() override { return true; }
  bool SubPathType() override { return true; }
  bool SubGreedyType() override { return true; }

  void ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                      ResolvedKeyPathElements& match_elements,
                      const AnimaXKeyPath& current_partial_path) override;

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

 private:
  std::unique_ptr<Matrix> matrix_;
  std::unique_ptr<Path> path_;

  BaseLayer& layer_;

  std::unique_ptr<FloatKeyframeAnimation> copies_;
  std::unique_ptr<FloatKeyframeAnimation> offset_;
  std::unique_ptr<TransformKeyframeAnimation> transform_;

  std::unique_ptr<ContentGroup> content_group_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_SHAPE_REPEATER_CONTENT_H_
