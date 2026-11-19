// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// cspell:ignore NOPTS linesize

#include "ffmpeg_decoder_web.h"

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace lynx {
namespace animax {
namespace video_wasm {

constexpr size_t kAVIOBufSize = 4096;

FFmpegDecoderWeb::~FFmpegDecoderWeb() {
  if (codec_context) {
    avcodec_free_context(&codec_context);
  }
  if (format_context) {
    auto* io_ctx = format_context->pb;
    if (io_ctx) {
      avio_context_free(&io_ctx);
      format_context->pb = nullptr;
    }
    avformat_close_input(&format_context);
  }
}

int ReadVideoData(void* opaque, uint8_t* buf, int buf_size) {
  auto* decoder = static_cast<FFmpegDecoderWeb*>(opaque);
  if (!decoder) {
    return AVERROR(EINVAL);
  }
  if (decoder->data_position >= decoder->data_size) {
    return AVERROR_EOF;
  }
  int64_t remaining = decoder->data_size - decoder->data_position;
  int64_t to_read = std::min(static_cast<int64_t>(buf_size), remaining);
  memcpy(buf, decoder->data + decoder->data_position,
         static_cast<size_t>(to_read));
  decoder->data_position += to_read;
  return static_cast<int>(to_read);
}

int64_t SeekVideoData(void* opaque, int64_t offset, int whence) {
  auto* decoder = static_cast<FFmpegDecoderWeb*>(opaque);
  if (!decoder) {
    return 0;
  }

  int64_t new_pos = 0;
  switch (whence) {
    case SEEK_SET:
      new_pos = offset;
      break;
    case SEEK_CUR:
      new_pos = decoder->data_position + offset;
      break;
    case SEEK_END:
      new_pos = decoder->data_size + offset;
      break;
    default:
      return AVERROR(EINVAL);
  }
  if (new_pos < 0 || new_pos > decoder->data_size) {
    return AVERROR(EINVAL);
  }
  decoder->data_position = new_pos;
  return new_pos;
}

FFmpegDecoderWeb* CreateVideoDecoder(const uint8_t* data, int32_t size) {
  if (!data || size <= 0) {
    return nullptr;
  }

  auto decoder = new FFmpegDecoderWeb();
  decoder->data = data;
  decoder->data_size = size;

  decoder->format_context = avformat_alloc_context();
  if (!decoder->format_context) {
    delete decoder;
    return nullptr;
  }

  uint8_t* io_buffer = static_cast<uint8_t*>(av_malloc(kAVIOBufSize));
  if (!io_buffer) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to allocate AVIO buffer");
    delete decoder;
    return nullptr;
  }

  AVIOContext* avio_ctx =
      avio_alloc_context(io_buffer, kAVIOBufSize, 0, decoder, ReadVideoData,
                         nullptr, SeekVideoData);
  if (!avio_ctx) {
    av_free(io_buffer);
    delete decoder;
    return nullptr;
  }
  avio_ctx->seekable = AVIO_SEEKABLE_NORMAL;
  decoder->format_context->pb = avio_ctx;
  if (avformat_open_input(&decoder->format_context, "data:", nullptr,
                          nullptr) != 0) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to open input from memory");
    delete decoder;
    return nullptr;
  }

  if (avformat_find_stream_info(decoder->format_context, nullptr) < 0) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to find stream info");
    delete decoder;
    return nullptr;
  }

  decoder->video_stream_index = -1;
  for (unsigned i = 0; i < decoder->format_context->nb_streams; i++) {
    if (decoder->format_context->streams[i]->codecpar->codec_type ==
        AVMEDIA_TYPE_VIDEO) {
      decoder->video_stream_index = static_cast<int>(i);
      break;
    }
  }
  if (decoder->video_stream_index == -1) {
    ANIMAX_VIDEO_WASM_LOGE("No video stream found");
    delete decoder;
    return nullptr;
  }

  auto* stream = decoder->format_context->streams[decoder->video_stream_index];
  auto* codec_params = stream->codecpar;
  const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
  if (!codec) {
    ANIMAX_VIDEO_WASM_LOGE("Unsupported codec");
    delete decoder;
    return nullptr;
  }

  decoder->width = codec_params->width;
  decoder->height = codec_params->height;
  decoder->frame_rate = av_q2d(stream->r_frame_rate);
  decoder->codec_context = avcodec_alloc_context3(codec);
  if (!decoder->codec_context ||
      avcodec_parameters_to_context(decoder->codec_context, codec_params) < 0) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to copy codec parameters");
    delete decoder;
    return nullptr;
  }

  decoder->codec_context->pkt_timebase = AV_TIME_BASE_Q;
  if (avcodec_open2(decoder->codec_context, codec, nullptr) < 0) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to open codec");
    delete decoder;
    return nullptr;
  }

  return decoder;
}

void DestroyVideoDecoder(FFmpegDecoderWeb* decoder) { delete decoder; }

