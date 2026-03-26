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

#ifndef ANIMAX_SRC_CONTENT_CONTENT_H_
#define ANIMAX_SRC_CONTENT_CONTENT_H_

#include <string>
#include <vector>

#include "src/property/key_path_element.h"
#include "src/property/key_path_resolve_util.h"
#include "src/render/canvas.h"
#include "src/render/matrix.h"
#include "src/render/path.h"

namespace lynx {
namespace animax {

enum class ContentType : uint8_t {
  kUnknown = 0,
  kGreedy,
  kPath,
  kDrawing,
  kShapeModifier,
  kGroup,
  kTrimPath,
  kRoundCorner
};

class Content : public KeyPathElement {
 public:
  virtual ~Content() = default;

  virtual void SetContents(std::vector<Content*>& contents_before,
                           std::vector<Content*>& contents_after) = 0;

  virtual void Init() = 0;

  virtual const std::string& GetName() override { return name_; }

  virtual ContentType MainType() { return ContentType::kUnknown; }

  // drawing
  virtual void Draw(Canvas& canvas, Matrix& parent_matrix,
                    int32_t parent_alpha) {}

  virtual void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                         bool apply_parents) {}

  virtual bool SubDrawingType() { return false; }

  // greedy
  virtual void AbsorbContent(std::vector<std::unique_ptr<Content>>& contents) {}

  virtual bool SubGreedyType() { return false; }

  // path
  virtual Path* GetPath() { return nullptr; }

  virtual bool SubPathType() { return false; }

  void ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                      ResolvedKeyPathElements& match_elements,
                      const AnimaXKeyPath& current_partial_path) override {
    KeyPathResolveUtil::FullyResolveElement(*this, path, depth, match_elements,
                                            current_partial_path);
  }

 protected:
  std::string name_;
  bool hidden_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_CONTENT_H_
