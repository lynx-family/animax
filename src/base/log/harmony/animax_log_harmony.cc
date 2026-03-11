// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <hilog/log.h>
#include <stdio.h>

#include "src/base/log/log.h"
#include "src/base/util/animax_native.h"

const unsigned int LOG_PRINT_DOMAIN = 0xFF00;

void lynx::animax::Log(LogMessage* msg) {
  auto* fn = lynx::animax::AnimaXNative::Instance().GetALogFunction();
  auto stream_str = msg->stream().str();

  if (fn == nullptr) {
    OH_LOG_Print(LOG_APP, LogLevel::LOG_INFO, LOG_PRINT_DOMAIN, "[AnimaX]",
                 "%{public}s", stream_str.c_str());
  } else {
    fn(msg->level(), "AnimaX", stream_str.c_str());
  }
}
