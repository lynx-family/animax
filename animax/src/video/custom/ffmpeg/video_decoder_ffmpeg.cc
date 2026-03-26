// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/custom/ffmpeg/video_decoder_ffmpeg.h"

#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/video/custom/ffmpeg/ffmpeg_lib_loader.h"
#include "src/video/custom/ffmpeg/ffmpeg_video_context.h"
#include "src/video/custom/ffmpeg/video_asset_ffmpeg.h"
#include "src/video/custom/yuv_frame_info.h"

namespace lynx {
namespace animax {

VideoDecoderFFmpeg::VideoDecoderFFmpeg() = default;

VideoDecoderFFmpeg::~VideoDecoderFFmpeg() { DestroyDecoder(); }

bool VideoDecoderFFmpeg::IsValid() {
  return FFmpegLibLoader::Instance().IsValid() && video_context_ != nullptr &&
         video_context_->IsValid();
}

bool VideoDecoderFFmpeg::CreateDecoder(std::shared_ptr<VideoAsset> asset) {
  ANIMAX_LOGI("Creating decoder");

  auto ffmpeg_asset = std::static_pointer_cast<VideoAssetFFmpeg>(asset);
  if (!ffmpeg_asset) {
    ANIMAX_LOGE("Invalid video asset");
    return false;
  }

  const auto& video_path = ffmpeg_asset->GetVideoPath();
  video_context_ = std::make_unique<FFmpegVideoContext>(video_path);
  if (!IsValid()) {
    ANIMAX_LOGE("Failed to create video context");
    return false;
  }

  ANIMAX_LOGI("Decoder created successfully");
  return true;
}

bool VideoDecoderFFmpeg::DestroyDecoder() {
  ANIMAX_LOGI("Destroying decoder");
  video_context_.reset();
  return true;
}

std::shared_ptr<YUVFrameInfo> VideoDecoderFFmpeg::DecodeFrameData(
    std::vector<uint8_t>& data,
    const std::shared_ptr<YUVFrameInfo>& reusable_frame) {
  if (!IsValid()) {
    return nullptr;
  }

  DCHECK(!data.empty());

  auto frame = FFmpegLibLoader::Instance().DecodeFrameData(
      video_context_.get(), data.data(), data.size());
  if (!frame) {
    return nullptr;
  }

  auto yuv_info =
      reusable_frame ? reusable_frame : std::make_shared<YUVFrameInfo>();
  yuv_info->SetWidth(frame->width);
  yuv_info->SetHeight(frame->height);

  auto pix_format = static_cast<AVPixelFormat>(frame->format);
  switch (pix_format) {
    case AV_PIX_FMT_YUV420P:
      yuv_info->SetYUVPixFmt(YUVPixFmt::kYUVPixFmtI420);
      break;
    case AV_PIX_FMT_YUV422P:
      yuv_info->SetYUVPixFmt(YUVPixFmt::kYUVPixFmtI422);
      break;
    case AV_PIX_FMT_YUV444P:
      yuv_info->SetYUVPixFmt(YUVPixFmt::kYUVPixFmtI444);
      break;
    default:
      ANIMAX_LOGE("Invalid video pixel format " << pix_format);
      return nullptr;
  }

  for (int i = 0; i < YUVFrameInfo::kYUVChannels; i++) {
    yuv_info->SetLineSize(i, frame->linesize[i]);
    yuv_info->UpdateChannelData(i, frame->data[i]);
  }

  return yuv_info;
}

}  // namespace animax
}  // namespace lynx
