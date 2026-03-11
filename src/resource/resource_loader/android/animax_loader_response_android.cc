// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/animax_loader_response_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "include/resource/resource_task.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoaderResponseNativeAdapter_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoaderResponseNativeAdapter_register_jni.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

enum AnimaXLoaderResponseType { kFilePath, kData, kBitmap, kError };

LoaderError AnimaXLoaderResponseToLoaderError(JNIEnv* env, jobject response) {
  const auto response_type = static_cast<AnimaXLoaderResponseType>(
      Java_AnimaXLoaderResponseNativeAdapter_getType(env, response));
  if (response_type != kError) {
    return LoaderError{};
  }
  auto j_error_msg =
      Java_AnimaXLoaderResponseNativeAdapter_getErrorMessage(env, response);
  auto error_msg =
      base::android::JNIConvertHelper::ConvertToString(env, j_error_msg.Get());
  return LoaderError{
      .code = kUserDefined,
      .message = std::move(error_msg),
  };
}

TransformRequestAndroid AnimaXLoaderResponseToTransformRequestAndroid(
    JNIEnv* env, ResourceRequestType request_type, jobject response) {
  const auto response_type = static_cast<AnimaXLoaderResponseType>(
      Java_AnimaXLoaderResponseNativeAdapter_getType(env, response));
  auto transform_request = TransformRequestAndroid{};

  switch (request_type) {
    case ResourceRequestType::kDownloadToLocal:
      transform_request.output_type = ResourcePayloadType::kFilePath;
      break;
    case ResourceRequestType::kLoadBitmap:
      transform_request.output_type = ResourcePayloadType::kBitmap;
      break;
    case ResourceRequestType::kLoadRawData:
      transform_request.output_type = ResourcePayloadType::kRawData;
    case ResourceRequestType::kInvalid:
      break;
  }

  switch (response_type) {
    case kFilePath: {
      auto j_file_path =
          Java_AnimaXLoaderResponseNativeAdapter_getFilePath(env, response);
      transform_request.input_type =
          TransformRequestInputTypeAndroid::kJStringFilePath;
      transform_request.input =
          base::android::ScopedGlobalJavaRef<jobject>{j_file_path};
      break;
    }
    case kData: {
      auto j_byte_array =
          Java_AnimaXLoaderResponseNativeAdapter_getData(env, response);
      transform_request.input_type =
          TransformRequestInputTypeAndroid::kJByteArray;
      transform_request.input =
          base::android::ScopedGlobalJavaRef<jobject>{j_byte_array};
      break;
    }
    case kBitmap: {
      auto j_bitmap_ref =
          Java_AnimaXLoaderResponseNativeAdapter_getBitmap(env, response);
      transform_request.input_type =
          TransformRequestInputTypeAndroid::kJAnimaXCloseableBitmapReference;
      transform_request.input =
          base::android::ScopedGlobalJavaRef<jobject>{j_bitmap_ref};
      break;
    }
    case kError:
      // Error should be checked with AnimaXLoaderResponseToLoaderError first.
      DCHECK(false);
      break;
  }
  return transform_request;
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXLoaderResponseNativeAdapter(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
