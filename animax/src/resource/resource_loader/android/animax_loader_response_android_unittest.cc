// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/animax_loader_response_android.h"

#include <android/bitmap.h>

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/jni_utils.h"
#include "gtest/gtest.h"
#include "src/resource/resource_loader/android/animax_closeable_bitmap_reference.h"

using namespace lynx::base::android;
using namespace lynx::animax;

namespace {

ScopedGlobalJavaRef<jobject>
AnimaXLoaderResponseToAnimaXLoaderResponseNativeAdapter(
    JNIEnv* env, ScopedGlobalJavaRef<jobject> j_response) {
  jclass adapter_cls = env->FindClass(
      "com/lynx/animax/loader/AnimaXLoaderResponseNativeAdapter");
  jmethodID create = env->GetStaticMethodID(
      adapter_cls, "create",
      "(Lcom/lynx/animax/loader/AnimaXLoaderResponse;)Lcom/lynx/animax/loader/"
      "AnimaXLoaderResponseNativeAdapter;");
  auto j_response_adapter = ScopedGlobalJavaRef<jobject>{
      env, env->CallStaticObjectMethod(adapter_cls, create, j_response.Get())};
  return j_response_adapter;
}

}  // namespace

TEST(AnimaXLoaderResponseAndroidTest, JStringResponse) {
  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->FindClass("com/lynx/animax/loader/AnimaXLoaderResponse");
  jmethodID createFilePathResponseMethodId = env->GetStaticMethodID(
      cls, "createStringFilePathResponse",
      "(Ljava/lang/String;)Lcom/lynx/animax/loader/AnimaXLoaderResponse;");

  auto file_path = std::string{"/user/tmp/test.json"};
  auto j_file_path = JNIConvertHelper::ConvertToJNIStringUTF(env, file_path);
  auto j_response = ScopedGlobalJavaRef<jobject>{
      env, env->CallStaticObjectMethod(cls, createFilePathResponseMethodId,
                                       j_file_path.Get())};
  auto j_response_adapter =
      AnimaXLoaderResponseToAnimaXLoaderResponseNativeAdapter(env, j_response);

  auto transform_request = AnimaXLoaderResponseToTransformRequestAndroid(
      env, lynx::animax::ResourceRequestType::kDownloadToLocal,
      j_response_adapter.Get());

  EXPECT_EQ(ResourcePayloadType::kFilePath, transform_request.output_type);
  EXPECT_EQ(TransformRequestInputTypeAndroid::kJStringFilePath,
            transform_request.input_type);
  EXPECT_EQ(file_path,
            JNIConvertHelper::ConvertToString(
                env, static_cast<jstring>(transform_request.input.Get())));
}

TEST(AnimaXLoaderResponseAndroidTest, JByteArrayResponse) {
  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->FindClass("com/lynx/animax/loader/AnimaXLoaderResponse");
  jmethodID createDataResponseMethodId = env->GetStaticMethodID(
      cls, "createByteArrayResponse",
      "([B)Lcom/lynx/animax/loader/AnimaXLoaderResponse;");

  auto j_data = ScopedLocalJavaRef<jbyteArray>{
      env, env->NewByteArray(10)};  // Example data array
  env->SetByteArrayRegion(j_data.Get(), 0, 10,
                          reinterpret_cast<const jbyte*>("1234567890"));
  auto j_response = ScopedGlobalJavaRef<jobject>{
      env, env->CallStaticObjectMethod(cls, createDataResponseMethodId,
                                       j_data.Get())};
  auto j_response_adapter =
      AnimaXLoaderResponseToAnimaXLoaderResponseNativeAdapter(env, j_response);

  auto transform_request = AnimaXLoaderResponseToTransformRequestAndroid(
      env, lynx::animax::ResourceRequestType::kLoadRawData,
      j_response_adapter.Get());

  EXPECT_EQ(ResourcePayloadType::kRawData, transform_request.output_type);
  EXPECT_EQ(TransformRequestInputTypeAndroid::kJByteArray,
            transform_request.input_type);
}

