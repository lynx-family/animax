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

#ifndef ANIMAX_SRC_CONTENT_CONTENT_GROUP_H_
#define ANIMAX_SRC_CONTENT_CONTENT_GROUP_H_

#include <memory>
#include <string>
#include <vector>

#include "src/animation/keyframe_animation.h"
#include "src/animation/transform_keyframe_animation.h"
#include "src/content/content.h"
#include "src/model/content_model.h"

namespace lynx {
namespace animax {

class ShapeGroupModel;
class AnimatableTransformModel;
class BaseLayer;
class CompositionModel;

class ContentGroup : public Content, public AnimationListener {
 public:
  static void ContentsFromModels(
      CompositionModel& composition, BaseLayer& layer,
      std::vector<ContentModel*>& content_models,
      std::vector<std::unique_ptr<Content>>& contents);

  static AnimatableTransformModel* FindTransform(
      const std::vector<ContentModel*>& content_models);

  ContentGroup(BaseLayer& layer, ShapeGroupModel& shape_group,
               CompositionModel& composition);
  ContentGroup(BaseLayer& layer, const std::string& name, bool hidden,
               AnimatableTransformModel* transform);

  ContentType MainType() override { return ContentType::kGroup; }

  void Init() override;
  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;
  Path* GetPath() override;

  void Draw(Canvas& canvas, Matrix& parent_matrix,
            int32_t parent_alpha) override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parent) override;
  void OnValueChanged() override;

  void GetPathList(std::vector<Content*>& path_contents);
  Matrix& GetTransformationMatrix();
  std::vector<std::unique_ptr<Content>>& GetContents() { return contents_; }

  bool SubDrawingType() override { return true; }
  bool SubPathType() override { return true; }

  void ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                      ResolvedKeyPathElements& match_elements,
                      const AnimaXKeyPath& current_partial_path) override;

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

 private:
  std::vector<std::unique_ptr<Content>> contents_;

  std::unique_ptr<Matrix> matrix_;
  std::unique_ptr<Path> path_;
  RectF rect_;

  std::unique_ptr<TransformKeyframeAnimation> transform_animation_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_CONTENT_GROUP_H_
