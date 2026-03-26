// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_H_
#define ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_H_

#include <atomic>
#include <memory>

#include "src/audio/audio_controller.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/audio_asset_model.h"

namespace lynx {
namespace animax {

class AudioAsset : public BaseAsset<AudioAsset> {
 public:
  /**
   * Constructs a default AudioAsset.
   */
  AudioAsset() = default;

  static std::shared_ptr<AudioAsset> Make(AudioAssetModel model);

  /**
   * Constructs a AudioAsset with a specified AudioAssetModel.
   *
   * @param model AudioAssetModel used to initialize the AudioAsset.
   */
  explicit AudioAsset(AudioAssetModel model);

  ~AudioAsset() override = default;

  /**
   * Retrieves the current AudioAssetModel.
   *
   * @return Constant reference to the AudioAssetModel.
   */
  const AudioAssetModel& Model() const;

  /**
   * Replaces the current AudioAssetModel with a new model and unloads the
   * associated audio.
   *
   * @param info New AudioAssetModel to replace the current one.
   */
  void ResetModel(AudioAssetModel info);

  /**
   * Loads the audio from a local path.
   *
   * @param file_path Local file system path to the audio file.
   */
  virtual void LoadLocal(const std::string& file_path) = 0;

  bool IsValid();

 protected:
  std::atomic<bool> is_valid_ = false;

 private:
  std::string file_path_;
  AudioAssetModel model_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_H_
