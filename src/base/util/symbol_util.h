// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_SYMBOL_UTIL_H_
#define ANIMAX_SRC_BASE_UTIL_SYMBOL_UTIL_H_

#include <dlfcn.h>

#include "include/base/macros.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT SymbolUtil {
 public:
  template <typename T>
  static bool LoadSymbol(void *handle, T *&function, const char *symbol) {
    if (!handle) {
      return false;
    }
    function = reinterpret_cast<T *>(dlsym(handle, symbol));
    if (!function) {
      ANIMAX_LOGE("SymbolUtil: Failed to load symbol " << symbol)
      return false;
    }
    return true;
  }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_SYMBOL_UTIL_H_
