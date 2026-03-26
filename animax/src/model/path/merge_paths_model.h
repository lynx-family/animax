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

#ifndef ANIMAX_SRC_MODEL_PATH_MERGE_PATHS_MODEL_H_
#define ANIMAX_SRC_MODEL_PATH_MERGE_PATHS_MODEL_H_

#include "src/content/path/merge_paths_content.h"
#include "src/model/content_model.h"

namespace lynx {
namespace animax {

enum class MergePathsMode : uint8_t {
  kMerge = 0,
  kAdd,
  kSubtract,
  kIntersect,
  kExcludeIntersections
};

class MergePathsModel : public ContentModel {
 public:
  MergePathsModel(std::string name, int32_t mode_int, bool hidden)
      : hidden_(hidden) {
    name_ = std::move(name);
    mode_ = static_cast<MergePathsMode>(mode_int - 1);
  }

  ~MergePathsModel() override {}

  std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                         BaseLayer& layer) override {
    return std::unique_ptr<MergePathsContent>(new MergePathsContent(*this));
  }

 private:
  friend class MergePathsContent;

  MergePathsMode mode_;
  bool hidden_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_PATH_MERGE_PATHS_MODEL_H_
