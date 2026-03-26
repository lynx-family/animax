// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_VALUE_CALLBACK_ANDROID_H_
#define ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_VALUE_CALLBACK_ANDROID_H_

#include <jni.h>

#include <memory>

#include "base/include/platform/android/jni_utils.h"
#include "src/model/value/base_value.h"
#include "src/property/animax_frame_info.h"
#include "src/property/animax_value_callback.h"
#include "src/property/property_type.h"

namespace lynx {
namespace animax {

/**
 * JNI implementation of AnimaXValueCallback.
 * Bridges between Java callbacks and C++ animation system.
 */
class AnimaXValueCallbackAndroid : public AnimaXValueCallback {
 public:
  /**
   * Factory method to create the appropriate callback
   *
   * @param env JNI environment
   * @param java_callback The Java callback object
   * @param property_type The type of property this callback will modify
   * @return A unique_ptr to the created callback
   */
  static std::unique_ptr<AnimaXValueCallbackAndroid> Create(
      JNIEnv* env, jobject java_callback, LayerPropertyType property_type);

  /**
   * Constructor for JNI callback
   * @param env JNI environment
   * @param callback Java callback object
   * @param property_type The property type this callback is for
   */
  AnimaXValueCallbackAndroid(JNIEnv* env, jobject callback,
                             LayerPropertyType property_type);

  /**
   * Implementation of the GetValue method that calls back to Java
   */
  std::unique_ptr<Value> GetValue(const Value* original_value,
                                  const AnimaXFrameInfo& frame_info) override;

 private:
  base::android::ScopedGlobalJavaRef<jobject> callback_;
  LayerPropertyType property_type_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_VALUE_CALLBACK_ANDROID_H_
