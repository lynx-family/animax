// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/harmony/video_asset_harmony.h"

#include <fcntl.h>
#include <multimedia/player_framework/native_avcapability.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iomanip>
#include <sstream>

#include "src/base/log/log.h"
#include "src/resource/asset/video_asset_model.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return std::make_shared<VideoAssetHarmony>(std::move(model));
}

VideoAssetHarmony::VideoAssetHarmony(VideoAssetModel model)
    : VideoAsset(std::move(model)) {}

VideoAssetHarmony::~VideoAssetHarmony() {
  if (av_demuxer_) {
    OH_AVDemuxer_Destroy(av_demuxer_);
    av_demuxer_ = nullptr;
  }
  if (av_source_) {
    OH_AVSource_Destroy(av_source_);
    av_source_ = nullptr;
  }
}

bool VideoAssetHarmony::InitVideoInfo() {
  if (!data_.video_format) {
    ANIMAX_LOGE("The video format is not found.")
    return false;
  }

  // Parse the video width, height and frame rate
  OH_AVFormat_GetIntValue(data_.video_format.get(), OH_MD_KEY_WIDTH,
                          &video_width_);
  OH_AVFormat_GetIntValue(data_.video_format.get(), OH_MD_KEY_HEIGHT,
                          &video_height_);
  OH_AVFormat_GetDoubleValue(data_.video_format.get(), OH_MD_KEY_FRAME_RATE,
                             &frame_rate_);

  // Check the video parameter is valid
  return video_width_ > 0 && video_height_ > 0 && frame_rate_ > 0;
}

void VideoAssetHarmony::FindVideoTrack() {
  auto* file_format = OH_AVSource_GetSourceFormat(av_source_);
  if (!file_format) {
    ANIMAX_LOGE("OH_AVSource_GetSourceFormat fail");
    return;
  }

  // Loop through all video track
  int32_t track_count = 0;
  OH_AVFormat_GetIntValue(file_format, OH_MD_KEY_TRACK_COUNT, &track_count);
  for (int32_t index = 0; index < track_count; index++) {
    auto format = OH_AVSource_GetTrackFormat(av_source_, index);
    int32_t track_type;
    OH_AVFormat_GetIntValue(format, OH_MD_KEY_TRACK_TYPE, &track_type);
    // Only pick update video track info
    if (track_type == OH_MediaType::MEDIA_TYPE_VID) {
      OH_AVDemuxer_SelectTrackByID(av_demuxer_, index);
      data_.track_index = index;
      data_.video_format =
          std::shared_ptr<OH_AVFormat>(format, OH_AVFormat_Destroy);
    }
  }
  OH_AVFormat_Destroy(file_format);
}

bool VideoAssetHarmony::ReadSampleBuffers(int64_t file_size) {
  bool has_end = false;
  int32_t current_offset = 0;
  OH_AVDemuxer_SelectTrackByID(av_demuxer_, data_.track_index);
  auto* cache_buffer = OH_AVBuffer_Create(file_size);
  while (!has_end) {
    OH_AVErrCode err_code = OH_AVDemuxer_ReadSampleBuffer(
        av_demuxer_, data_.track_index, cache_buffer);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGI("OH_AVDemuxer_ReadSampleBuffer fail");
      return false;
    }

    OH_AVCodecBufferAttr attr;
    err_code = OH_AVBuffer_GetBufferAttr(cache_buffer, &attr);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGI("OH_AVBuffer_GetBufferAttr fail");
      return false;
    }

    // End of stream, stop the read loop
    if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
      has_end = true;
      continue;
    }

    if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
      data_.keyframe_index.push_back(data_.frame_list.size());
    }

    VideoFrameInfo frame = {.begin = current_offset,
                            .size = attr.size,
                            .timestamp = attr.pts,
                            .flags = attr.flags};
    // Use the relative offset to indicate the data position in buffer_data.
    current_offset += attr.size;
    data_.frame_list.push_back(frame);

    auto* buffer_addr = OH_AVBuffer_GetAddr(cache_buffer);
    data_.buffer_data.insert(data_.buffer_data.end(), buffer_addr,
                             buffer_addr + attr.size);
  }

  data_.frame_count = data_.frame_list.size();

  OH_AVBuffer_Destroy(cache_buffer);
  return true;
}

bool VideoAssetHarmony::PrepareFrameData(const std::string& video_path) {
  int fd = open(video_path.c_str(), O_RDONLY);
  if (fd == -1) {
    ANIMAX_LOGE("Failed to open file: " << video_path);
    return false;
  }

  struct stat file_stat;
  if (fstat(fd, &file_stat) == -1) {
    ANIMAX_LOGE("Failed to get file size: " << video_path);
    close(fd);
    return false;
  }
  int64_t file_size = file_stat.st_size;

  // Create AVSource by local file's fd
  av_source_ = OH_AVSource_CreateWithFD(fd, 0, file_size);

  if (!av_source_) {
    ANIMAX_LOGE("OH_AVSource_CreateWithFD fail");
    close(fd);
    return false;
  }

  av_demuxer_ = OH_AVDemuxer_CreateWithSource(av_source_);
  if (!av_demuxer_) {
    ANIMAX_LOGE("OH_AVDemuxer_CreateWithSource fail");
    return false;
  }

  FindVideoTrack();
  if (!InitVideoInfo()) {
    return false;
  }

  if (!ReadSampleBuffers(file_size)) {
    return false;
  }

  auto frame_count = data_.frame_count;
  auto keyframe_count = data_.keyframe_index.size();

  ANIMAX_LOGI("width: " << video_width_ << ", height: " << video_height_
                        << ", frame_rate: " << frame_rate_
                        << ", frame count:" << frame_count
                        << ", keyframe count:" << keyframe_count);

  if (frame_count > 0 && keyframe_count > 0) {
    is_valid_ = true;
    return true;
  }

  return true;
}

}  // namespace animax
}  // namespace lynx
