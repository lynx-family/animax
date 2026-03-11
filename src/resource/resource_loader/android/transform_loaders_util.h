// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_LOADERS_UTIL_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_LOADERS_UTIL_H_

#include <jni.h>

#include <memory>

#include "base/include/platform/android/scoped_java_ref.h"
#include "include/resource/raw_data.h"

namespace lynx {
namespace animax {
std::unique_ptr<lynx::animax::RawData> JavaByteArrayToRawData(JNIEnv* env,
                                                              jbyteArray data);

bool SaveByteArrayIntoFile(const std::string& dst_file_path, jbyteArray data);

lynx::base::android::ScopedLocalJavaRef<jbyteArray> ReadFile(jstring file_path);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_LOADERS_UTIL_H_
