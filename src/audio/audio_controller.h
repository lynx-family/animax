// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_AUDIO_CONTROLLER_H_
#define ANIMAX_SRC_AUDIO_AUDIO_CONTROLLER_H_

#include <memory>

#include "src/player/animax_player_context.h"

namespace lynx {
namespace animax {
class AudioPlayer;
class AudioAsset;
/**
 * The AudioPlayer is created and held by GPU Thread.
 */
class AudioController : public std::enable_shared_from_this<AudioController> {
 public:
  explicit AudioController(std::unique_ptr<AudioPlayer> player);
  void Init(const std::string& file_path);
  void OnResume();
  void OnPause();
  void OnProgress(double progress);
  void SetVolume(double volume);

 private:
  bool can_play_ = false;
  bool is_playing_ = false;
  std::unique_ptr<AudioPlayer> audio_player_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_AUDIO_CONTROLLER_H_
