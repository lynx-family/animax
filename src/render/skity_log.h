// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SKITY_LOG_H_
#define ANIMAX_SRC_RENDER_SKITY_LOG_H_

#include "src/base/util/animax_native.h"

namespace lynx {
namespace animax {

class SkityLog final {
 public:
  SkityLog() = delete;
  ~SkityLog() = delete;

  static void RegisterLogFunction(ALogFunction logger_function);

  static ALogFunction GetLogFunction();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SKITY_LOG_H_
