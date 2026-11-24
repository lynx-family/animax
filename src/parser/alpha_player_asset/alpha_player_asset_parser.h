// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PARSER_ALPHA_PLAYER_ASSET_ALPHA_PLAYER_ASSET_PARSER_H_
#define ANIMAX_SRC_PARSER_ALPHA_PLAYER_ASSET_ALPHA_PLAYER_ASSET_PARSER_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "include/base/macros.h"

namespace lynx {
namespace animax {
class AudioAsset;
class CompositionModel;
class VideoAsset;
struct VideoAssetModel;

struct AlphaPlayerConfig {
  std::string video_path;
  std::array<int32_t, 4> rgb_frame{};
  std::array<int32_t, 4> alpha_frame{};
};

class AlphaPlayerAssetParser final {
 public:
  // Reads configuration without preparing media. The final model still needs
  // the independently prepared video's frame count, frame rate and dimensions.
  static std::shared_ptr<const AlphaPlayerConfig> ParseConfig(
      const std::string &unzip_path, const char *config, size_t length);

  // Builds a fresh timeline without loading media or modifying either asset.
  // The caller first validates the description with
  // VideoAsset::ResolvePreparedModel and commits resolved_video_model to video
  // only after this succeeds.
  ANIMAX_EXPORT static std::shared_ptr<CompositionModel> BuildFromPreparedVideo(
      const AlphaPlayerConfig &config, const std::shared_ptr<VideoAsset> &video,
      const VideoAssetModel &resolved_video_model,
      const std::shared_ptr<AudioAsset> &audio, float scale,
      bool enable_audio = false);

  static std::shared_ptr<CompositionModel> Parse(const std::string &unzip_path,
                                                 const char *config,
                                                 size_t length, float scale,
                                                 bool enable_audio = false);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PARSER_ALPHA_PLAYER_ASSET_ALPHA_PLAYER_ASSET_PARSER_H_
