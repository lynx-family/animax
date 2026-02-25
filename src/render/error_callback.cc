// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/error_callback.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

void SkityErrorCallbackDefault(skity::GPUError, char const *message,
                               void *userdata) {
  ANIMAX_LOGE("skity error: " << message);
}

}  // namespace animax
}  // namespace lynx
