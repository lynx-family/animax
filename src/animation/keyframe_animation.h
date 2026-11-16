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

#ifndef ANIMAX_SRC_ANIMATION_KEYFRAME_ANIMATION_H_
#define ANIMAX_SRC_ANIMATION_KEYFRAME_ANIMATION_H_

#include <memory>
#include <vector>

#include "src/base/log/log.h"
#include "src/model/keyframe/keyframe_model.h"
#include "src/model/value/base_value.h"
#include "src/property/animax_frame_info.h"
#include "src/property/animax_value_callback.h"

namespace lynx {
namespace animax {

class AnimationListener {
 public:
  virtual ~AnimationListener() = default;
  virtual void OnValueChanged() = 0;
};

class KeyframesWrapper {
 public:
  virtual ~KeyframesWrapper() = default;
  virtual bool IsEmpty() = 0;
  virtual bool IsValueChanged(float progress) = 0;
  virtual KeyframeModel* GetKeyframe(int32_t index) = 0;
  virtual int32_t GetSize() = 0;
  virtual KeyframeModel& GetCurrentKeyframe() = 0;
  virtual float GetStartDelayProgress() = 0;
  virtual float GetEndProgress() = 0;
  virtual bool IsCachedValueEnabled(float progress) = 0;
};

class SingleKeyframeWrapper : public KeyframesWrapper {
 public:
  SingleKeyframeWrapper(std::shared_ptr<KeyframeModelList> frames)
      : keyframes_(std::move(frames)) {}
  bool IsEmpty() override { return false; }
  bool IsValueChanged(float progress) override;
  int32_t GetSize() override { return 1; }
  KeyframeModel* GetKeyframe(int32_t index) override;
  KeyframeModel& GetCurrentKeyframe() override;
  float GetStartDelayProgress() override;
  float GetEndProgress() override;
  bool IsCachedValueEnabled(float progress) override;

 private:
  std::shared_ptr<KeyframeModelList> keyframes_;
  float cached_progress_ = -1;
};

class KeyframesWrapperImpl : public KeyframesWrapper {
 public:
  KeyframesWrapperImpl(std::shared_ptr<KeyframeModelList> frames);
  bool IsEmpty() override { return false; }
  bool IsValueChanged(float progress) override;
  int32_t GetSize() override;
  KeyframeModel* GetKeyframe(int32_t index) override;
  KeyframeModel& GetCurrentKeyframe() override;
  float GetStartDelayProgress() override;
  float GetEndProgress() override;
  bool IsCachedValueEnabled(float progress) override;

 private:
  KeyframeModel* FindKeyframe(float progress);

  std::shared_ptr<KeyframeModelList> keyframes_;
  KeyframeModel* cur_keyframe_ = nullptr;
  KeyframeModel* cached_cur_keyframe_ = nullptr;
  float cached_progress_ = 0;
};

class KeyframeAnimation {
 public:
  KeyframeAnimation(std::shared_ptr<KeyframeModelList> frames);

  virtual ~KeyframeAnimation() = default;

  virtual int32_t GetSize();

  virtual void SetProgress(float progress);

  virtual Value& GetValue(KeyframeModel& keyframe, float progress) {
    static Value empty_value;
    return empty_value;
  }

  virtual Value& GetValue(KeyframeModel& keyframe, float progress,
                          float x_progress, float y_progress) {
    static Value empty_value;
    return empty_value;
  }

  virtual Value& GetValue();

  KeyframeModel* GetKeyframe(int32_t index);

  bool CheckNullValue(KeyframeModel& keyframe) const;

  ValueType Type();

  KeyframeModel& GetCurrentKeyframe() const;

  float GetLinearCurrentKeyframeProgress() const;

  float GetCurrentKeyframeProgress() const;

  float GetCurrentProgress(KeyframeModel& keyframe) const;

  void AddUpdateListener(AnimationListener* listener);

  void NotifyListeners();

  void SetIsDiscrete();

  float GetProgress();

  /**
   * Sets a value callback for this animation.
   * The callback will be invoked during getValue() to allow runtime value
   * modifications.
   *
   * @param callback The callback to use
   */
  void SetValueCallback(std::shared_ptr<AnimaXValueCallback> callback) {
    value_callback_ = std::move(callback);
  }

  /**
   * Gets the value callback for this animation.
   *
   * @return The value callback or nullptr if none exists
   */
  std::shared_ptr<AnimaXValueCallback> GetValueCallback() const {
    return value_callback_;
  }

  /**
   * Gets the frame info for the current animation state.
   * This is used by value callbacks.
   *
   * @return The current animation frame info
   */
  AnimaXFrameInfo GetCurrentFrameInfo() const;

  /**
   * Gets a value from the callback if one exists.
   *
   * @param original_value The original value from interpolation
   * @param start_value The start value of the current keyframe
   * @param end_value The end value of the current keyframe
   * @param progress The current progress between start and end values
   * @param start_frame The start frame
   * @param end_frame The end frame
   * @return Value from the callback, or nullptr if no callback exists or
   * returns null
   */
  std::unique_ptr<Value> GetValueFromCallback(const Value* original_value,
                                              const Value* start_value,
                                              const Value* end_value,
                                              float progress, float start_frame,
                                              float end_frame);
  /**
   * Checks if this animation has a value callback.
   *
   * @return true if a value callback is set
   */
  bool HasValueCallback() const { return value_callback_ != nullptr; }

  virtual void OnValueChanged() {}

 protected:
  float progress_ = 0;
  bool is_discrete_ = false;
  std::unique_ptr<KeyframesWrapper> wrapper_;
  std::vector<AnimationListener*> listeners_;
  std::shared_ptr<AnimaXValueCallback> value_callback_;

 private:
  float GetStartDelayProgress();
  float GetEndProgress();

  float cached_start_delay_progress_ = -1;
  float cached_end_progress_ = -1;
};

class AnimationHost {
 public:
  virtual ~AnimationHost() = default;
  void AddAnimation(KeyframeAnimation* animation);
  const std::vector<KeyframeAnimation*>& GetAnimations() const {
    return animations_;
  }

 protected:
  std::vector<KeyframeAnimation*> animations_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATION_KEYFRAME_ANIMATION_H_
