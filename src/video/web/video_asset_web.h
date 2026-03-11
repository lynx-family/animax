// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_WEB_VIDEO_ASSET_WEB_H_
#define ANIMAX_SRC_VIDEO_WEB_VIDEO_ASSET_WEB_H_

#include "include/base/macros.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/asset/video_asset_model.h"

namespace lynx {
namespace animax {

class VideoAssetWeb : public VideoAsset {
 public:
  explicit VideoAssetWeb(VideoAssetModel model);
  ~VideoAssetWeb() override = default;

  bool PrepareFrameData(const std::string& video_path) override;
  int32_t GetFrameCount() const override { return frame_count_; }

  std::vector<uint8_t> GetVideoParameterSets() const override;
  std::vector<uint8_t> GetFrameRawData(int32_t frame) const override;
  int GetPrevKeyFrame(const int32_t frame) const override;

 private:
  int32_t frame_count_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_WEB_VIDEO_ASSET_WEB_H_
