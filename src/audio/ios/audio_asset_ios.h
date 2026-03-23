// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_IOS_AUDIO_ASSET_IOS_H_
#define ANIMAX_SRC_AUDIO_IOS_AUDIO_ASSET_IOS_H_

#include <AudioToolbox/AudioToolbox.h>

#include <string>
#include <vector>

#include "src/resource/asset/audio_asset.h"

namespace lynx {
namespace animax {

struct AudioInfo {
  int64_t duration = 0;  // us
  AudioStreamBasicDescription client_format{};
};

class AudioAssetIOS : public AudioAsset {
 public:
  explicit AudioAssetIOS(AudioAssetModel model);
  ~AudioAssetIOS() override = default;
  void LoadLocal(const std::string& file_path) override;

  const AudioInfo& GetAudioInfo() const;
  const std::vector<uint8_t>& GetAudioData() const;

 private:
  bool InitAudioFormatAndInfo(ExtAudioFileRef ext_audio_file);
  bool ReadAllFrames(ExtAudioFileRef ext_audio_file);

  AudioInfo info_;
  std::vector<uint8_t> audio_data_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_IOS_AUDIO_ASSET_IOS_H_
