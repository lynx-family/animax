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

#ifndef ANIMAX_SRC_CONTENT_PATH_MERGE_PATHS_CONTENT_H_
#define ANIMAX_SRC_CONTENT_PATH_MERGE_PATHS_CONTENT_H_

#include "src/animation/keyframe_animation.h"
#include "src/content/content.h"

namespace lynx {
namespace animax {

class MergePathsModel;

class MergePathsContent : public Content {
 public:
  MergePathsContent(MergePathsModel& model);

  ~MergePathsContent() override;

  void Init() override {}

  Path* GetPath() override;
  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;
  void AbsorbContent(std::vector<std::unique_ptr<Content>>& contents) override;

  void AddPaths();
  void OpFirstPathWithRest(PathOp op);

  bool SubPathType() override { return true; }
  bool SubGreedyType() override { return true; }

 private:
  std::unique_ptr<Path> first_path_;
  std::unique_ptr<Path> remainder_path_;
  std::unique_ptr<Path> path_;

  std::vector<std::unique_ptr<Content>> path_contents_;
  MergePathsModel& merge_paths_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_PATH_MERGE_PATHS_CONTENT_H_
