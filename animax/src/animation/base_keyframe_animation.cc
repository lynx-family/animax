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

#include "src/animation/base_keyframe_animation.h"

#include <utility>

#include "src/base/util/misc_util.h"

namespace lynx {
namespace animax {

// Static MakeDefault methods implementation
std::unique_ptr<ColorKeyframeAnimation> ColorKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<ColorKeyframeAnimation, Color>(0);
}

std::unique_ptr<ColorFilterKeyframeAnimation>
ColorFilterKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<ColorFilterKeyframeAnimation, ColorFilter>(0,
                                                                           3);
}

std::unique_ptr<FloatKeyframeAnimation> FloatKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<FloatKeyframeAnimation, Float>(0);
}

std::unique_ptr<IntegerKeyframeAnimation>
IntegerKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<IntegerKeyframeAnimation, Integer>(0);
}

std::unique_ptr<PointKeyframeAnimation> PointKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<PointKeyframeAnimation, PointF>(0, 0, 0);
}

std::unique_ptr<ScaleKeyframeAnimation> ScaleKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<ScaleKeyframeAnimation, ScaleF>(1, 1, 1);
}

std::unique_ptr<PathKeyframeAnimation> PathKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<PathKeyframeAnimation, PointF>(0, 0, 0);
}

std::unique_ptr<GradientKeyframeAnimation>
GradientKeyframeAnimation::MakeDefault() {
  return AnimationFactory::Make<GradientKeyframeAnimation, GradientColor>(0);
}

Color& ColorKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                        float progress) {
  intermediate_.Reset();
  if (KeyframeAnimation::CheckNullValue(keyframe)) {
    return intermediate_;
  }

  auto* start_value = keyframe.GetStartValue<Color>();
  auto* end_value = keyframe.GetEndValue<Color>();
  if (AnimationFactory::HandleValueCallback(
          this, keyframe, progress, start_value, end_value, intermediate_)) {
    return intermediate_;
  }
  if (start_value && end_value) {
    GammaEvaluate(*start_value, *end_value, progress, &intermediate_);
  }

  return intermediate_;
}

Color& ColorKeyframeAnimation::GetValue() {
  return static_cast<Color&>(KeyframeAnimation::GetValue());
}

Float& FloatKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                        float progress) {
  intermediate_.Reset();
  if (KeyframeAnimation::CheckNullValue(keyframe)) {
    return intermediate_;
  }

  auto* start_value = keyframe.GetStartValue<Float>();
  auto* end_value = keyframe.GetEndValue<Float>();
  if (AnimationFactory::HandleValueCallback(
          this, keyframe, progress, start_value, end_value, intermediate_)) {
    return intermediate_;
  }
  if (start_value && end_value) {
    Lerp(*start_value, *end_value, progress, &intermediate_);
  }

  return intermediate_;
}

Float& FloatKeyframeAnimation::GetValue() {
  return static_cast<Float&>(KeyframeAnimation::GetValue());
}

Integer& IntegerKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                            float progress) {
  intermediate_.Reset();
  if (KeyframeAnimation::CheckNullValue(keyframe)) {
    return intermediate_;
  }

  auto* start_value = keyframe.GetStartValue<Integer>();
  auto* end_value = keyframe.GetEndValue<Integer>();
  if (AnimationFactory::HandleValueCallback(
          this, keyframe, progress, start_value, end_value, intermediate_)) {
    return intermediate_;
  }
  if (start_value && end_value) {
    Lerp(*start_value, *end_value, progress, &intermediate_);
  }

  return intermediate_;
}

Integer& IntegerKeyframeAnimation::GetValue() {
  return static_cast<Integer&>(KeyframeAnimation::GetValue());
}

PointF& PointKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                         float progress) {
  return PointKeyframeAnimation::GetValueXY(keyframe, progress, progress,
                                            progress);
}

PointF& PointKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                         float progress, float x_progress,
                                         float y_progress) {
  return PointKeyframeAnimation::GetValueXY(keyframe, progress, x_progress,
                                            y_progress);
}

PointF& PointKeyframeAnimation::GetValueXY(KeyframeModel& keyframe,
                                           float linear_progress,
                                           float x_progress, float y_progress) {
  intermediate_.Reset();
  if (KeyframeAnimation::CheckNullValue(keyframe)) {
    return intermediate_;
  }

  auto* start_value = keyframe.GetStartValue<PointF>();
  auto* end_value = keyframe.GetEndValue<PointF>();
  if (AnimationFactory::HandleValueCallback(this, keyframe, linear_progress,
                                            start_value, end_value,
                                            intermediate_)) {
    return intermediate_;
  }
  if (start_value && end_value) {
    intermediate_.Set(
        Lerp(start_value->GetX(), end_value->GetX(), x_progress),
        Lerp(start_value->GetY(), end_value->GetY(), y_progress),
        Lerp(start_value->GetZ(), end_value->GetZ(), linear_progress));
  }

  return intermediate_;
}

PointF& PointKeyframeAnimation::GetValue() {
  return static_cast<PointF&>(KeyframeAnimation::GetValue());
}

ScaleF& ScaleKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                         float progress) {
  intermediate_.Reset();
  if (KeyframeAnimation::CheckNullValue(keyframe)) {
    return intermediate_;
  }

  auto* start_value = keyframe.GetStartValue<ScaleF>();
  auto* end_value = keyframe.GetEndValue<ScaleF>();
  if (AnimationFactory::HandleValueCallback(
          this, keyframe, progress, start_value, end_value, intermediate_)) {
    return intermediate_;
  }
  if (start_value && end_value) {
    intermediate_.Set(Lerp(start_value->GetX(), end_value->GetX(), progress),
                      Lerp(start_value->GetY(), end_value->GetY(), progress),
                      Lerp(start_value->GetZ(), end_value->GetZ(), progress));
  }

  return intermediate_;
}

