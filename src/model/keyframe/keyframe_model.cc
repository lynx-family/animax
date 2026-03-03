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

#include "src/model/keyframe/keyframe_model.h"

#include "src/base/log/log.h"
#include "src/model/composition_model.h"

namespace lynx {
namespace animax {

KeyframeModel::KeyframeModel(std::unique_ptr<Value> value)
    : composition_(nullptr),
      start_value_(value->Copy()),
      end_value_(std::move(value)),
      start_frame_(Float::kMin),
      end_frame_(Float::kMax) {}

KeyframeModel::KeyframeModel(CompositionModel& composition,
                             std::unique_ptr<Value> value)
    : composition_(&composition),
      start_value_(value->Copy()),
      end_value_(std::move(value)),
      start_frame_(Float::kMin),
      end_frame_(Float::kMax) {}

KeyframeModel::KeyframeModel(CompositionModel& composition,
                             std::unique_ptr<Value> start_value,
                             std::unique_ptr<Value> end_value,
                             std::unique_ptr<Interpolator> interpolator,
                             float start_frame, float end_frame)
    : composition_(&composition),
      start_value_(std::move(start_value)),
      end_value_(std::move(end_value)),
      start_frame_(start_frame),
      end_frame_(end_frame),
      interpolator_(std::move(interpolator)) {}

KeyframeModel::KeyframeModel(CompositionModel& composition,
                             std::unique_ptr<Value> start_value,
                             std::unique_ptr<Value> end_value,
                             std::unique_ptr<Interpolator> x_interpolator,
                             std::unique_ptr<Interpolator> y_interpolator,
                             float start_frame, float end_frame)
    : composition_(&composition),
      start_value_(std::move(start_value)),
      end_value_(std::move(end_value)),
      start_frame_(start_frame),
      end_frame_(end_frame),
      x_interpolator_(std::move(x_interpolator)),
      y_interpolator_(std::move(y_interpolator)) {}

float KeyframeModel::GetStartFrame() { return start_frame_; }
float KeyframeModel::GetEndFrame() { return end_frame_; }

void KeyframeModel::SetStartFrame(float frame) { start_frame_ = frame; }
void KeyframeModel::SetEndFrame(float frame) { end_frame_ = frame; }

ValueType KeyframeModel::Type() {
  return start_value_ ? start_value_->GetValueType() : ValueType::kUnknown;
}

std::unique_ptr<Value> KeyframeModel::CopyStartValue() {
  return start_value_ ? start_value_->Copy() : nullptr;
}

std::unique_ptr<Value> KeyframeModel::CopyEndValue() {
  return end_value_ ? end_value_->Copy() : nullptr;
}

void KeyframeModel::SetEndValue(std::unique_ptr<Value> end_value) {
  end_value_ = std::move(end_value);
}

void KeyframeModel::SetPathCps(std::unique_ptr<PointF> cp1,
                               std::unique_ptr<PointF> cp2) {
  path_cp1_ = std::move(cp1);
  path_cp2_ = std::move(cp2);
}

bool KeyframeModel::IsPathCpNotEmpty() {
  return path_cp1_ && !path_cp1_->IsEmpty() && path_cp2_ &&
         !path_cp2_->IsEmpty();
}

PointF* KeyframeModel::GetPathCp1() { return path_cp1_.get(); }
PointF* KeyframeModel::GetPathCp2() { return path_cp2_.get(); }
std::unique_ptr<Interpolator>& KeyframeModel::GetInterpolator() {
  return interpolator_;
}

bool KeyframeModel::IsStartValueEmpty() {
  return !start_value_ || start_value_->IsEmpty();
}

bool KeyframeModel::IsEndValueEmpty() {
  return !end_value_ || end_value_->IsEmpty();
}

bool KeyframeModel::IsStatic() const {
  return interpolator_ == nullptr && x_interpolator_ == nullptr &&
         y_interpolator_ == nullptr;
}

long KeyframeModel::GetDurationMs() {
  if (composition_) {
    return composition_->GetDuration();
  }
  return 0;
}

float KeyframeModel::GetStartProgress() {
  if (composition_ == nullptr || composition_->GetDurationFrames() == 0) {
    return 0;
  }
  if (start_progress_ == Float::kMin) {
    start_progress_ = (start_frame_ - composition_->GetStartFrame()) /
                      composition_->GetDurationFrames();
  }
  return start_progress_;
}

float KeyframeModel::GetEndProgress() {
  if (composition_ == nullptr || composition_->GetDurationFrames() == 0) {
    return 1;
  }
  if (end_progress_ == Float::kMin) {
    if (end_frame_ == Float::kMin) {
      end_progress_ = 1;
    } else {
      end_progress_ = (end_frame_ - composition_->GetStartFrame()) /
                      composition_->GetDurationFrames();
    }
  }
  return end_progress_;
}

bool KeyframeModel::ContainsProgress(float progress) {
  return progress >= GetStartProgress() && progress < GetEndProgress();
}

bool KeyframeModel::HasMultiDimenInterpolator() {
  return x_interpolator_ && y_interpolator_;
}

float KeyframeModel::GetProgress(float progress) {
  return interpolator_ ? interpolator_->GetInterpolation(progress) : 0;
}

float KeyframeModel::GetXProgress(float progress) {
  return x_interpolator_ ? x_interpolator_->GetInterpolation(progress) : 0;
}

float KeyframeModel::GetYProgress(float progress) {
  return y_interpolator_ ? y_interpolator_->GetInterpolation(progress) : 0;
}
}  // namespace animax
}  // namespace lynx
