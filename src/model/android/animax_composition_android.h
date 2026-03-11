// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_MODEL_ANDROID_ANIMAX_COMPOSITION_ANDROID_H_
#define ANIMAX_SRC_MODEL_ANDROID_ANIMAX_COMPOSITION_ANDROID_H_

#include "base/include/platform/android/jni_utils.h"
#include "src/model/composition_model.h"

namespace lynx {
namespace animax {

class AnimaXCompositionAndroid {
 public:
  static lynx::base::android::ScopedLocalJavaRef<jobject> Create(
      std::shared_ptr<CompositionModel> composition);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_ANDROID_ANIMAX_COMPOSITION_ANDROID_H_
