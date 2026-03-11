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

#ifndef ANIMAX_SRC_MODEL_KEYFRAME_KEYFRAME_MODEL_H_
#define ANIMAX_SRC_MODEL_KEYFRAME_KEYFRAME_MODEL_H_

#include <memory>

#include "src/animation/interpolator/interpolator.h"
#include "src/base/log/log.h"
#include "src/model/composition_model.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

enum class KeyframeType : uint8_t { kPath = 0, kNormal };

class CompositionModel;

class KeyframeModel {
 public:
  KeyframeModel(std::unique_ptr<Value> value);

  // Create by single start value
  KeyframeModel(CompositionModel& composition, std::unique_ptr<Value> value);

  // Create by both start/end value
  KeyframeModel(CompositionModel& composition,
                std::unique_ptr<Value> start_value,
                std::unique_ptr<Value> end_value,
                std::unique_ptr<Interpolator> interpolator, float start_frame,
                float end_frame);

  // Create multi dimens interpolator
  KeyframeModel(CompositionModel& composition,
                std::unique_ptr<Value> start_value,
                std::unique_ptr<Value> end_value,
                std::unique_ptr<Interpolator> x_interpolator,
                std::unique_ptr<Interpolator> y_interpolator, float start_frame,
                float end_frame);

  virtual ~KeyframeModel() = default;

  KeyframeModel(const KeyframeModel&) = delete;
  KeyframeModel& operator=(const KeyframeModel&) = delete;

  long GetDurationMs();
  float GetStartFrame();
  float GetEndFrame();

  void SetStartFrame(float frame);
  void SetEndFrame(float frame);

  virtual void OnValueChanged() {}

  template <typename T>
  T* GetStartValue() {
    return start_value_ ? static_cast<T*>(start_value_.get()) : nullptr;
  }

  template <typename T>
  T* GetEndValue() {
    return end_value_ ? static_cast<T*>(end_value_.get()) : nullptr;
  }

  ValueType Type();

  std::unique_ptr<Value> CopyStartValue();
  std::unique_ptr<Value> CopyEndValue();

  void SetEndValue(std::unique_ptr<Value> end_value);

  void SetPathCps(std::unique_ptr<PointF> cp1, std::unique_ptr<PointF> cp2);

  bool IsPathCpNotEmpty();

  PointF* GetPathCp1();
  PointF* GetPathCp2();
  std::unique_ptr<Interpolator>& GetInterpolator();

  bool IsStartValueEmpty();
  bool IsEndValueEmpty();
  bool IsStatic() const;

  float GetStartProgress();
  float GetEndProgress();
  bool ContainsProgress(float progress);

  bool HasMultiDimenInterpolator();

  float GetProgress(float progress);
  float GetXProgress(float progress);
  float GetYProgress(float progress);

  virtual KeyframeType GetType() { return KeyframeType::kNormal; }

 private:
  CompositionModel* composition_;

  std::unique_ptr<Value> start_value_;
  std::unique_ptr<Value> end_value_;

  float start_frame_ = 0;
  float end_frame_ = Float::kMin;

  std::unique_ptr<PointF> path_cp1_;
  std::unique_ptr<PointF> path_cp2_;

  float start_progress_ = Float::kMin;
  float end_progress_ = Float::kMin;

  std::unique_ptr<Interpolator> interpolator_;
  std::unique_ptr<Interpolator> x_interpolator_;
  std::unique_ptr<Interpolator> y_interpolator_;
};

using KeyframeModelList = std::vector<std::unique_ptr<KeyframeModel>>;

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_KEYFRAME_KEYFRAME_MODEL_H_
