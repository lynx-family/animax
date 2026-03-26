// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_UNZIP_LOADER_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_UNZIP_LOADER_ANDROID_H_

#include <string>

#include "include/resource/loader.h"
#include "include/resource/unzip_task.h"
#include "src/base/util/android/jni_helper.h"

namespace lynx {
namespace animax {

class AnimaxUnzipLoaderAndroid : public Loader<UnzipRequest, UnzipResponse> {
 public:
  void Load(UnzipRequest, CallbackType) override;
  ~AnimaxUnzipLoaderAndroid() override = default;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_UNZIP_LOADER_ANDROID_H_
