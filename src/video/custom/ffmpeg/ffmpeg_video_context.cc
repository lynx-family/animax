// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/custom/ffmpeg/ffmpeg_video_context.h"

#include "src/base/log/log.h"
#include "src/video/custom/ffmpeg/ffmpeg_lib_loader.h"

namespace lynx {
namespace animax {

void FormatContextDeleter::operator()(AVFormatContext* context) const {
  FFmpegLibLoader::Instance().FreeFormatContext(&context);
}

void CodecContextDeleter::operator()(AVCodecContext* context) const {
  FFmpegLibLoader::Instance().FreeCodecContext(&context);
}

void PacketDeleter::operator()(AVPacket* packet) const {
  FFmpegLibLoader::Instance().FreePacket(&packet);
}

void FrameDeleter::operator()(AVFrame* frame) const {
  FFmpegLibLoader::Instance().FreeFrame(&frame);
}

FFmpegVideoContext::FFmpegVideoContext(const std::string& video_path) {
  ANIMAX_LOGI("Creating video context for: " << video_path);

  if (!FFmpegLibLoader::Instance().IsValid()) {
    ANIMAX_LOGE("FFmpeg loader not initialized");
    return;
  }

  AVFormatContext* format_ctx_ptr = nullptr;
  if (FFmpegLibLoader::Instance().avformat_open_input_func_(
          &format_ctx_ptr, video_path.c_str(), nullptr, nullptr) != 0) {
    ANIMAX_LOGE("Failed to open video file: " << video_path);
    return;
  }
  format_context_ = FormatContextPtr(format_ctx_ptr, FormatContextDeleter());

  if (FFmpegLibLoader::Instance().avformat_find_stream_info_func_(
          format_context_.get(), nullptr) < 0) {
    ANIMAX_LOGE("Failed to find stream info");
    return;
  }

  int video_stream_index = -1;
  for (int i = 0; i < format_context_->nb_streams; i++) {
    if (format_context_->streams[i]->codecpar->codec_type ==
        AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }
  if (video_stream_index == -1) {
    ANIMAX_LOGE("No video stream found in file");
    return;
  }

  stream_ = format_context_->streams[video_stream_index];

  auto codec_params = format_context_->streams[video_stream_index]->codecpar;
  const AVCodec* codec = FFmpegLibLoader::Instance().avcodec_find_decoder_func_(
      codec_params->codec_id);
  if (!codec) {
    ANIMAX_LOGE(
        "Failed to find decoder for codec id: " << codec_params->codec_id);
    return;
  }

  AVCodecContext* codec_ctx_ptr =
      FFmpegLibLoader::Instance().avcodec_alloc_context3_func_(codec);
  if (!codec_ctx_ptr) {
    ANIMAX_LOGE("Failed to allocate codec context");
    return;
  }
  codec_context_ = CodecContextPtr(codec_ctx_ptr, CodecContextDeleter());

  if (FFmpegLibLoader::Instance().avcodec_parameters_to_context_func_(
          codec_context_.get(), codec_params) < 0) {
    ANIMAX_LOGE("Failed to copy codec parameters");
    return;
  }

  if (FFmpegLibLoader::Instance().avcodec_open2_func_(codec_context_.get(),
                                                      codec, nullptr) < 0) {
    ANIMAX_LOGE("Failed to open codec");
    return;
  }

  is_valid_ = true;
  ANIMAX_LOGI("Video context created successfully");
}

bool FFmpegVideoContext::IsValid() const {
  return is_valid_ && codec_context_ != nullptr && format_context_ != nullptr &&
         stream_ != nullptr;
}

int32_t FFmpegVideoContext::GetWidth() const {
  if (!is_valid_ || !stream_ || !stream_->codecpar) {
    return 0;
  }
  return stream_->codecpar->width;
}

int32_t FFmpegVideoContext::GetHeight() const {
  if (!is_valid_ || !stream_ || !stream_->codecpar) {
    return 0;
  }
  return stream_->codecpar->height;
}

double FFmpegVideoContext::GetFrameRate() const {
  if (!is_valid_ || !stream_) {
    return 0.0;
  }
  return av_q2d(stream_->r_frame_rate);
}

}  // namespace animax
}  // namespace lynx
