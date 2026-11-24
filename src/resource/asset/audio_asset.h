// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_H_
#define ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_H_

#include <atomic>
#include <memory>

#include "include/base/macros.h"
#include "include/resource/loader_error.h"
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
  ANIMAX_EXPORT const AudioAssetModel& Model() const;

  /**
   * Replaces the current AudioAssetModel without changing prepared audio data
   * or its validity. This does not load audio from the new model's source.
   *
   * @param info New AudioAssetModel to replace the current one.
   */
  ANIMAX_EXPORT void ResetModel(AudioAssetModel info);

  /**
   * Loads the audio from a local path.
   *
   * @param file_path Local file system path to the audio file.
   */
  virtual void LoadLocal(const std::string& file_path) = 0;

  // Independent loading only publishes complete audio. Each call uses a fresh
  // candidate; backends opt in without tightening legacy playback behavior.
  virtual LoaderError PrepareForPreload(const std::string& file_path) {
    return {kNoSuchLoader, "Independent audio preparation is unavailable"};
  }

  ANIMAX_EXPORT bool IsValid();

  // Owned CPU storage only. Platform decoder/OS allocations are not included.
  virtual int64_t GetMemoryUsageBytes() const;

 protected:
  std::atomic<bool> is_valid_ = false;

 private:
  std::string file_path_;
  AudioAssetModel model_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_H_
