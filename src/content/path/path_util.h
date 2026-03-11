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

#ifndef ANIMAX_SRC_CONTENT_PATH_PATH_UTIL_H_
#define ANIMAX_SRC_CONTENT_PATH_PATH_UTIL_H_

#include <memory>
#include <unordered_map>

#include "src/render/path.h"
#include "src/render/path_measure.h"

namespace lynx {
namespace animax {

class TrimPathContent;

// the conversion factor from degrees to radians
static constexpr float kDegreesToRadians = 0.017453292519943295;

// the value of the mathematical constant pi
static constexpr float kPI = 3.14159265358979323846;

static constexpr float kRadiansToDegrees = 180 / kPI;

class ANIMAX_EXPORT PathUtil {
 public:
  static void ApplyTrimPathIfNeeds(Path& path, TrimPathContent* trim_path);

  static void ApplyTrimPathIfNeeds(Path& path, float start_value,
                                   float end_value, float offset_value);

  static float ToRadians(float ang_deg);

  static float ToDegrees(float radians);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_PATH_PATH_UTIL_H_
