// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_VALUE_PARAM_ANDROID_H_
#define ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_VALUE_PARAM_ANDROID_H_

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/scoped_java_ref.h"
#include "include/property/animax_value_param.h"
#include "src/property/property_type.h"

namespace lynx {
namespace animax {

class AnimaXValueParamAndroid : public AnimaXValueParam {
 public:
  /**
   * Creates a AnimaXValueParamAndroid from a Java AnimaXValueParam
   * @param env JNI environment
   * @param value_param Java value parameter
   */
  explicit AnimaXValueParamAndroid(JNIEnv* env, jobject value_param);

  /**
   * Gets the Java object reference
   * @return The Java object reference
   */
  jobject GetJavaObject() const { return value_param_.Get(); }

  /**
   * Creates a Java AnimaXValueParam from a string value
   * @param env JNI environment
   * @param value String value
   * @return Java AnimaXValueParam object
   */
  static base::android::ScopedLocalJavaRef<jobject> CreateFromString(
      JNIEnv* env, const std::string& value);

  /**
   * Creates a Java AnimaXValueParam from a numeric value
   * @param env JNI environment
   * @param value Numeric value
   * @return Java AnimaXValueParam object
   */
  static base::android::ScopedLocalJavaRef<jobject> CreateFromNumber(
      JNIEnv* env, double value);

  /**
   * Creates a Java AnimaXValueParam from a boolean value
   * @param env JNI environment
   * @param value Boolean value
   * @return Java AnimaXValueParam object
   */
  static base::android::ScopedLocalJavaRef<jobject> CreateFromBoolean(
      JNIEnv* env, bool value);

  /**
   * Creates a Java AnimaXValueParam from coordinate coordinates
   * @param env JNI environment
   * @param x X coordinate
   * @param y Y coordinate
   * @param z Z coordinate
   * @return Java AnimaXValueParam object
   */
  static base::android::ScopedLocalJavaRef<jobject> CreateFromCoordinate(
      JNIEnv* env, double x, double y, double z);

  /**
   * Creates a Java AnimaXValueParam from a color value
   * @param env JNI environment
   * @param color Color integer value
   * @return Java AnimaXValueParam object
   */
  static base::android::ScopedLocalJavaRef<jobject> CreateFromColor(
      JNIEnv* env, int32_t color);

  /**
   * Creates a Java AnimaXValueParam from a color filter value
   * @param env JNI environment
   * @param color Color integer value
   * @param mode Filter mode value
   * @return Java AnimaXValueParam object
   */
  static base::android::ScopedLocalJavaRef<jobject> CreateFromColorFilter(
      JNIEnv* env, int32_t color, int32_t mode);

  /**
   * Creates a Java AnimaXValueParam from a Value* based on type
   * @param env JNI environment
   * @param value The Value to convert
   * @return Java AnimaXValueParam object or nullptr if conversion not possible
   */
  static base::android::ScopedLocalJavaRef<jobject> CreateFromValue(
      JNIEnv* env, const Value* value);

  /**
   * Converts a Java AnimaXValueParam to a native Value based on property type
   * @param env JNI environment
   * @param value_param Java value parameter to convert
   * @param property_type The property type (to determine how to interpret the
   * value)
   * @return The native Value object or nullptr if conversion not possible
   */
  static std::unique_ptr<Value> ValueParamToValue(
      JNIEnv* env, jobject value_param, LayerPropertyType property_type);

 private:
  base::android::ScopedGlobalJavaRef<jobject> value_param_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_VALUE_PARAM_ANDROID_H_
