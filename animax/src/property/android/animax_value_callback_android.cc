// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/android/animax_value_callback_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXValueCallback_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXValueCallback_register_jni.h"
#include "src/property/android/animax_value_param_android.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXValueCallback(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

AnimaXValueCallbackAndroid::AnimaXValueCallbackAndroid(
    JNIEnv* env, jobject callback, LayerPropertyType property_type)
    : callback_(env, callback), property_type_(property_type) {}

std::unique_ptr<AnimaXValueCallbackAndroid> AnimaXValueCallbackAndroid::Create(
    JNIEnv* env, jobject java_callback, LayerPropertyType property_type) {
  // Skip creating anything if no callback provided
  if (!java_callback) {
    return nullptr;
  }

  return std::make_unique<AnimaXValueCallbackAndroid>(env, java_callback,
                                                      property_type);
}

std::unique_ptr<Value> AnimaXValueCallbackAndroid::GetValue(
    const Value* original_value, const AnimaXFrameInfo& frame_info) {
  JNIEnv* env = base::android::AttachCurrentThread();

  const auto* start_value = frame_info.GetStartValue();
  auto start_param_ref = AnimaXValueParamAndroid::CreateFromValue(
      env, start_value ? start_value : original_value);

  const auto* end_value = frame_info.GetEndValue();
  auto end_param_ref = AnimaXValueParamAndroid::CreateFromValue(
      env, end_value ? end_value : original_value);

  auto result = Java_AnimaXValueCallback_getValueInternal(
      env, callback_.Get(), frame_info.GetStartFrame(),
      frame_info.GetEndFrame(), start_param_ref.Get(), end_param_ref.Get(),
      frame_info.GetLinearProgress(), frame_info.GetInterpolatedProgress(),
      frame_info.GetOverallProgress());

  if (result.IsNull()) {
    return nullptr;
  }

  return AnimaXValueParamAndroid::ValueParamToValue(env, result.Get(),
                                                    property_type_);
}

}  // namespace animax
}  // namespace lynx
