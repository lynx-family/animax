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

#ifndef ANIMAX_SRC_MODEL_KEYFRAME_PATH_KEYFRAME_MODEL_H_
#define ANIMAX_SRC_MODEL_KEYFRAME_PATH_KEYFRAME_MODEL_H_

#include <memory>

#include "src/model/composition_model.h"
#include "src/model/keyframe/keyframe_model.h"
#include "src/model/value/base_value.h"
#include "src/render/path.h"

namespace lynx {
namespace animax {

constexpr uint8_t kPathPlaneCount = 2;
enum class PathPlane : uint8_t { kXY = 0, kYZ };

class PathKeyframeModel : public KeyframeModel {
 public:
  PathKeyframeModel(CompositionModel& composition,
                    KeyframeModel& point_keyframe);
  ~PathKeyframeModel() override = default;

  KeyframeType GetType() override { return KeyframeType::kPath; }

  template <PathPlane>
  Path* GetOrCreatePath();

  void OnValueChanged() override;

 private:
  void InitControlPoints(KeyframeModel& keyframe);

  template <PathPlane>
  void CreatePath(std::unique_ptr<Path>& path);

  std::unique_ptr<Path>& GetPath(PathPlane plane) {
    return path_[static_cast<int>(plane)];
  }
  std::unique_ptr<Path> path_[kPathPlaneCount];
  bool is_path_valid_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_KEYFRAME_PATH_KEYFRAME_MODEL_H_