namespace {
jobject Create1x1ARGBMutableBitmap(JNIEnv* env) {
  // Find the Bitmap class and the createBitmap method
  jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
  jmethodID createBitmapMethod = env->GetStaticMethodID(
      bitmapClass, "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

  // Find the Bitmap.Config class and the ARGB_8888 field
  jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
  jfieldID argb8888Field = env->GetStaticFieldID(
      bitmapConfigClass, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
  jobject argb8888 =
      env->GetStaticObjectField(bitmapConfigClass, argb8888Field);

  // Create a bitmap of 100x100 pixels
  jobject bitmap = env->CallStaticObjectMethod(bitmapClass, createBitmapMethod,
                                               1, 1, argb8888);

  // Check for exceptions
  if (env->ExceptionCheck()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return nullptr;
  }

  return bitmap;
}
}  // namespace

TEST(AnimaXLoaderResponseAndroidTest,
     kJAnimaXCloseableBitmapReferenceResponse) {
  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->FindClass("com/lynx/animax/loader/AnimaXLoaderResponse");
  jmethodID createBitmapResponseMethodId = env->GetStaticMethodID(
      cls, "createBitmapResponse",
      "(Lcom/lynx/animax/loader/IAnimaXCloseableBitmapReference;)Lcom/lynx/"
      "animax/loader/AnimaXLoaderResponse;");

  // Create a dummy Bitmap object via Java (assuming a helper function exists to
  // create a Bitmap)
  jobject j_bitmap =
      Create1x1ARGBMutableBitmap(env);  // Helper function to create a Bitmap
  ASSERT_NE(nullptr, j_bitmap);
  ScopedLocalJavaRef<jobject> scopedBitmap(env, j_bitmap);
  // Wrap the Bitmap into SimpleCloseableBitmapReference
  jclass simple_closeable_bitmap_class =
      env->FindClass("com/lynx/animax/loader/SimpleCloseableBitmapReference");
  jmethodID simpleCloseableBitmapReferenceConstructor = env->GetMethodID(
      simple_closeable_bitmap_class, "<init>", "(Landroid/graphics/Bitmap;)V");
  auto simple_closeable_bitmap_ref = ScopedLocalJavaRef<jobject>{
      env, env->NewObject(simple_closeable_bitmap_class,
                          simpleCloseableBitmapReferenceConstructor,
                          scopedBitmap.Get())};

  ScopedGlobalJavaRef<jobject> j_response(
      env, env->CallStaticObjectMethod(cls, createBitmapResponseMethodId,
                                       simple_closeable_bitmap_ref.Get()));

  auto j_response_adapter =
      AnimaXLoaderResponseToAnimaXLoaderResponseNativeAdapter(env, j_response);

  auto transform_request = AnimaXLoaderResponseToTransformRequestAndroid(
      env, lynx::animax::ResourceRequestType::kLoadBitmap,
      j_response_adapter.Get());

  EXPECT_EQ(ResourcePayloadType::kBitmap, transform_request.output_type);
  EXPECT_EQ(TransformRequestInputTypeAndroid::kJAnimaXCloseableBitmapReference,
            transform_request.input_type);
  // Access and verify bitmap dimensions
  AndroidBitmapInfo info;

  auto bitmap_ref = AnimaXCloseableBitmapReference{
      ScopedLocalJavaRef<jobject>{transform_request.input}};

  if (AndroidBitmap_getInfo(env, bitmap_ref.GetBitmap().Get(), &info) ==
      ANDROID_BITMAP_RESULT_SUCCESS) {
    EXPECT_EQ(1, info.width);   // Check the width
    EXPECT_EQ(1, info.height);  // Check the height
  } else {
    FAIL() << "Failed to get bitmap info";
  }

  // Close the bitmap
  bitmap_ref.Close();
}

namespace {

ScopedLocalJavaRef<jobject> CreateThrowable(JNIEnv* env,
                                            const std::string& message) {
  auto j_string = JNIConvertHelper::ConvertToJNIStringUTF(env, message);
  jclass throwableClass = env->FindClass("java/lang/Exception");
  jmethodID constructor =
      env->GetMethodID(throwableClass, "<init>", "(Ljava/lang/String;)V");
  jobject exception =
      env->NewObject(throwableClass, constructor, j_string.Get());
  return ScopedLocalJavaRef<jobject>{env, exception};
}

}  // namespace

TEST(AnimaXLoaderResponseAndroidTest, ErrorResponse) {
  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->FindClass("com/lynx/animax/loader/AnimaXLoaderResponse");
  jmethodID createErrorResponseMethodId = env->GetStaticMethodID(
      cls, "createErrorResponse",
      "(Ljava/lang/Throwable;)Lcom/lynx/animax/loader/AnimaXLoaderResponse;");
  const auto error_message = std::string{"example message"};
  auto j_throwable = CreateThrowable(env, error_message);
  ScopedGlobalJavaRef<jobject> j_response(
      env, env->CallStaticObjectMethod(cls, createErrorResponseMethodId,
                                       j_throwable.Get()));

  auto j_response_adapter =
      AnimaXLoaderResponseToAnimaXLoaderResponseNativeAdapter(env, j_response);

  LoaderError result =
      AnimaXLoaderResponseToLoaderError(env, j_response_adapter.Get());

  EXPECT_TRUE(!!result);  // Implicit conversion to bool, should be true if
                          // there's an error
  EXPECT_EQ(result.code, kUserDefined);
  EXPECT_EQ(result.message, error_message);
}

TEST(AnimaXLoaderResponseAndroidTest, NonErrorResponse) {
  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->FindClass("com/lynx/animax/loader/AnimaXLoaderResponse");
  jmethodID createFilePathResponseMethodId = env->GetStaticMethodID(
      cls, "createStringFilePathResponse",
      "(Ljava/lang/String;)Lcom/lynx/animax/loader/AnimaXLoaderResponse;");

  auto j_file_path = ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF("/user/tmp/test.json"));
  ScopedGlobalJavaRef<jobject> j_response(
      env, env->CallStaticObjectMethod(cls, createFilePathResponseMethodId,
                                       j_file_path.Get()));

  auto j_response_adapter =
      AnimaXLoaderResponseToAnimaXLoaderResponseNativeAdapter(env, j_response);

  LoaderError result =
      AnimaXLoaderResponseToLoaderError(env, j_response_adapter.Get());

  EXPECT_FALSE(!!result);     // Should be false, indicating no error
  EXPECT_EQ(result.code, 0);  // Code should be zero, indicating no error
  EXPECT_TRUE(result.message.empty());  // Message should be empty
}
