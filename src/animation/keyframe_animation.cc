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

#include "src/animation/keyframe_animation.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

bool SingleKeyframeWrapper::IsValueChanged(float progress) {
  return !keyframes_->front()->IsStatic();
}

KeyframeModel* SingleKeyframeWrapper::GetKeyframe(int32_t index) {
  return index == 0 ? keyframes_->front().get() : nullptr;
}

KeyframeModel& SingleKeyframeWrapper::GetCurrentKeyframe() {
  return *keyframes_->front();
}

float SingleKeyframeWrapper::GetStartDelayProgress() {
  return keyframes_->front()->GetStartProgress();
}

float SingleKeyframeWrapper::GetEndProgress() {
  return keyframes_->front()->GetEndProgress();
}

bool SingleKeyframeWrapper::IsCachedValueEnabled(float progress) {
  if (cached_progress_ == progress) {
    return true;
  }
  cached_progress_ = progress;
  return false;
}

KeyframesWrapperImpl::KeyframesWrapperImpl(
    std::shared_ptr<KeyframeModelList> frames)
    : keyframes_(std::move(frames)) {
  DCHECK(keyframes_);
  cur_keyframe_ = FindKeyframe(0);
}

bool KeyframesWrapperImpl::IsValueChanged(float progress) {
  if (cur_keyframe_->ContainsProgress(progress)) {
    return !cur_keyframe_->IsStatic();
  }
  cur_keyframe_ = FindKeyframe(progress);
  return true;
}

int32_t KeyframesWrapperImpl::GetSize() { return keyframes_->size(); }

KeyframeModel* KeyframesWrapperImpl::GetKeyframe(int32_t index) {
  return (index >= 0 && index < keyframes_->size()) ? (*keyframes_)[index].get()
                                                    : nullptr;
}

KeyframeModel& KeyframesWrapperImpl::GetCurrentKeyframe() {
  return *cur_keyframe_;
}

float KeyframesWrapperImpl::GetStartDelayProgress() {
  return (*keyframes_)[0]->GetStartProgress();
}

float KeyframesWrapperImpl::GetEndProgress() {
  return (*keyframes_)[keyframes_->size() - 1]->GetEndProgress();
}

bool KeyframesWrapperImpl::IsCachedValueEnabled(float progress) {
  if (cached_cur_keyframe_ == cur_keyframe_ && cached_progress_ == progress) {
    return true;
  }
  cached_cur_keyframe_ = cur_keyframe_;
  cached_progress_ = progress;
  return false;
}

KeyframeModel* KeyframesWrapperImpl::FindKeyframe(float progress) {
  auto& keyframe = (*keyframes_)[keyframes_->size() - 1];
  if (progress >= keyframe->GetStartProgress()) {
    return keyframe.get();
  }

  if (keyframes_->size() <= 2) {
    return (*keyframes_)[0].get();
  }

  for (auto i = keyframes_->size() - 2; i >= 1; i--) {
    auto& cur_keyframe = (*keyframes_)[i];
    if (cur_keyframe_ == cur_keyframe.get()) {
      continue;
    }
    if (cur_keyframe->ContainsProgress(progress)) {
      return cur_keyframe.get();
    }
  }

  return (*keyframes_)[0].get();
}

KeyframeAnimation::KeyframeAnimation(
    std::shared_ptr<KeyframeModelList> frames) {
  DCHECK(frames);
  if (frames->empty()) {
    ANIMAX_LOGI("frames cannot be empty!");
  } else if (frames->size() == 1) {
    wrapper_ = std::unique_ptr<SingleKeyframeWrapper>(
        new SingleKeyframeWrapper(std::move(frames)));
  } else {
    wrapper_ = std::unique_ptr<KeyframesWrapperImpl>(
        new KeyframesWrapperImpl(std::move(frames)));
  }
}

ValueType KeyframeAnimation::Type() {
  auto* frame = wrapper_->GetKeyframe(0);
  return frame ? frame->Type() : ValueType::kUnknown;
}

int32_t KeyframeAnimation::GetSize() {
  return wrapper_ ? wrapper_->GetSize() : 0;
}

KeyframeModel* KeyframeAnimation::GetKeyframe(int32_t index) {
  return wrapper_ ? wrapper_->GetKeyframe(index) : nullptr;
}

bool KeyframeAnimation::CheckNullValue(KeyframeModel& keyframe) const {
  if (keyframe.IsStartValueEmpty() || keyframe.IsEndValueEmpty()) {
    ANIMAX_LOGI("KeyframeAnimation start_value or end_value is null");
    return true;
  }
  return false;
}

