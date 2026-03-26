// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANDROID_METRICS_CALLBACK_ANDROID_H_
#define ANIMAX_SRC_BASE_UTIL_ANDROID_METRICS_CALLBACK_ANDROID_H_

#include <jni.h>

#include "base/include/platform/android/jni_convert_helper.h"

namespace lynx {
namespace animax {
class MetricsCallbackAndroid {
 public:
  explicit MetricsCallbackAndroid(
      lynx::base::android::ScopedGlobalJavaRef<jobject> java_callback,
      lynx::base::android::ScopedGlobalJavaRef<jobject> java_events_map)
      : java_callback_{std::move(java_callback)},
        java_events_map_{std::move(java_events_map)} {}

  void OnMetricsReady(jobject metrics);

 private:
  lynx::base::android::ScopedGlobalJavaRef<jobject> java_callback_;
  lynx::base::android::ScopedGlobalJavaRef<jobject> java_events_map_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_ANDROID_METRICS_CALLBACK_ANDROID_H_
