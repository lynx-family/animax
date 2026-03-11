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

#ifndef ANIMAX_SRC_CONTENT_SHAPE_SHAPE_CONTENT_H_
#define ANIMAX_SRC_CONTENT_SHAPE_SHAPE_CONTENT_H_

#include <vector>

#include "src/animation/keyframe_animation.h"
#include "src/animation/shape_keyframe_animation.h"
#include "src/content/path/trim_path_content.h"
#include "src/content/shape/shape_modifier_content.h"
#include "src/layer/base_layer.h"
#include "src/model/value/shape_data.h"

namespace lynx {
namespace animax {

class ShapePathModel;

class ShapeContent : public Content, public AnimationListener {
 public:
  ShapeContent(BaseLayer& layer, ShapePathModel& model);

  void OnValueChanged() override;

  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;
  Path* GetPath() override;
  void Init() override;

  bool SubPathType() override { return true; }

 private:
  std::unique_ptr<Path> path_;

  std::unique_ptr<ShapeKeyframeAnimation> shape_animation_;
  std::vector<ShapeModifierContent*> shape_modifier_contents_;

  bool is_path_valid_ = false;
  CompoundTrimPathContent trim_paths_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_SHAPE_SHAPE_CONTENT_H_
