// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/animax_loader_request_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoaderRequest_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoaderRequest_register_jni.h"
#include "src/base/util/android/java_only_map.h"

namespace lynx {
namespace animax {

namespace {
constexpr const char* kImageWidthKey = "image_width";
constexpr const char* kImageHeightKey = "image_height";
constexpr const char* kImageKey = "image";
constexpr const char* kSchemeKey = "scheme";
}  // namespace

base::android::ScopedLocalJavaRef<jobject> ResourceRequestToAnimaXLoaderRequest(
    JNIEnv* env, const ResourceRequest& resource_request) {
  const auto& uri = resource_request.uri_info.uri;
  auto j_uri = base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, uri);
  auto map = lynx::animax::android::JavaOnlyMap{};
  if (resource_request.type == ResourceRequestType::kLoadBitmap) {
    map.PushString(kImageKey, "");
    map.PushInt(kImageWidthKey, resource_request.width);
    map.PushInt(kImageHeightKey, resource_request.height);
    map.PushInt(kSchemeKey, static_cast<int>(resource_request.uri_info.scheme));
  }
  auto request = Java_AnimaXLoaderRequest_createRequestWithParams(
      env, j_uri.Get(), map.jni_object());
  return request;
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXLoaderRequest(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
