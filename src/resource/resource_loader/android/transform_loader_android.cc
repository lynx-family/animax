// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/transform_loader_android.h"

#include <chrono>

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/jni_utils.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader/android/animax_closeable_bitmap_reference.h"
#include "src/resource/resource_loader/android/resource_loader_android.h"
#include "src/resource/resource_loader/android/transform_loaders_util.h"

namespace lynx {
namespace animax {
TransformLoaderAndroid::Ptr MatrixGetLoader(
    const TransformLoaderAndroid::TransformMatrix& matrix,
    TransformRequestInputTypeAndroid input, ResourcePayloadType output) {
  return matrix[static_cast<size_t>(input)][static_cast<size_t>(output)];
}

void MatrixSetLoader(TransformLoaderAndroid::TransformMatrix& matrix,
                     TransformRequestInputTypeAndroid input,
                     ResourcePayloadType output,
                     TransformLoaderAndroid::Ptr loader) {
  matrix[static_cast<size_t>(input)][static_cast<size_t>(output)] =
      std::move(loader);
}

TransformLoaderAndroid::TransformLoaderAndroid() {
  // load all the handlers into the matrix
  MatrixSetLoader(
      matrix_,
      TransformRequestInputTypeAndroid::kJAnimaXCloseableBitmapReference,
      ResourcePayloadType::kBitmap,
      ResourceThreadExecLoader(TransformLoaderAndroid::Make<
                               JBitmapToNativeBitmapTransformLoader>()));
  MatrixSetLoader(
      matrix_, TransformRequestInputTypeAndroid::kJByteArray,
      ResourcePayloadType::kRawData,
      ResourceThreadExecLoader(TransformLoaderAndroid::Make<
                               JByteArrayToNativeRawDataTransformLoader>()));
  MatrixSetLoader(
      matrix_, TransformRequestInputTypeAndroid::kJByteArray,
      ResourcePayloadType::kFilePath,
      ResourceThreadExecLoader(TransformLoaderAndroid::Make<
                               JByteArrayToNativeStringTransformLoader>()));
  MatrixSetLoader(
      matrix_, TransformRequestInputTypeAndroid::kJStringFilePath,
      ResourcePayloadType::kFilePath,
      TransformLoaderAndroid::Make<JStringToNativeStringTransformLoader>());
  MatrixSetLoader(
      matrix_, TransformRequestInputTypeAndroid::kJStringFilePath,
      ResourcePayloadType::kRawData,
      ResourceThreadExecLoader(TransformLoaderAndroid::Make<
                               JStringToNativeRawDataTransformLoader>()));
}

void TransformLoaderAndroid::Load(TransformRequestAndroid input_data,
                                  CallbackType callback) {
  auto loader =
      MatrixGetLoader(matrix_, input_data.input_type, input_data.output_type);

  if (!loader) {
    callback(TransformResponseAndroid{},
             LoaderError{.code = LoaderErrorCode::kInvalidLoader,
                         .message = "Transform Loader is unable to transform: "
                                    "Failed to find matching loader."});
    return;
  }

  loader->Load(std::move(input_data),
               [callback = std::move(callback)](
                   TransformResponseAndroid response, LoaderError err) {
                 callback(std::move(response), std::move(err));
               });
}

TransformLoaderAndroid::Ptr TransformLoaderAndroid::GetLoader(
    TransformRequestInputTypeAndroid input_type,
    ResourcePayloadType output_type) {
  return matrix_[static_cast<size_t>(input_type)]
                [static_cast<size_t>(output_type)];
}

void JStringToNativeRawDataTransformLoader::Load(
    TransformRequestAndroid input_data, CallbackType callback) {
  JNIEnv* env = base::android::AttachCurrentThread();
  jstring path = static_cast<jstring>(input_data.input.Get());

  auto res = ReadFile(path);

  // if getting a nullptr
  if (!res.Get()) {
    callback(TransformResponseAndroid{},
             LoaderError{.code = kInvalidFilePath,
                         .message = "Failed to read the file."});
  } else {
    callback(
        TransformResponseAndroid{.payload = MakeRawDataResourcePayload(
                                     JavaByteArrayToRawData(env, res.Get()))},
        LoaderError{});
  }
}

void JStringToNativeStringTransformLoader::Load(
    TransformRequestAndroid input_data, CallbackType callback) {
  JNIEnv* env = base::android::AttachCurrentThread();
  jstring j_str = static_cast<jstring>(input_data.input.Get());

  // the function ConvertToString always succeeds.
  callback(
      TransformResponseAndroid{
          .payload = MakePathResourcePayload(
              base::android::JNIConvertHelper::ConvertToString(env, j_str))},
      LoaderError{});
}

void JBitmapToNativeBitmapTransformLoader::Load(
    TransformRequestAndroid input_data, CallbackType callback) {
  JNIEnv* env = base::android::AttachCurrentThread();
  auto java_bitmap_ref =
      base::android::ScopedLocalJavaRef<jobject>(input_data.input);
  auto bitmap_ref = AnimaXCloseableBitmapReference{java_bitmap_ref};

  auto res = std::unique_ptr<Bitmap>{};
  auto error = LoaderError{};

  if (!bitmap_ref.IsValid()) {
    error = LoaderError{.code = kBitmapError,
                        .message = "AnimaXCloseableBitmapReference is closed."};
  } else if (bitmap_ref.GetBitmap().IsNull()) {
    error = LoaderError{
        .code = kBitmapError,
        .message = "AnimaXCloseableBitmapReference returns a null bitmap."};
  } else {
    auto bitmap = bitmap_ref.GetBitmap();
    res = JavaBitmapToBitmap(env, bitmap.Get());
    bitmap_ref.Close();
  }

  if (error) {
    callback(TransformResponseAndroid{}, std::move(error));
  } else {
    callback(TransformResponseAndroid{.payload = MakeBitmapResourcePayload(
                                          std::move(res))},
             LoaderError{});
  }
}

void JByteArrayToNativeStringTransformLoader::Load(
    TransformRequestAndroid input_data, CallbackType callback) {
  jbyteArray j_byte_array = static_cast<jbyteArray>(input_data.input.Get());

  // create a temporary file for storing the byte array
  // get the temporary folder where the file is intended to be
  std::string cache_dir_path = GetCacheDirectory();

  // if getting the folder fails, simply return.
  if (cache_dir_path.empty()) {
    callback(TransformResponseAndroid{},
             LoaderError{.code = kInvalidFilePath,
                         .message = "Failed to create a temporary file for "
                                    "storing the byte array."});
    return;
  }

  // assemble the full path to the file
  uintptr_t pointer_value = reinterpret_cast<uintptr_t>(input_data.input.Get());
  auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
  size_t hash_value =
      std::hash<uintptr_t>{}(pointer_value) ^ std::hash<uint64_t>{}(timestamp);
  std::string local_temporary_file_path =
      cache_dir_path + "/" + std::to_string(hash_value);

  bool res = SaveByteArrayIntoFile(local_temporary_file_path, j_byte_array);

  if (res) {
    callback(TransformResponseAndroid{.payload = MakePathResourcePayload(
                                          local_temporary_file_path)},
             LoaderError{});
    return;
  }
  callback(TransformResponseAndroid{},
           LoaderError{.code = kInvalidRawData,
                       .message = "Failed to save a byte array on disk."});
}

void JByteArrayToNativeRawDataTransformLoader::Load(
    TransformRequestAndroid input_data, CallbackType callback) {
  JNIEnv* env = base::android::AttachCurrentThread();
  jbyteArray j_byte_array = static_cast<jbyteArray>(input_data.input.Get());
  auto res = JavaByteArrayToRawData(env, j_byte_array);

  if (!res) {
    callback(
        TransformResponseAndroid{},
        LoaderError{.code = kInvalidRawData,
                    .message = "Failed to convert a byte array to raw data."});
  } else {
    callback(TransformResponseAndroid{.payload = MakeRawDataResourcePayload(
                                          std::move(res))},
             LoaderError{});
  }
}
}  // namespace animax
}  // namespace lynx
