// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_VIDEO_PLAYER_LISTENER_H_
#define ANIMAX_SRC_VIDEO_VIDEO_PLAYER_LISTENER_H_

#include <string>

#include "src/player/animax_event.h"

namespace lynx {
namespace animax {

class VideoPlayerListener {
 public:
  virtual ~VideoPlayerListener() = default;

  void OnVideoPlayerError(const std::string& err_msg) {
    OnVideoPlayerError(EventError::kVideoPlayerError, err_msg);
  }

  virtual void OnVideoPlayerError(EventError error,
                                  const std::string& err_msg) = 0;

  virtual void OnVideoPlayerWarning(EventWarning warning,
                                    const std::string& warning_msg) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_VIDEO_PLAYER_LISTENER_H_
