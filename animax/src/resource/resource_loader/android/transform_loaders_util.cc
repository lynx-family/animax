// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/transform_loaders_util.h"

#include "base/include/platform/android/jni_convert_helper.h"

namespace lynx {
namespace animax {
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
}  // namespace animax
}  // namespace lynx
