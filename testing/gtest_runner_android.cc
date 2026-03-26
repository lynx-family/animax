// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "base/include/platform/android/jni_convert_helper.h"
#include "gtest/gtest.h"
#include "testing/jni_headers/gen/GTestRunnerTest_jni.h"

extern "C" JNIEXPORT void JNICALL
Java_com_lynx_animax_GTestRunnerTest_registerJNI(JNIEnv* env, jclass jclazz) {
  RegisterNativesImpl(env);
}

jint RunGTestsNative(JNIEnv* env, jobject jcaller, jobjectArray argv) {
  lynx::base::android::ScopedLocalJavaRef<jobject> local_ref(env, jcaller);
  int32_t c_argc = env->GetArrayLength(argv) + 1;
  char** c_argv = new char*[c_argc];
  // c_argv is passed to InitGoogleTest with reference
  // to the argument in the main function.
  // The first argument is the name of the file to be executed,
  // which can be ignored as a null pointer in this scenario.
  c_argv[0] = nullptr;
  std::vector<std::string> args_vec = lynx::base::android::JNIConvertHelper::
      ConvertJavaStringArrayToStringVector(env, argv);
  for (size_t i = 0; i < args_vec.size(); i++) {
    c_argv[i + 1] = const_cast<char*>(args_vec[i].c_str());
  }
  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();
  testing::InitGoogleTest(&c_argc, c_argv);
  delete[] c_argv;
  int result = RUN_ALL_TESTS();
  std::string output = testing::internal::GetCapturedStdout() +
                       testing::internal::GetCapturedStderr();
  auto jni_msg =
      lynx::base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, output);
  Java_GTestRunnerTest_onTestMessage(env, local_ref.Get(), jni_msg.Get());
  return result;
}
