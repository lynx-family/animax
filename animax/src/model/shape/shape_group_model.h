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

#ifndef ANIMAX_SRC_MODEL_SHAPE_SHAPE_GROUP_MODEL_H_
#define ANIMAX_SRC_MODEL_SHAPE_SHAPE_GROUP_MODEL_H_

#include <memory>
#include <vector>

#include "src/model/content_model.h"

namespace lynx {
namespace animax {

class ShapeGroupModel : public ContentModel {
 public:
  ShapeGroupModel(){};
  ShapeGroupModel(std::string name,
                  std::vector<std::unique_ptr<ContentModel>>& items,
                  bool hidden) {
    name_ = std::move(name);
    hidden_ = hidden;
    for (auto& item : items) {
      items_.push_back(item.get());
    }
  };

  ~ShapeGroupModel() = default;

  std::vector<ContentModel*>& GetItems() { return items_; }

  void AddOwnItem(std::unique_ptr<ContentModel> item) {
    items_.push_back(item.get());
    own_items_.push_back(std::move(item));
  }

  std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                         BaseLayer& layer) override;

  ModelType Type() override { return ModelType::kShapeGroup; }

 private:
  std::vector<ContentModel*> items_;
  std::vector<std::unique_ptr<ContentModel>> own_items_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_SHAPE_SHAPE_GROUP_MODEL_H_
