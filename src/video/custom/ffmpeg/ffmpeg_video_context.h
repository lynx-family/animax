// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_VIDEO_CONTEXT_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_VIDEO_CONTEXT_H_

#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace lynx {
namespace animax {

struct FormatContextDeleter {
  void operator()(AVFormatContext* context) const;
};

struct CodecContextDeleter {
  void operator()(AVCodecContext* context) const;
};

struct PacketDeleter {
  void operator()(AVPacket* packet) const;
};

struct FrameDeleter {
  void operator()(AVFrame* frame) const;
};

class FFmpegVideoContext {
 public:
  using FormatContextPtr =
      std::unique_ptr<AVFormatContext, FormatContextDeleter>;
  using CodecContextPtr = std::unique_ptr<AVCodecContext, CodecContextDeleter>;
  using PacketPtr = std::unique_ptr<AVPacket, PacketDeleter>;
  using FramePtr = std::unique_ptr<AVFrame, FrameDeleter>;

  explicit FFmpegVideoContext(const std::string& video_path);
  ~FFmpegVideoContext() = default;

  bool IsValid() const;

  int32_t GetWidth() const;
  int32_t GetHeight() const;
  double GetFrameRate() const;
  const FormatContextPtr& GetFormatContext() const { return format_context_; }
  const CodecContextPtr& GetCodecContext() const { return codec_context_; }
  AVStream* GetStream() const { return stream_; }

 private:
  FormatContextPtr format_context_;
  CodecContextPtr codec_context_;
  AVStream* stream_ = nullptr;
  bool is_valid_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_VIDEO_CONTEXT_H_