ScaleF& ScaleKeyframeAnimation::GetValue() {
  return static_cast<ScaleF&>(KeyframeAnimation::GetValue());
}

PathKeyframeAnimation::PathKeyframeAnimation(
    std::shared_ptr<KeyframeModelList> frames, bool enable_3d)
    : PointKeyframeAnimation(std::move(frames)),
      enable_3d_(enable_3d),
      path0_measure_(std::make_unique<PathMeasure>()) {
  if (enable_3d_) {
    path1_measure_ = std::make_unique<PathMeasure>();
  }
}

PointF& PathKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                        float progress) {
  auto* start_value = keyframe.GetStartValue<PointF>();
  if (start_value) {
    intermediate_ = *start_value;
  } else {
    intermediate_.Reset();
  }

  if (keyframe.GetType() != KeyframeType::kPath) {
    return intermediate_;
  }

  auto path_keyframe = static_cast<PathKeyframeModel*>(&keyframe);
  if (path_measure_keyframe_ != path_keyframe) {
    path_measure_keyframe_ = path_keyframe;
    path0_valid_ = path1_valid_ = false;
    auto path0 = path_keyframe->GetOrCreatePath<PathPlane::kXY>();
    if (path0) {
      path0_measure_->SetPath(*path0, false);
      path0_valid_ = path0_measure_->GetLength() > 0;
    }
    if (enable_3d_) {
      auto path1 = path_keyframe->GetOrCreatePath<PathPlane::kYZ>();
      if (path1) {
        path1_measure_->SetPath(*path1, false);
        path1_valid_ = path1_measure_->GetLength() > 0;
      }
    }
  }

  PointF out;
  if (enable_3d_ && path1_valid_) {
    if (path1_measure_->GetPosTan(progress * path1_measure_->GetLength(),
                                  &out)) {
      // for PathPlane::kYZ, Y and Z will be saved in first and second dimension
      intermediate_.Set(intermediate_.GetX(), out.GetX(), out.GetY());
    }
  }
  if (path0_valid_ &&
      path0_measure_->GetPosTan(progress * path0_measure_->GetLength(), &out)) {
    intermediate_.Set(out.GetX(), out.GetY(), intermediate_.GetZ());
  }
  return intermediate_;
}

PointF& PathKeyframeAnimation::GetValue() {
  return static_cast<PointF&>(KeyframeAnimation::GetValue());
}

void PathKeyframeAnimation::OnValueChanged() {
  path_measure_keyframe_ = nullptr;
}

GradientKeyframeAnimation::GradientKeyframeAnimation(
    std::shared_ptr<KeyframeModelList> frames)
    : KeyframeAnimation(std::move(frames)) {
  auto size = 0;
  if (GetSize() > 0 && !GetKeyframe(0)->IsStartValueEmpty()) {
    auto* start_value = GetKeyframe(0)->GetStartValue<GradientColor>();
    if (start_value) {
      size = start_value->GetSize();
    }
  }
  intermediate_.Init(size);
}

GradientColor& GradientKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                                   float progress) {
  auto* start_value = keyframe.GetStartValue<GradientColor>();
  auto* end_value = keyframe.GetEndValue<GradientColor>();
  if (start_value && end_value) {
    intermediate_.LerpColor(*start_value, *end_value, progress);
  }

  return intermediate_;
}

GradientColor& GradientKeyframeAnimation::GetValue() {
  return static_cast<GradientColor&>(KeyframeAnimation::GetValue());
}

Quaternion& OrientationKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                                   float progress) {
  intermediate_.Reset();
  if (KeyframeAnimation::CheckNullValue(keyframe)) {
    return intermediate_;
  }

  auto* start_value = keyframe.GetStartValue<Orientation>();
  auto* end_value = keyframe.GetEndValue<Orientation>();
  if (!start_value || !end_value) {
    return intermediate_;
  }

  auto& ref = map_[&keyframe];
  if (ref == nullptr) {
    ref = std::make_unique<Quaternions>(Quaternions{
        .start = {start_value->GetX(), start_value->GetY(),
                  start_value->GetZ()},
        .end = {end_value->GetX(), end_value->GetY(), end_value->GetZ()}});
    ref->end.AutoAdjustEndForInterpolation(ref->start);
  }
  ref->start.SphericalLinearInterpolation(ref->end, progress, intermediate_);
  return intermediate_;
}

Quaternion& OrientationKeyframeAnimation::GetValue() {
  return static_cast<Quaternion&>(KeyframeAnimation::GetValue());
}

ColorFilter& ColorFilterKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                                    float progress) {
  if (KeyframeAnimation::CheckNullValue(keyframe)) {
    return intermediate_;
  }
  auto* start_value = keyframe.GetStartValue<ColorFilter>();
  auto* end_value = keyframe.GetEndValue<ColorFilter>();
  if (AnimationFactory::HandleValueCallback(
          this, keyframe, progress, start_value, end_value, intermediate_)) {
    return intermediate_;
  }
  if (start_value && end_value) {
    auto color =
        GammaEvaluate(start_value->GetColor(), end_value->GetColor(), progress);
    intermediate_.Set(color, intermediate_.GetMode());
  }
  return intermediate_;
}

ColorFilter& ColorFilterKeyframeAnimation::GetValue() {
  return static_cast<ColorFilter&>(KeyframeAnimation::GetValue());
}

}  // namespace animax
}  // namespace lynx