bool PrepareFrameData(FFmpegDecoderWeb* decoder,
                      std::vector<int32_t>* key_frames,
                      std::vector<std::vector<uint8_t>>* frames_data,
                      std::vector<int64_t>* frame_pts) {
  if (!decoder || !key_frames || !frames_data || !frame_pts ||
      !decoder->format_context) {
    return false;
  }

  auto packet = av_packet_alloc();
  if (!packet) {
    return false;
  }

  int index = 0;
  while (av_read_frame(decoder->format_context, packet) >= 0) {
    if (packet->stream_index == decoder->video_stream_index) {
      if ((packet->flags & AV_PKT_FLAG_KEY) != 0) {
        key_frames->push_back(index);
      }
      frames_data->emplace_back(packet->data, packet->data + packet->size);
      auto time_base =
          decoder->format_context->streams[decoder->video_stream_index]
              ->time_base;
      frame_pts->push_back(
          packet->pts == AV_NOPTS_VALUE
              ? AV_NOPTS_VALUE
              : av_rescale_q(packet->pts, time_base, AV_TIME_BASE_Q));
      index++;
    }
    av_packet_unref(packet);
  }
  av_packet_free(&packet);

  return true;
}

bool Flush(FFmpegDecoderWeb* decoder) {
  if (!decoder || !decoder->codec_context) {
    return false;
  }
  avcodec_flush_buffers(decoder->codec_context);
  return true;
}

VideoSendStatus SendPacket(FFmpegDecoderWeb* decoder, const uint8_t* data,
                           int32_t size, int64_t pts) {
  if (!decoder || !decoder->codec_context || size < 0) {
    return VideoSendStatus::kError;
  }
  if (size == 0) {
    int ret = avcodec_send_packet(decoder->codec_context, nullptr);
    if (ret < 0) {
      ANIMAX_VIDEO_WASM_LOGE("Failed to send end-of-stream packet: %d", ret);
      return VideoSendStatus::kError;
    }
    return VideoSendStatus::kSuccess;
  }
  if (!data) {
    return VideoSendStatus::kError;
  }

  auto packet = av_packet_alloc();
  if (!packet) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to allocate video packet");
    return VideoSendStatus::kError;
  }
  int ret = av_new_packet(packet, size);
  if (ret < 0) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to allocate video packet data: %d", ret);
    av_packet_free(&packet);
    return VideoSendStatus::kError;
  }
  memcpy(packet->data, data, static_cast<size_t>(size));
  packet->pts = pts;

  ret = avcodec_send_packet(decoder->codec_context, packet);
  av_packet_free(&packet);

  if (ret < 0) {
    ANIMAX_VIDEO_WASM_LOGE("Failed to send video packet: %d", ret);
    return VideoSendStatus::kError;
  }
  return VideoSendStatus::kSuccess;
}

VideoDecodeStatus ReceiveFrame(FFmpegDecoderWeb* decoder, AVFrame*& output) {
  if (!decoder || !decoder->codec_context) {
    return VideoDecodeStatus::kError;
  }
  auto frame = av_frame_alloc();
  if (!frame) {
    return VideoDecodeStatus::kError;
  }

  int ret = avcodec_receive_frame(decoder->codec_context, frame);
  if (ret == 0) {
    output = frame;
    return VideoDecodeStatus::kFrameReady;
  }
  av_frame_free(&frame);
  if (ret == AVERROR(EAGAIN)) {
    return VideoDecodeStatus::kNeedInput;
  }
  if (ret == AVERROR_EOF) {
    return VideoDecodeStatus::kEndOfStream;
  }
  ANIMAX_VIDEO_WASM_LOGE("Failed to receive video frame: %d", ret);
  return VideoDecodeStatus::kError;
}

bool GetFrameData(AVFrame* frame, int32_t* width, int32_t* height,
                  int32_t* format, int32_t (*yuv_linesize)[3],
                  uint8_t* (*yuv_data)[3], int64_t* pts) {
  if (!frame || !width || !height || !format || !yuv_linesize || !yuv_data ||
      !pts) {
    return false;
  }

  *width = frame->width;
  *height = frame->height;
  *pts = frame->pts;

  for (int i = 0; i < 3; i++) {
    (*yuv_linesize)[i] = frame->linesize[i];
    (*yuv_data)[i] = frame->data[i];
  }

  switch (static_cast<AVPixelFormat>(frame->format)) {
    case AV_PIX_FMT_YUV420P:
      *format = 1;
      break;
    case AV_PIX_FMT_YUV422P:
      *format = 2;
      break;
    case AV_PIX_FMT_YUV444P:
      *format = 3;
      break;
    default:
      *format = 0;
      break;
  }

  return true;
}

void ReleaseFrame(AVFrame* frame) {
  if (frame) {
    av_frame_free(&frame);
  }
}

}  // namespace video_wasm
}  // namespace animax
}  // namespace lynx
