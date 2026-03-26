// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_BASE_LOG_LOG_MESSAGE_H_
#define ANIMAX_INCLUDE_BASE_LOG_LOG_MESSAGE_H_

#include <sstream>

#include "include/base/macros.h"
#include "src/base/log/animax_log_level.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT LogMessage {
 public:
  LogMessage(const char* file, int line, AnimaXLogLevel level);
  ~LogMessage();
  std::ostringstream& stream() { return stream_; }
  AnimaXLogLevel level() { return level_; }
  size_t messageStart() { return message_start_; }

  const char* file_;
  const int line_;
  AnimaXLogLevel level_;

 private:
  void Init(const char* file, int line);
  std::ostringstream stream_;
  size_t message_start_;

  LogMessage(const LogMessage&) = delete;
  LogMessage& operator=(const LogMessage&) = delete;
};

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class ANIMAX_EXPORT LogMessageVoidify {
 public:
  LogMessageVoidify() {}
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream&) {}
};

/**
 Specialized for different log levels to omit severity argument
 at callsite for binary size optimization.
 */
class ANIMAX_EXPORT LogMessageI : public LogMessage {
 public:
  LogMessageI(const char* file, int line) : LogMessage(file, line, LOG_INFO) {}
};

class ANIMAX_EXPORT LogMessageW : public LogMessage {
 public:
  LogMessageW(const char* file, int line)
      : LogMessage(file, line, LOG_WARNING) {}
};

class ANIMAX_EXPORT LogMessageE : public LogMessage {
 public:
  LogMessageE(const char* file, int line) : LogMessage(file, line, LOG_ERROR) {}
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_BASE_LOG_LOG_MESSAGE_H_
