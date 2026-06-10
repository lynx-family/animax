// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/darwin/video_asset_darwin.h"

#include "base/include/string/string_utils.h"
#include "src/base/log/log.h"
#include "src/resource/asset/asset.h"
#include "src/video/video_player.h"

#include "src/base/monitor/trace_event.h"

namespace lynx {
namespace animax {

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return std::make_shared<VideoAssetDarwin>(std::move(model));
}

VideoAssetDarwin::VideoAssetDarwin(VideoAssetModel model) : VideoAsset(std::move(model)) {}

VideoAssetDarwin::~VideoAssetDarwin() {
  if (desc_) {
    CFRelease(desc_);
  }
}

bool VideoAssetDarwin::PrepareFrameData(const std::string& video_path) {
  ANIMAX_TRACE_EVENT_ASSET_ID(kPrepareAlphaVideoFrameData, Model().id);
  NSString* file_path = [NSString stringWithUTF8String:video_path.c_str()];
  if (!file_path) {
    ANIMAX_LOGE("prepare frame data fail, file path is empty");
    return false;
  }
  NSURL* file_url = [NSURL fileURLWithPath:file_path];
  if (!file_url) {
    const char* path = [file_path UTF8String];
    if (path) {
      ANIMAX_LOGE("prepare frame data fail, file path: " << std::string(path));
    } else {
      ANIMAX_LOGE("prepare frame data fail, file url is empty");
    }
    return false;
  }
  AVAsset* asset = [AVAsset assetWithURL:file_url];
  AVAssetTrack* video_track = [[asset tracksWithMediaType:AVMediaTypeVideo] firstObject];
  CGSize video_size = [video_track naturalSize];
  video_width_ = (int32_t)video_size.width;
  video_height_ = (int32_t)video_size.height;
  frame_rate_ = [video_track nominalFrameRate];
  NSArray* formats = video_track.formatDescriptions;
  for (NSUInteger i = 0; i < [formats count]; ++i) {
    CMFormatDescriptionRef desc = (__bridge CMFormatDescriptionRef)formats[i];
    if (CMFormatDescriptionGetMediaType(desc) == kCMMediaType_Video) {
      desc_ = desc;
      CFRetain(desc_);
      break;
    }
  }
  if (!desc_) {
    return false;
  }
  NSError* error = nil;
  AVAssetReader* reader = [[AVAssetReader alloc] initWithAsset:asset error:&error];
  if (!reader || error) {
    const char* error_msg = [[error description] UTF8String];
    if (error_msg) {
      ANIMAX_LOGE("can't create video asset reader: " << std::string(error_msg));
    } else {
      ANIMAX_LOGE("can't create video asset reader");
    }
    return false;
  }
  AVAssetReaderTrackOutput* output = [[AVAssetReaderTrackOutput alloc] initWithTrack:video_track
                                                                      outputSettings:nil];
  [reader addOutput:output];
  [reader startReading];
  NSNumber* file_size = nil;
  [file_url getResourceValue:&file_size forKey:NSURLFileSizeKey error:nil];
  NSMutableData* frames = [NSMutableData dataWithCapacity:[file_size unsignedIntegerValue]];
  CMSampleBufferRef sample_buffer;
  CMBlockBufferRef block_buffer;
  size_t total_size = 0;
  size_t frame_size = 0;
  char* frame_data = nullptr;
  std::vector<std::pair<double, uint32_t>>
      gop;                  // Group of Pictures, first is presentation time, second is decode index
  uint32_t sorted_num = 0;  // number of frame info which has correct presetation index
  while ((sample_buffer = [output copyNextSampleBuffer])) {
    block_buffer = CMSampleBufferGetDataBuffer(sample_buffer);
    if (block_buffer) {
      if (IsKeyFrame(sample_buffer)) {
        ComputePresentationIndex(gop, sorted_num);
        key_frames_.push_back(frame_infos_.size());
      }
      frame_size = 0;
      frame_data = nullptr;
      CMBlockBufferGetDataPointer(block_buffer, 0, nullptr, &frame_size, &frame_data);
      [frames appendBytes:frame_data length:frame_size];
      CMSampleTimingInfo timing_info{
          .duration = CMSampleBufferGetDuration(sample_buffer),
          .presentationTimeStamp = CMSampleBufferGetPresentationTimeStamp(sample_buffer),
          .decodeTimeStamp = CMSampleBufferGetDecodeTimeStamp(sample_buffer),
      };
      double presentation_time_s = CMTimeGetSeconds(timing_info.presentationTimeStamp);
      gop.emplace_back(presentation_time_s, frame_infos_.size());
      frame_infos_.emplace_back(total_size, total_size + frame_size, timing_info);
      total_size += frame_size;
    }
    CMSampleBufferInvalidate(sample_buffer);
    CFRelease(sample_buffer);
  }
  [reader cancelReading];

  ComputePresentationIndex(gop, sorted_num);

  ANIMAX_LOGI("video size: " << total_size << " bytes, frame count: " << frame_infos_.size()
                             << ", key frame count: " << key_frames_.size());

  if (GetVideoWidth() > 0 && GetVideoHeight() > 0 && IsKeyFramesValid() && GetFrameRate() > 0.f) {
    frames_ = frames;
    is_valid_.store(true);
  }
  return IsValid();
}

void VideoAssetDarwin::ComputePresentationIndex(std::vector<std::pair<double, uint32_t>>& gop,
                                                uint32_t& sorted_num) {
  std::sort(gop.begin(), gop.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
  const auto gop_size = gop.size();
  for (auto i = 0; i < gop_size; ++i) {
    const auto presentation_index = sorted_num + i;
    const auto decode_index = gop[i].second;
    frame_infos_[decode_index].presentation_index_ = presentation_index;
  }
  sorted_num += gop_size;
  gop.clear();
}

bool VideoAssetDarwin::IsKeyFrame(CMSampleBufferRef current_sample_buffer) {
  DCHECK(current_sample_buffer);
  bool is_key_frame = false;
  CFArrayRef attachments =
      CMSampleBufferGetSampleAttachmentsArray(current_sample_buffer, 0 /*createIfNecessary*/);
  if (!attachments || !CFArrayGetCount(attachments)) {
    is_key_frame = true;
  } else {
    CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(attachments, 0);
    CFBooleanRef not_sync =
        (CFBooleanRef)CFDictionaryGetValue(dict, kCMSampleAttachmentKey_NotSync);
    // the key-value doesn't exists, or the value is false
    is_key_frame = !not_sync || !CFBooleanGetValue(not_sync);
  }
  return is_key_frame;
}

bool VideoAssetDarwin::IsKeyFramesValid() const {
  return !key_frames_.empty() && (key_frames_[0] == 0);
}

int VideoAssetDarwin::GetPrevKeyFrame(const int32_t frame) const {
  auto it = std::upper_bound(key_frames_.begin(), key_frames_.end(), frame);
  return *--it;
}

}  // namespace animax
}  // namespace lynx
