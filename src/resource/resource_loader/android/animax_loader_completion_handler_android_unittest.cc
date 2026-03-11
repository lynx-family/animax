// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/animax_loader_completion_handler_android.h"

#include <future>

#include "base/include/platform/android/jni_convert_helper.h"
#include "gtest/gtest.h"

using namespace lynx::base::android;
using namespace lynx::animax;

TEST(AnimaXLoaderCompletionHandlerAndroidTest,
     ShouldHandleSuccessResponseCorrectly) {
  // Given: Setup the Java environment and prepare necessary JNI references
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
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback = [file_path, &promise](TransformRequestAndroid res,
                                        LoaderError error) {
    JNIEnv* env = AttachCurrentThread();

    // Then: Verify that the response is processed correctly without errors
    EXPECT_FALSE(!!error);
    EXPECT_EQ(TransformRequestInputTypeAndroid::kJStringFilePath,
              res.input_type);
    EXPECT_EQ(file_path, JNIConvertHelper::ConvertToString(
                             env, static_cast<jstring>(res.input.Get())));
    EXPECT_EQ(ResourcePayloadType::kFilePath, res.output_type);
    promise.set_value();
  };

  // When: Trigger the completion handler with a prepared response
  auto completion_handler =
      LoaderWrapperAndroidCallbackToAnimaXLoaderCompletionHandler(
          env, lynx::animax::ResourceRequestType::kDownloadToLocal,
          std::move(callback));
  jclass completion_handler_cls =
      env->FindClass("com/lynx/animax/loader/AnimaXLoaderCompletionHandler");
  jmethodID onCompletion =
      env->GetMethodID(completion_handler_cls, "onComplete",
                       "(Lcom/lynx/animax/loader/AnimaXLoaderResponse;)V");
  env->CallVoidMethod(completion_handler.Get(), onCompletion, j_response.Get());

  // Then: Assert that the callback is called.
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
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

TEST(AnimaXLoaderCompletionHandlerAndroidTest,
     ShouldHandleErrorResponseCorrectly) {
  // Given: Setup the Java environment and prepare necessary JNI references
  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->FindClass("com/lynx/animax/loader/AnimaXLoaderResponse");
  jmethodID createErrorResponseMethodId = env->GetStaticMethodID(
      cls, "createErrorResponse",
      "(Ljava/lang/Throwable;)Lcom/lynx/animax/loader/AnimaXLoaderResponse;");

  auto error_message = "Sample error message";
  auto j_throwable = CreateThrowable(env, error_message);
  ScopedGlobalJavaRef<jobject> j_response(
      env, env->CallStaticObjectMethod(cls, createErrorResponseMethodId,
                                       j_throwable.Get()));
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback = [error_message, &promise](TransformRequestAndroid res,
                                            LoaderError error) {
    JNIEnv* env = AttachCurrentThread();

    // Then: Verify that the response is processed correctly with errors
    EXPECT_TRUE(!!error);
    EXPECT_EQ(TransformRequestInputTypeAndroid::kInvalid, res.input_type);
    EXPECT_EQ(ResourcePayloadType::kInvalid, res.output_type);
    EXPECT_EQ(error_message, error.message);
    promise.set_value();
  };

  // When: Trigger the completion handler with a prepared response
  auto completion_handler =
      LoaderWrapperAndroidCallbackToAnimaXLoaderCompletionHandler(
          env, lynx::animax::ResourceRequestType::kDownloadToLocal,
          std::move(callback));
  jclass completion_handler_cls =
      env->FindClass("com/lynx/animax/loader/AnimaXLoaderCompletionHandler");
  jmethodID onCompletion =
      env->GetMethodID(completion_handler_cls, "onComplete",
                       "(Lcom/lynx/animax/loader/AnimaXLoaderResponse;)V");
  env->CallVoidMethod(completion_handler.Get(), onCompletion, j_response.Get());

  // Then: Assert that the callback is called.
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}

TEST(AnimaXLoaderCompletionHandlerAndroidTest,
     TriggerOnCompletionMultipleTimesShouldInvokeCallbackOnce) {
  // Given: Setup the Java environment and prepare necessary JNI references
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
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback_called_count = 0;
  auto callback = [&callback_called_count, file_path, &promise](
                      TransformRequestAndroid res, LoaderError error) {
    // When the callback is trigger, increment the counter.
    ++callback_called_count;
    promise.set_value();
  };
  auto completion_handler =
      LoaderWrapperAndroidCallbackToAnimaXLoaderCompletionHandler(
          env, lynx::animax::ResourceRequestType::kDownloadToLocal,
          std::move(callback));
  jclass completion_handler_cls =
      env->FindClass("com/lynx/animax/loader/AnimaXLoaderCompletionHandler");
  jmethodID onCompletion =
      env->GetMethodID(completion_handler_cls, "onComplete",
                       "(Lcom/lynx/animax/loader/AnimaXLoaderResponse;)V");

  // When: Trigger the completion handler with a prepared response multiple
  // times
  for (int i = 0; i < 10; ++i) {
    env->CallVoidMethod(completion_handler.Get(), onCompletion,
                        j_response.Get());
  }

  // Then: Assert that the callback is only called once.
  EXPECT_EQ(1, callback_called_count);
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}
