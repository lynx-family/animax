// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_HARMONY_VIDEO_ASSET_HARMONY_H_
#define ANIMAX_SRC_VIDEO_HARMONY_VIDEO_ASSET_HARMONY_H_

#include <multimedia/player_framework/native_avcodec_videodecoder.h>
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avsource.h>

#include <queue>

#include "src/resource/asset/video_asset.h"
#include "src/resource/asset/video_asset_model.h"

namespace lynx {
namespace animax {

struct VideoFrameInfo {
  int32_t begin;
  int32_t size;
  int64_t timestamp;
  uint32_t flags;
};

struct VideoData {
  // The buffer read from demuxer.
  std::vector<uint8_t> buffer_data;
  // The frame parameter, will pass to codec's input buffer
  std::vector<VideoFrameInfo> frame_list;
  // The list contains the keyframe's index in frame_list
  std::vector<int32_t> keyframe_index;
  // Total frame count on video
  int32_t frame_count;
  // The video format, will be used on codec's initialization.
  std::shared_ptr<OH_AVFormat> video_format;
  // The video track's index
  int32_t track_index;
};

class VideoAssetHarmony : public VideoAsset {
 public:
  explicit VideoAssetHarmony(VideoAssetModel model);
  ~VideoAssetHarmony() override;

  bool PrepareFrameData(const std::string& video_path) override;

  int32_t GetFrameCount() const override { return data_.frame_count; }

  VideoData& GetVideoData() { return data_; }

 private:
  // Find video track by av format
  void FindVideoTrack();
  // Read video basic parameters
  bool InitVideoInfo();
  // Read the video raw buffer by demuxer and save into buffer_data.
  bool ReadSampleBuffers(int64_t file_size);

  OH_AVSource* av_source_ = nullptr;
  OH_AVDemuxer* av_demuxer_ = nullptr;
  VideoData data_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_HARMONY_VIDEO_ASSET_HARMONY_H_
