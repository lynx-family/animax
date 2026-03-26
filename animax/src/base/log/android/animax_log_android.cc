// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <android/log.h>
#include <stdio.h>

#include <string>

#include "platform/android/animax_android/src/main/jni/gen/AnimaXLog_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLog_register_jni.h"
#include "src/base/log/log.h"
#include "src/base/util/animax_native.h"

namespace {
constexpr const char* kAnimaXTAG = "[AnimaX]";

inline std::string CreatePrefixedTag(const char* tag) {
  return std::string(kAnimaXTAG) + tag;
}

/**
 * Performs the actual log writing operation
 * @param level The severity level of the log message
 * @param tag The tag used to identify the log source
 * @param msg The actual message content to be logged
 *
 * Uses custom AnimaX logging function if available, falls back to Android
 * native logging otherwise
 */
inline void WriteLog(lynx::animax::AnimaXLogLevel level, const char* tag,
                     const char* msg) {
  auto* fn = lynx::animax::AnimaXNative::Instance().GetALogFunction();
  if (fn == nullptr) {
    __android_log_write(ANDROID_LOG_VERBOSE, tag, msg);
  } else {
    fn(level, tag, msg);
  }
}
}  // namespace

/**
 * Entry point for logging from C++ platform
 * @param msg LogMessage object containing log level, content and other metadata
 *
 * Handles logging from C++ code by converting LogMessage stream data to string
 * and writing to log
 */
void lynx::animax::Log(LogMessage* msg) {
  if (msg == nullptr) {
    return;
  }

  auto stream_str = msg->stream().str();
  WriteLog(msg->level(), kAnimaXTAG, stream_str.c_str());
}

/**
 * Entry point for logging from Java platform through JNI
 * @param env JNI environment pointer
 * @param jcaller The calling Java class
 * @param level Log severity level
 * @param tag Log tag string from Java
 * @param msg Log message content from Java
 *
 * Handles logging from Java code by converting JNI strings to C++ strings and
 * writing to log
 */
static void Log(JNIEnv* env, jclass jcaller, jint level, jstring tag,
                jstring msg) {
  if (tag == nullptr || msg == nullptr) {
    return;
  }

  const char* tag_str = env->GetStringUTFChars(tag, nullptr);
  const char* msg_str = env->GetStringUTFChars(msg, nullptr);

  if (tag_str && msg_str) {
    std::string prefixed_tag = CreatePrefixedTag(tag_str);
    WriteLog(static_cast<lynx::animax::AnimaXLogLevel>(level),
             prefixed_tag.c_str(), msg_str);
  }

  if (tag_str) {
    env->ReleaseStringUTFChars(tag, tag_str);
  }
  if (msg_str) {
    env->ReleaseStringUTFChars(msg, msg_str);
  }
}

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXLog(JNIEnv* env) { return RegisterNativesImpl(env); }
}  // namespace jni
}  // namespace animax
