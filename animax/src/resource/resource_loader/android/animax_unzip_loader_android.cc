// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/resource_loader/android/animax_unzip_loader_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXUnzipLoader_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXUnzipLoader_register_jni.h"
#include "src/resource/resource_loader.h"

using lynx::base::android::JNIConvertHelper;

namespace lynx {
namespace animax {

void AnimaxUnzipLoaderAndroid::Load(
    UnzipRequest request, AnimaxUnzipLoaderAndroid::CallbackType callback) {
  JNIEnv* env = base::android::AttachCurrentThread();

  if (request.unzip_folder_path.empty()) {
    std::string cache_dir_path = GetCacheDirectory();
    if (cache_dir_path.empty()) {
      callback(UnzipResponse{}, LoaderError{.code = kUnzipFailed});
      return;
    }
    request.unzip_folder_path =
        cache_dir_path + "/" +
        std::to_string(std::hash<std::string>{}(request.zip_file_path));
  }

  auto res = Java_AnimaXUnzipLoader_unzip(
      env,
      JNIConvertHelper::ConvertToJNIStringUTF(env, request.zip_file_path).Get(),
      JNIConvertHelper::ConvertToJNIStringUTF(env, request.unzip_folder_path)
          .Get());

  auto error = Java_AnimaXUnzipLoader_getError(env, res.Get());
  if (!JNIConvertHelper::ConvertToString(env, error.Get()).empty()) {
    callback(UnzipResponse{}, LoaderError{.code = kUnzipFailed});
    return;
  }

  auto path = Java_AnimaXUnzipLoader_getPath(env, res.Get());
  callback(
      UnzipResponse{.path = JNIConvertHelper::ConvertToString(env, path.Get())},
      LoaderError{});
}
}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXUnzipLoader(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
