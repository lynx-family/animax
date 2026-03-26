// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/resource_loader_android.h"

#include <android/bitmap.h>

#include <cstring>

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/jni_utils.h"
#include "include/resource/bitmap.h"
#include "include/resource/raw_data.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXResourceLoader_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXResourceLoader_register_jni.h"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"
#include "src/base/thread/thread_assert.h"

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXResourceLoader(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax

namespace {

std::unique_ptr<lynx::animax::RawData> JavaByteArrayToRawData(JNIEnv* env,
                                                              jbyteArray data) {
  if (!data) {
    return nullptr;
  }
  auto bytes =
      lynx::base::android::JNIConvertHelper::ConvertJavaBinary(env, data);
  auto* bytes_ptr = new decltype(bytes){std::move(bytes)};
  return lynx::animax::RawData::MakeRawData(
      bytes_ptr->data(), bytes_ptr->size(),
      [](const void* bytes_ptr) {
        delete static_cast<decltype(bytes)*>(const_cast<void*>(bytes_ptr));
      },
      bytes_ptr);
}

}  // namespace

namespace lynx {
namespace animax {

std::string GetCacheDirectory() {
  JNIEnv* env = base::android::AttachCurrentThread();
  auto cache_directory = Java_AnimaXResourceLoader_getCacheDirectory(env);
  return base::android::JNIConvertHelper::ConvertToString(
      env, cache_directory.Get());
}

std::unique_ptr<lynx::animax::Bitmap> JavaBitmapToBitmap(JNIEnv* env,
                                                         jobject java_bitmap) {
  if (!java_bitmap) {
    return nullptr;
  }
  AndroidBitmapInfo info;
  void* java_bitmap_data = nullptr;
  AndroidBitmap_getInfo(env, java_bitmap, &info);
  bool valid_format = info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                      info.format == ANDROID_BITMAP_FORMAT_RGB_565;
  if (!valid_format || info.height == 0 || info.width == 0 ||
      info.stride == 0) {
    ANIMAX_LOGE(
        "Failed to convert Android Bitmap to animax::Bitmap. The issue could "
        "be due to invalid dimensions or the bitmap not being in the RGBA8888 "
        "or RGB565"
        "format.");
    return nullptr;
  }
  const auto bitmap_byte_size = info.stride * info.height;
  AndroidBitmap_lockPixels(env, java_bitmap, &java_bitmap_data);
  // A call to AndroidBitmap_lockPixels must be balanced by a call to
  // AndroidBitmap_unlockPixels We use a unique_ptr to ensure that the call to
  // AndroidBitmap_unlockPixels is made
  auto android_bitmap_lock_release_deleter = [env](jobject java_bitmap) {
    AndroidBitmap_unlockPixels(env, java_bitmap);
  };
  auto android_bitmap_lock_guard =
      std::unique_ptr<std::remove_pointer_t<jobject>,
                      decltype(android_bitmap_lock_release_deleter)>{
          java_bitmap, android_bitmap_lock_release_deleter};
  if (java_bitmap_data == nullptr) {
    ANIMAX_LOGE("Failed to lock bitmap pixels");
    return nullptr;
  }
  void* bitmap_data = std::malloc(bitmap_byte_size);
  if (bitmap_data == nullptr) {
    ANIMAX_LOGE("Failed to malloc bitmap data");
    return nullptr;
  }
  // java_bitmap_data != nullptr && bitmap_data != nullptr && bitmap_byte_size
  // != 0
  memcpy(bitmap_data, java_bitmap_data, bitmap_byte_size);
  BitmapFormat format = info.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                            ? BitmapFormat::kRGBA
                            : BitmapFormat::kRGB565;
  auto bitmap = lynx::animax::Bitmap::Make(
      info.width, info.height, bitmap_data,
      [](const void* bitmap_data) {
        std::free(const_cast<void*>(bitmap_data));
      },
      bitmap_data, format);
  return bitmap;
}

bool SaveByteArrayIntoFile(const std::string& dst_file_path, jbyteArray data) {
  JNIEnv* env = base::android::AttachCurrentThread();
  auto res = Java_AnimaXResourceLoader_saveByteArrayIntoFile(
      env,
      base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, dst_file_path)
          .Get(),
      data);
  return res == JNI_TRUE;
}

lynx::base::android::ScopedLocalJavaRef<jbyteArray> ReadFile(
    jstring file_path) {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXResourceLoader_readFile(env, file_path);
}

}  // namespace animax
}  // namespace lynx
