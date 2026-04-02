// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/thread/thread_assert.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

ThreadAssert::Type *ThreadAssert::Get() {
  static thread_local Type type;
  return &type;
}

ThreadAssert::Type ThreadAssert::GetCurrent() { return *Get(); }

void ThreadAssert::Init(const Type type) { *Get() = type; }

void ThreadAssert::Assert(const Type type) {
  // On Web platform, thread assert is not supported, since all thread are
  // actually the same type.
#ifndef OS_WASM
  DCHECK(*Get() == type);
#endif
}

}  // namespace animax
}  // namespace lynx
