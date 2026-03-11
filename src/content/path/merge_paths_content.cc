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

#include "src/content/path/merge_paths_content.h"

#include "src/content/content_group.h"
#include "src/model/path/merge_paths_model.h"

namespace lynx {
namespace animax {

MergePathsContent::MergePathsContent(MergePathsModel& model)
    : first_path_(std::make_unique<Path>()),
      remainder_path_(std::make_unique<Path>()),
      path_(std::make_unique<Path>()),
      merge_paths_(model) {
  name_ = model.name_;
}

MergePathsContent::~MergePathsContent() = default;

Path* MergePathsContent::GetPath() {
  path_->Reset();

  if (merge_paths_.hidden_) {
    return path_.get();
  }

  switch (merge_paths_.mode_) {
    case MergePathsMode::kMerge:
      AddPaths();
      break;
    case MergePathsMode::kAdd:
      OpFirstPathWithRest(PathOp::kUnion);
      break;
    case MergePathsMode::kSubtract:
      OpFirstPathWithRest(PathOp::kReverseDifference);
      break;
    case MergePathsMode::kIntersect:
      OpFirstPathWithRest(PathOp::kIntersect);
      break;
    case MergePathsMode::kExcludeIntersections:
      OpFirstPathWithRest(PathOp::kXor);
      break;
  }

  return path_.get();
}

void MergePathsContent::SetContents(std::vector<Content*>& contents_before,
                                    std::vector<Content*>& contents_after) {
  for (auto& path_content : path_contents_) {
    path_content->SetContents(contents_before, contents_after);
  }
}

void MergePathsContent::AddPaths() {
  for (auto& content : path_contents_) {
    if (content->SubPathType()) {
      path_->AddPath(content->GetPath());
    }
  }
}

void MergePathsContent::OpFirstPathWithRest(PathOp op) {
  remainder_path_->Reset();
  first_path_->Reset();

  for (auto i = path_contents_.size() - 1; i >= 1; i--) {
    auto content = path_contents_[i].get();
    if (!content->SubPathType()) {
      continue;
    }

    if (content->MainType() == ContentType::kGroup) {
      auto content_group = static_cast<ContentGroup*>(content);
      std::vector<Content*> path_contents;
      content_group->GetPathList(path_contents);
      for (auto it = path_contents.rbegin(); it != path_contents.rend(); it++) {
        auto path = (*it)->GetPath();
        path->Transform(content_group->GetTransformationMatrix());
        remainder_path_->AddPath(path);
      }
    } else {
      remainder_path_->AddPath(content->GetPath());
    }
  }

  auto last_content = path_contents_[0].get();
  if (last_content->SubPathType()) {
    if (last_content->MainType() == ContentType::kGroup) {
      auto content_group = static_cast<ContentGroup*>(last_content);
      std::vector<Content*> path_contents;
      content_group->GetPathList(path_contents);
      for (auto& path_content : path_contents) {
        auto path = path_content->GetPath();
        path->Transform(content_group->GetTransformationMatrix());
        first_path_->AddPath(path);
      }
    } else {
      first_path_->Set(last_content->GetPath());
    }
  }

  if (!first_path_->IsEmpty() && !remainder_path_->IsEmpty()) {
    path_->Op(*first_path_, *remainder_path_, op);
  } else if (first_path_->IsEmpty()) {
    path_->AddPath(*remainder_path_);
  } else {
    path_->AddPath(*first_path_);
  }
}

void MergePathsContent::AbsorbContent(
    std::vector<std::unique_ptr<Content>>& contents) {
  bool find_content = false;
  for (auto it = contents.rbegin(); it != contents.rend(); it++) {
    auto& content = *it;
    if (!find_content && content.get() == this) {
      find_content = true;
      continue;
    }

    if (find_content) {
      if (content->SubPathType()) {
        path_contents_.push_back(std::move(content));
        contents.erase(std::next(it).base());
      }
    }
  }
}

}  // namespace animax
}  // namespace lynx
