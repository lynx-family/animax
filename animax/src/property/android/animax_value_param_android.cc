// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/android/animax_value_param_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXValueParam_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXValueParam_register_jni.h"
#include "src/model/value/base_value.h"
#include "src/property/property_type.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXValueParam(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

AnimaXValueParamAndroid::AnimaXValueParamAndroid(JNIEnv* env,
                                                 jobject value_param)
    : AnimaXValueParam([&]() -> AnimaXValueParam {
        // Get target frame if exists
        auto target_frame =
            Java_AnimaXValueParam_getTargetFrame(env, value_param);
        // Convert based on type
        Type type = static_cast<Type>(
            Java_AnimaXValueParam_getTypeIndex(env, value_param));
        switch (type) {
          case Type::kString: {
            auto string_value =
                Java_AnimaXValueParam_getStringValue(env, value_param);
            if (!string_value.IsNull()) {
              std::string value =
                  base::android::JNIConvertHelper::ConvertToString(
                      env, string_value.Get());
              return AnimaXValueParam(value, target_frame);
            }
            break;
          }
          case Type::kNumber: {
            auto number_value =
                Java_AnimaXValueParam_getNumberValue(env, value_param);
            return AnimaXValueParam(number_value, target_frame);
          }
          case Type::kBoolean: {
            auto bool_value =
                Java_AnimaXValueParam_getBooleanValue(env, value_param);
            return AnimaXValueParam(static_cast<bool>(bool_value),
                                    target_frame);
          }
          case Type::kCoordinate: {
            float x = Java_AnimaXValueParam_getX(env, value_param);
            float y = Java_AnimaXValueParam_getY(env, value_param);
            float z = Java_AnimaXValueParam_getZ(env, value_param);
            return AnimaXValueParam(x, y, z, target_frame);
          }
          case Type::kColor: {
            int32_t color =
                Java_AnimaXValueParam_getColorValue(env, value_param);
            return AnimaXValueParam(color, target_frame);
          }
          case Type::kColorFilter: {
            int32_t color =
                Java_AnimaXValueParam_getColorValue(env, value_param);
            int32_t mode =
                Java_AnimaXValueParam_getFilterMode(env, value_param);
            AnimaXValueParam::ColorFilterParam params{color, mode};
            return AnimaXValueParam(params, target_frame);
          }
          case Type::kNull:
          default:
            return AnimaXValueParam();
        }
        return AnimaXValueParam();
      }()),
      value_param_(env, value_param) {}

base::android::ScopedLocalJavaRef<jobject>
AnimaXValueParamAndroid::CreateFromString(JNIEnv* env,
                                          const std::string& value) {
  auto jstr =
      base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, value);
  auto result = Java_AnimaXValueParam_fromString(env, jstr.Get());
  return result;
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXValueParamAndroid::CreateFromNumber(JNIEnv* env, double value) {
  return Java_AnimaXValueParam_fromNumber(env, value);
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXValueParamAndroid::CreateFromBoolean(JNIEnv* env, bool value) {
  return Java_AnimaXValueParam_fromBoolean(env, static_cast<jboolean>(value));
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXValueParamAndroid::CreateFromCoordinate(JNIEnv* env, double x, double y,
                                              double z) {
  return Java_AnimaXValueParam_fromCoordinate(env, x, y, z);
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXValueParamAndroid::CreateFromColor(JNIEnv* env, int32_t color) {
  return Java_AnimaXValueParam_fromColor(env, color);
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXValueParamAndroid::CreateFromColorFilter(JNIEnv* env, int32_t color,
                                               int32_t mode) {
  return Java_AnimaXValueParam_fromColorFilter(env, color, mode);
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXValueParamAndroid::CreateFromValue(JNIEnv* env, const Value* value) {
  if (!value) {
    return base::android::ScopedLocalJavaRef<jobject>();
  }

  // Use the base class conversion to create an AnimaXValueParam from Value
  auto value_param = AnimaXValueParam::FromValue(value);
  if (!value_param) {
    return base::android::ScopedLocalJavaRef<jobject>();
  }

  // Then convert the AnimaXValueParam to Java object based on its type
  switch (value_param->GetType()) {
    case AnimaXValueParam::Type::kString:
      return CreateFromString(env, value_param->GetStringValue());
    case AnimaXValueParam::Type::kNumber:
      return CreateFromNumber(env, value_param->GetNumberValue());
    case AnimaXValueParam::Type::kBoolean:
      return CreateFromBoolean(env, value_param->GetBooleanValue());
    case AnimaXValueParam::Type::kCoordinate:
      return CreateFromCoordinate(env, value_param->GetX(), value_param->GetY(),
                                  value_param->GetZ());
    case AnimaXValueParam::Type::kColor:
      return CreateFromColor(env, value_param->GetColorValue());
    case AnimaXValueParam::Type::kColorFilter:
      return CreateFromColorFilter(env, value_param->GetColorValue(),
                                   value_param->GetNumberValue());
    case AnimaXValueParam::Type::kNull:
    default:
      return base::android::ScopedLocalJavaRef<jobject>();
  }
}

std::unique_ptr<Value> AnimaXValueParamAndroid::ValueParamToValue(
    JNIEnv* env, jobject value_param, LayerPropertyType property_type) {
  if (!value_param) {
    return nullptr;
  }

  // Use AnimaXValueParamAndroid to extract values from the Java object
  AnimaXValueParamAndroid param(env, value_param);

  // Use the base class conversion to create a Value from AnimaXValueParam
  return param.ToValue(property_type);
}

}  // namespace animax
}  // namespace lynx
