// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_ERROR_CALLBACK_H_
#define ANIMAX_SRC_RENDER_ERROR_CALLBACK_H_

#include "skity/gpu/gpu_context.hpp"

namespace lynx {
namespace animax {

void SkityErrorCallbackDefault(skity::GPUError error, char const *message,
                               void *userdata);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_ERROR_CALLBACK_H_
