// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_VIDEO_ASSET_FFMPEG_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_VIDEO_ASSET_FFMPEG_H_

#include <memory>
#include <string>
#include <vector>

#include "src/resource/asset/video_asset.h"

namespace lynx {
namespace animax {

class VideoAssetFFmpeg : public VideoAsset {
 public:
  explicit VideoAssetFFmpeg(VideoAssetModel model);
  ~VideoAssetFFmpeg() override = default;
  bool PrepareFrameData(const std::string& video_path) override;
  int32_t GetFrameCount() const override { return frame_count_; }
  int GetPrevKeyFrame(const int32_t frame) const override;
  std::vector<uint8_t> GetVideoParameterSets() const override;
  std::vector<uint8_t> GetFrameRawData(int32_t frame) const override;
  const std::string& GetVideoPath() const { return video_path_; }

 private:
  std::vector<int32_t> key_frames_;
  std::vector<std::vector<uint8_t>> frames_data_;
  int32_t frame_count_ = 0;
  std::string video_path_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_VIDEO_ASSET_FFMPEG_H_