Value& KeyframeAnimation::GetValue() {
  float progress = GetLinearCurrentKeyframeProgress();
  // TODO(aiyongbiao.rick): Optimize get performance by cached value.

  auto& keyframe = GetCurrentKeyframe();

  if (keyframe.HasMultiDimenInterpolator()) {
    auto x_progress = keyframe.GetXProgress(progress);
    auto y_progress = keyframe.GetYProgress(progress);
    return GetValue(keyframe, progress, x_progress, y_progress);
  } else {
    if (keyframe.IsStatic()) {
      progress = 0;
    } else {
      progress = keyframe.GetProgress(progress);
    }
    return GetValue(keyframe, progress);
  }
}

KeyframeModel& KeyframeAnimation::GetCurrentKeyframe() const {
  return wrapper_->GetCurrentKeyframe();
}

float KeyframeAnimation::GetLinearCurrentKeyframeProgress() const {
  if (is_discrete_) {
    return 0;
  }

  auto& keyframe = GetCurrentKeyframe();
  if (keyframe.IsStatic()) {
    return 0;
  }

  return GetCurrentProgress(keyframe);
}

float KeyframeAnimation::GetCurrentKeyframeProgress() const {
  auto& keyframe = GetCurrentKeyframe();
  return GetCurrentProgress(keyframe);
}

float KeyframeAnimation::GetCurrentProgress(KeyframeModel& keyframe) const {
  auto progress_into_frame = progress_ - keyframe.GetStartProgress();
  auto keyframe_progress =
      keyframe.GetEndProgress() - keyframe.GetStartProgress();
  if (keyframe_progress == 0) {
    return 0;
  }
  return progress_into_frame / keyframe_progress;
}

void KeyframeAnimation::AddUpdateListener(AnimationListener* listener) {
  listeners_.push_back(listener);
}

void KeyframeAnimation::SetProgress(float progress) {
  if (wrapper_ == nullptr) {
    return;
  }

  float target_progress = progress;
  if (target_progress < GetStartDelayProgress()) {
    target_progress = GetStartDelayProgress();
  } else if (target_progress > GetEndProgress()) {
    target_progress = GetEndProgress();
  }

  if (target_progress == progress_) {
    return;
  }

  progress_ = target_progress;
  if (value_callback_ || wrapper_->IsValueChanged(progress_)) {
    NotifyListeners();
  }
}

void KeyframeAnimation::NotifyListeners() {
  for (auto& listener : listeners_) {
    listener->OnValueChanged();
  }
}

void KeyframeAnimation::SetIsDiscrete() { is_discrete_ = true; }

float KeyframeAnimation::GetProgress() { return progress_; }

float KeyframeAnimation::GetStartDelayProgress() {
  if (cached_start_delay_progress_ == -1) {
    cached_start_delay_progress_ = wrapper_->GetStartDelayProgress();
  }
  return cached_start_delay_progress_;
}

float KeyframeAnimation::GetEndProgress() {
  if (cached_end_progress_ == -1) {
    cached_end_progress_ = wrapper_->GetEndProgress();
  }
  return cached_end_progress_;
}

AnimaXFrameInfo KeyframeAnimation::GetCurrentFrameInfo() const {
  float start_frame = 0.0f;
  float end_frame = 1.0f;
  if (wrapper_ && wrapper_->GetSize() > 0) {
    auto* first_keyframe = wrapper_->GetKeyframe(0);
    auto* last_keyframe = wrapper_->GetKeyframe(wrapper_->GetSize() - 1);
    if (first_keyframe && last_keyframe) {
      start_frame = first_keyframe->GetStartFrame();
      end_frame = last_keyframe->GetEndFrame();
    }
  }

  auto& keyframe = GetCurrentKeyframe();
  float current_frame = keyframe.GetStartFrame() +
                        (keyframe.GetEndFrame() - keyframe.GetStartFrame()) *
                            GetLinearCurrentKeyframeProgress();

  return AnimaXFrameInfo(start_frame, end_frame, current_frame,
                         GetLinearCurrentKeyframeProgress(),
                         GetCurrentKeyframeProgress(), progress_);
}

std::unique_ptr<Value> KeyframeAnimation::GetValueFromCallback(
    const Value* original_value, const Value* start_value,
    const Value* end_value, float progress, float start_frame,
    float end_frame) {
  if (!value_callback_) {
    return nullptr;  // No callback, use original value
  }

  // Create frame info for the callback
  AnimaXFrameInfo frame_info(
      start_frame, end_frame,
      start_frame + (end_frame - start_frame) * progress, start_value,
      end_value,
      progress,                            // Linear progress
      GetLinearCurrentKeyframeProgress(),  // Currently using same value for
                                           // interpolated progress
      GetProgress());

  // Get value from callback
  return value_callback_->GetValue(original_value, frame_info);
}

void AnimationHost::AddAnimation(KeyframeAnimation* animation) {
  if (animation) {
    animations_.push_back(animation);
  }
}

}  // namespace animax
}  // namespace lynx
