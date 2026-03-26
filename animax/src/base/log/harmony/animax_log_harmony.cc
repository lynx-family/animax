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
  LogLevel priority = LogLevel::LOG_DEBUG;
  switch (msg->level()) {
    case LOG_VERBOSE:
    case LOG_DEBUG:
      priority = LogLevel::LOG_DEBUG;
      break;
    case LOG_INFO:
      priority = LogLevel::LOG_INFO;
      break;
    case LOG_WARNING:
      priority = LogLevel::LOG_WARN;
      break;
    case LOG_ERROR:
      priority = LogLevel::LOG_ERROR;
      break;
    case LOG_FATAL:
      priority = LogLevel::LOG_FATAL;
      break;
    default:
      break;
  }
  OH_LOG_Print(LOG_APP, priority, LOG_PRINT_DOMAIN, "[AnimaX]", "%{public}s",
               stream_str.c_str());
  if (fn != nullptr) {
    fn(msg->level(), "AnimaX", stream_str.c_str());
  }
}
