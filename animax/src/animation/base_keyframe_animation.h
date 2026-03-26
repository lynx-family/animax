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

#ifndef ANIMAX_SRC_ANIMATION_BASE_KEYFRAME_ANIMATION_H_
#define ANIMAX_SRC_ANIMATION_BASE_KEYFRAME_ANIMATION_H_

#include <map>

#include "src/animation/keyframe_animation.h"
#include "src/model/keyframe/path_keyframe_model.h"
#include "src/model/value/base_value.h"
#include "src/model/value/gradient_color.h"
#include "src/render/color_filter.h"
#include "src/render/path_measure.h"
#include "src/render/quaternion.h"

namespace lynx {
namespace animax {

class AnimationFactory {
 public:
  template <typename Animation, typename Value, typename... Args>
  static std::unique_ptr<Animation> Make(Args&&... args) {
    auto frames = std::make_shared<KeyframeModelList>();
    frames->push_back(std::make_unique<KeyframeModel>(
        ValueFactory::Make<Value>(std::forward<Args>(args)...)));
    return std::make_unique<Animation>(std::move(frames));
  }

  template <typename T>
  static bool HandleValueCallback(KeyframeAnimation* animation,
                                  KeyframeModel& keyframe, float progress,
                                  T* start_value, T* end_value,
                                  T& intermediate_value) {
    if (!animation || !animation->HasValueCallback() || !start_value ||
        !end_value) {
      return false;
    }
    auto callback_value = animation->GetValueFromCallback(
        start_value, start_value, end_value, progress, keyframe.GetStartFrame(),
        keyframe.GetEndFrame());

    if (callback_value) {
      if (auto* typed_value = reinterpret_cast<T*>(callback_value.get())) {
        intermediate_value = *typed_value;
        return true;
      }
    }
    return false;
  }
};

class ColorKeyframeAnimation : public KeyframeAnimation {
 public:
  ColorKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}
  Color& GetValue(KeyframeModel& keyframe, float progress) override;
  Color& GetValue() override;

  static std::unique_ptr<ColorKeyframeAnimation> MakeDefault();

 private:
  Color intermediate_;
};

class ColorFilterKeyframeAnimation : public KeyframeAnimation {
 public:
  ColorFilterKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}
  ColorFilter& GetValue(KeyframeModel& keyframe, float progress) override;
  ColorFilter& GetValue() override;

  static std::unique_ptr<ColorFilterKeyframeAnimation> MakeDefault();

 private:
  ColorFilter intermediate_;
};

class FloatKeyframeAnimation : public KeyframeAnimation {
 public:
  FloatKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}
  Float& GetValue(KeyframeModel& keyframe, float progress) override;
  Float& GetValue() override;

  static std::unique_ptr<FloatKeyframeAnimation> MakeDefault();

 private:
  Float intermediate_;
};

class IntegerKeyframeAnimation : public KeyframeAnimation {
 public:
  IntegerKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}
  Integer& GetValue(KeyframeModel& keyframe, float progress) override;
  Integer& GetValue() override;

  static std::unique_ptr<IntegerKeyframeAnimation> MakeDefault();

 private:
  Integer intermediate_;
};

class PointKeyframeAnimation : public KeyframeAnimation {
 public:
  PointKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}
  PointF& GetValue(KeyframeModel& keyframe, float progress) override;
  PointF& GetValue() override;
  PointF& GetValue(KeyframeModel& keyframe, float progress, float x_progress,
                   float y_progress) override;

  static std::unique_ptr<PointKeyframeAnimation> MakeDefault();

 protected:
  PointF& GetValueXY(KeyframeModel& keyframe, float linear_progress,
                     float x_progress, float y_progress);

 protected:
  PointF intermediate_;
};

class ScaleKeyframeAnimation : public KeyframeAnimation {
 public:
  ScaleKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}
  ScaleF& GetValue(KeyframeModel& keyframe, float progress) override;
  ScaleF& GetValue() override;

  static std::unique_ptr<ScaleKeyframeAnimation> MakeDefault();

 private:
  ScaleF intermediate_;
};

class PathKeyframeAnimation : public PointKeyframeAnimation {
 public:
  PathKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames,
                        bool enable_3d = false);
  PointF& GetValue(KeyframeModel& keyframe, float progress) override;
  PointF& GetValue() override;

  static std::unique_ptr<PathKeyframeAnimation> MakeDefault();
  void OnValueChanged() override;

 private:
  const bool enable_3d_ = false;
  mutable PathKeyframeModel* path_measure_keyframe_ = nullptr;
  std::unique_ptr<PathMeasure> path0_measure_, path1_measure_;
  bool path0_valid_ = false, path1_valid_ = false;
  PointF intermediate_;
};

class GradientKeyframeAnimation : public KeyframeAnimation {
 public:
  GradientKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames);
  GradientColor& GetValue(KeyframeModel& keyframe, float progress) override;
  GradientColor& GetValue() override;

  static std::unique_ptr<GradientKeyframeAnimation> MakeDefault();

 private:
  GradientColor intermediate_;
};

class OrientationKeyframeAnimation : public KeyframeAnimation {
 public:
  OrientationKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}
  Quaternion& GetValue(KeyframeModel& keyframe, float progress) override;
  Quaternion& GetValue() override;

 protected:
  Quaternion intermediate_;
  struct Quaternions {
    Quaternion start, end;
  };
  std::map<const KeyframeModel*, std::unique_ptr<Quaternions>> map_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATION_BASE_KEYFRAME_ANIMATION_H_
