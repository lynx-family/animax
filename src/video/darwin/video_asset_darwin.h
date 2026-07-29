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
  const FrameInfo& GetFrameInfo(size_t decode_frame) const { return frame_infos_[decode_frame]; }
  char* GetFrameData() const { return (char*)[frames_ bytes]; }
  uint32_t GetFrameDataLength() const { return [frames_ length]; }
  // TODO(caitan): Darwin currently returns the decoding start frame, not just the
  // previous key frame, and will rename this API after all platforms adopt decode-start-frame
  // semantics.
  int GetPrevKeyFrame(const int32_t decode_frame) const override;
  int GetDecodeFrame(const int32_t presentation_frame) const {
    return presentation_frame_to_decode_frame_[presentation_frame];
  }

 private:
  static bool IsKeyFrame(CMSampleBufferRef current_sample_buffer);

  bool PrepareFrameData(NSString* file_path);
  void ComputePresentationIndex(std::vector<std::pair<double, uint32_t>>& gop,
                                uint32_t& sorted_num);
  void ComputePresentationFrameToDecodeFrame();
  void ComputeDecodeStartFrames();
  bool IsKeyFramesValid() const;

  CMFormatDescriptionRef desc_ = nullptr;
  std::vector<int32_t> key_frames_;
  std::vector<FrameInfo> frame_infos_;
  std::vector<int32_t> presentation_frame_to_decode_frame_;
  std::vector<int32_t> decode_start_frames_;
  NSMutableData* frames_ = nil;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_VIDEO_ASSET_DARWIN_H_
