// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_KEY_PATH_ANDROID_H_
#define ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_KEY_PATH_ANDROID_H_

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/scoped_java_ref.h"
#include "include/property/animax_key_path.h"

namespace lynx {
namespace animax {

class AnimaXKeyPathAndroid : public AnimaXKeyPath {
 public:
  explicit AnimaXKeyPathAndroid(JNIEnv* env, jobject key_path);

  /**
   * Gets the Java object reference
   * @return The Java object reference
   */
  jobject GetJavaObject() const { return key_path_.Get(); }

 private:
  base::android::ScopedGlobalJavaRef<jobject> key_path_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_KEY_PATH_ANDROID_H_
