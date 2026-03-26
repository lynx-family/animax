// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/base/log/log_message.h"

#include <string>

#include "src/base/log/log.h"

#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace lynx {
namespace animax {
LogMessage::LogMessage(const char* file, int line, AnimaXLogLevel level)
    : file_(file), line_(line), level_(level) {
  Init(file, line);
}

LogMessage::~LogMessage() {
  // on Windows, use spdlog which add newline at the end of each line.
#if !defined(_WIN32)
  stream_ << std::endl;
#endif

#if defined(OS_ANDROID) || defined(OS_IOS) || defined(OS_OSX) || \
    defined(OS_WIN) || defined(OS_HARMONY)
  lynx::animax::Log(this);
#else
  std::string str_newline(stream_.str());
  printf("animax: %s\n", str_newline.c_str());
#endif

  if (level_ == lynx::animax::LOG_FATAL) {
    abort();
  }
}

// writes the common header info to the stream
void LogMessage::Init(const char* file, int line) {
  std::string filename(file);

  stream_ << "[";
#if OS_ANDROID
  stream_ << gettid() << ':';
#endif
  stream_ << filename << "(" << line << ")] ";
  message_start_ = stream_.str().length();
}

}  // namespace animax
}  // namespace lynx
