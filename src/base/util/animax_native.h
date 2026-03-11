// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANIMAX_NATIVE_H_
#define ANIMAX_SRC_BASE_UTIL_ANIMAX_NATIVE_H_

#include "src/base/log/animax_log_level.h"

namespace lynx {
namespace animax {

/**
 * void alog_write_default_instance(unsigned int level,
 *                               const char *tag,
 *                               const char *text);
 */
typedef void (*ALogFunction)(AnimaXLogLevel level, const char* tag,
                             const char* text);

class AnimaXNative {
 public:
  static AnimaXNative& Instance();

  virtual ~AnimaXNative() = default;

  void RegisterALogFunction(ALogFunction logger_function);

  ALogFunction GetALogFunction();

 private:
  ALogFunction log_function_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_ANIMAX_NATIVE_H_
