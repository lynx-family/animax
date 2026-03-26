// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_LOG_ANIMAX_LOG_LEVEL_H_
#define ANIMAX_SRC_BASE_LOG_ANIMAX_LOG_LEVEL_H_

#define ANIMAX_LOG_LEVEL_VERBOSE 0
#define ANIMAX_LOG_LEVEL_DEBUG 1
#define ANIMAX_LOG_LEVEL_INFO 2
#define ANIMAX_LOG_LEVEL_WARNING 3
#define ANIMAX_LOG_LEVEL_ERROR 4
#define ANIMAX_LOG_LEVEL_FATAL 5
#define ANIMAX_LOG_LEVEL_NUM 6

namespace lynx {
namespace animax {

enum AnimaXLogLevel {
  LOG_VERBOSE = 0,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_FATAL,
  LOG_NUM_SEVERITIES
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_LOG_ANIMAX_LOG_LEVEL_H_
