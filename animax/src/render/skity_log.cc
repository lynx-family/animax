// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "render/skity_log.h"

#include "skity/io/logger.hpp"

namespace lynx {
namespace animax {

namespace {
ALogFunction logger_function_ = nullptr;

void SkityALogI(const char* msg) {
  if (logger_function_) {
    logger_function_(AnimaXLogLevel::LOG_INFO, "Skity", msg);
  }
}

void SkityALogE(const char* msg) {
  if (logger_function_) {
    logger_function_(AnimaXLogLevel::LOG_ERROR, "Skity", msg);
  }
}

void SkityALogD(const char* msg) {
  if (logger_function_) {
    logger_function_(AnimaXLogLevel::LOG_DEBUG, "Skity", msg);
  }
}
}  // namespace

void SkityLog::RegisterLogFunction(ALogFunction logger_function) {
  logger_function_ = logger_function;

  skity::Logger::CustomLogger logger = {
      .log_i = &SkityALogI,
      .log_d = &SkityALogD,
      .log_e = &SkityALogE,
  };
  skity::Logger::RegisterLog(&logger);
}

ALogFunction SkityLog::GetLogFunction() { return logger_function_; }

}  // namespace animax
}  // namespace lynx
