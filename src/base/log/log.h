// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_LOG_LOG_H_
#define ANIMAX_SRC_BASE_LOG_LOG_H_

#include <string>

#include "base/include/path_utils.h"
#include "include/base/log/log_message.h"
#include "src/base/log/animax_log_level.h"

namespace lynx {
namespace animax {

void Log(LogMessage* msg);

#ifdef __FILE_NAME__
#define __LOG_FILE_NAME__ __FILE_NAME__
#else
#define __LOG_FILE_NAME__ \
  lynx::base::PathUtils::GetLastPath(__FILE__, sizeof(__FILE__) - 1)
#endif

#define ANIMAX_LOG_IS_ON(level) \
  ((lynx::animax::LOG_##level) >= lynx::animax::LOG_VERBOSE)

#define ANIMAX_LOG_STREAM(level)                        \
  lynx::animax::LogMessage(__LOG_FILE_NAME__, __LINE__, \
                           lynx::animax::LOG_##level)   \
      .stream()

#define ANIMAX_LOG_STREAM_INFO \
  lynx::animax::LogMessageI(__LOG_FILE_NAME__, __LINE__).stream()

#define ANIMAX_LOG_STREAM_WARNING \
  lynx::animax::LogMessageW(__LOG_FILE_NAME__, __LINE__).stream()

#define ANIMAX_LOG_STREAM_ERROR \
  lynx::animax::LogMessageE(__LOG_FILE_NAME__, __LINE__).stream()

#define ANIMAX_LAZY_STREAM(stream, condition) \
  !(condition) ? (void)0 : lynx::animax::LogMessageVoidify() & (stream)

// Use this macro to suppress warning if the variable in log is not used.
#define UNUSED_LOG_VARIABLE __attribute__((unused))

#ifndef ANIMAX_MIN_LOG_LEVEL
#ifdef NDEBUG
#define ANIMAX_MIN_LOG_LEVEL ANIMAX_LOG_LEVEL_INFO
#else
#define ANIMAX_MIN_LOG_LEVEL ANIMAX_LOG_LEVEL_VERBOSE
#endif
#endif

#if ANIMAX_MIN_LOG_LEVEL <= ANIMAX_LOG_LEVEL_VERBOSE
#define ANIMAX_LOGV(msg)                                                      \
  {                                                                           \
    ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM(VERBOSE), ANIMAX_LOG_IS_ON(VERBOSE)) \
        << msg;                                                               \
  }
#else
#define ANIMAX_LOGV(msg)
#endif

#if ANIMAX_MIN_LOG_LEVEL <= ANIMAX_LOG_LEVEL_DEBUG
#define ANIMAX_LOGD(msg)                                                  \
  {                                                                       \
    ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM(DEBUG), ANIMAX_LOG_IS_ON(DEBUG)) \
        << msg;                                                           \
  }
#else
#define ANIMAX_LOGD(msg)
#endif

#if ANIMAX_MIN_LOG_LEVEL <= ANIMAX_LOG_LEVEL_INFO
#define ANIMAX_LOGI(msg) \
  { ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM_INFO, ANIMAX_LOG_IS_ON(INFO)) << msg; }
#else
#define ANIMAX_LOGI(msg)
#endif

#if ANIMAX_MIN_LOG_LEVEL <= ANIMAX_LOG_LEVEL_WARNING
#define ANIMAX_LOGW(msg)                                                     \
  {                                                                          \
    ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM_WARNING, ANIMAX_LOG_IS_ON(WARNING)) \
        << msg;                                                              \
  }
#else
#define ANIMAX_LOGW(msg)
#endif

#if ANIMAX_MIN_LOG_LEVEL <= ANIMAX_LOG_LEVEL_ERROR
#define ANIMAX_LOGE(msg)                                                 \
  {                                                                      \
    ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM_ERROR, ANIMAX_LOG_IS_ON(ERROR)) \
        << msg;                                                          \
  }
#else
#define ANIMAX_LOGE(msg)
#endif

#if ANIMAX_MIN_LOG_LEVEL <= ANIMAX_LOG_LEVEL_ERROR
#define ANIMAX_LOGR(msg)                                                 \
  {                                                                      \
    ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM_ERROR, ANIMAX_LOG_IS_ON(ERROR)) \
        << msg;                                                          \
  }
#else
#define ANIMAX_LOGR(msg)
#endif

#if ANIMAX_MIN_LOG_LEVEL <= ANIMAX_LOG_LEVEL_FATAL
#define ANIMAX_LOGF(msg)                                                  \
  {                                                                       \
    ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM(FATAL), ANIMAX_LOG_IS_ON(FATAL)) \
        << msg;                                                           \
  }
#else
#define ANIMAX_LOGF(msg)
#endif

#define ANIMAX_CHECK(condition)                              \
  ANIMAX_LAZY_STREAM(ANIMAX_LOG_STREAM(FATAL), !(condition)) \
      << "Check failed: " #condition ". "

// for debug, if check failed, log fatal and abort
#ifndef DCHECK
#ifndef NDEBUG
#define DCHECK(condition) ANIMAX_CHECK(condition)
#else
// for release, do nothing
#define DCHECK(condition)                                              \
  true || (condition)                                                  \
      ? (void)0                                                        \
      : lynx::animax::LogMessageVoidify() &                            \
            lynx::animax::LogMessage("", 0, lynx::animax::LOG_VERBOSE) \
                .stream()
#endif
#endif

#define ANIMAX_NOTREACHED() ANIMAX_LOGF("")

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_LOG_LOG_H_
