// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_DARWIN_VIDEO_ASSET_DARWIN_H_
#define ANIMAX_SRC_VIDEO_DARWIN_VIDEO_ASSET_DARWIN_H_

#import <AVFoundation/AVFoundation.h>

#include <vector>

#include "src/resource/asset/video_asset.h"
#include "src/video/darwin/frame_info.h"

namespace lynx {
namespace animax {

class VideoAssetDarwin : public VideoAsset {
 public:
  explicit VideoAssetDarwin(VideoAssetModel model);
  ~VideoAssetDarwin() override;
  bool PrepareFrameData(const std::string& video_path) override;
  bool PrepareFrameData(std::unique_ptr<RawData> raw_data) override { return false; }
  int32_t GetFrameCount() const override { return frame_infos_.size(); }

  CMFormatDescriptionRef GetFormatDescription() const { return desc_; }
  const FrameInfo& GetFrameInfo(size_t index) const { return frame_infos_[index]; }
  char* GetFrameData() const { return (char*)[frames_ bytes]; }
  uint32_t GetFrameDataLength() const { return [frames_ length]; }
  int GetPrevKeyFrame(const int32_t frame) const override;

 private:
  static bool IsKeyFrame(CMSampleBufferRef current_sample_buffer);

  bool PrepareFrameData(NSString* file_path);
  void ComputePresentationIndex(std::vector<std::pair<double, uint32_t>>& gop,
                                uint32_t& sorted_num);
  bool IsKeyFramesValid() const;

  CMFormatDescriptionRef desc_ = nullptr;
  std::vector<int32_t> key_frames_;
  std::vector<FrameInfo> frame_infos_;
  NSMutableData* frames_ = nil;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_VIDEO_ASSET_DARWIN_H_
