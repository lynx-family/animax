// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/custom/ffmpeg/ffmpeg_lib_loader.h"

#include <windows.h>

#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/video/custom/ffmpeg/ffmpeg_video_context.h"

namespace lynx {
namespace animax {

static void ffmpeg_log_callback_cstyle(void* avcl, int level, const char* fmt,
                                       va_list vl) {
  char log_buffer[1024];
  vsnprintf(log_buffer, sizeof(log_buffer), fmt, vl);
  ANIMAX_LOGI(std::string(log_buffer));
}

void FFmpegLibLoader::FreeFormatContext(AVFormatContext** ctx) {
  if (!IsValid() || !ctx || !*ctx) {
    return;
  }
  avformat_close_input_func_(ctx);
}

void FFmpegLibLoader::FreeCodecContext(AVCodecContext** ctx) {
  if (!IsValid() || !ctx || !*ctx) {
    return;
  }
  avcodec_free_context_func_(ctx);
}

void FFmpegLibLoader::FreePacket(AVPacket** packet) {
  if (!IsValid() || !packet || !*packet) {
    return;
  }
  av_packet_free_func_(packet);
}

void FFmpegLibLoader::FreeFrame(AVFrame** frame) {
  if (!IsValid() || !frame || !*frame) {
    return;
  }
  av_frame_free_func_(frame);
}

FFmpegLibLoader& FFmpegLibLoader::Instance() {
  static base::NoDestructor<FFmpegLibLoader> instance;
  return *instance;
}

FFmpegLibLoader::FFmpegLibLoader() {
  ThreadAssert::Assert(ThreadAssert::Type::kResource);
  if (!LoadLibrary()) {
    ANIMAX_LOGE("Failed to initialize FFmpeg library");
  }
}

bool FFmpegLibLoader::IsValid() const {
  return !dlls_.empty() && is_symbols_valid_;
}

FFmpegVideoContext::FramePtr FFmpegLibLoader::DecodeFrameData(
    const FFmpegVideoContext* video_context, const uint8_t* data,
    size_t data_size) {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  if (!IsValid() || !video_context || !video_context->IsValid() || !data) {
    return nullptr;
  }

  auto codec_ctx = video_context->GetCodecContext().get();

  FFmpegVideoContext::FramePtr frame(av_frame_alloc_func_(), FrameDeleter());
  if (!frame) {
    return nullptr;
  }

  FFmpegVideoContext::PacketPtr packet(av_packet_alloc_func_(),
                                       PacketDeleter());
  if (!packet) {
    return nullptr;
  }

  av_new_packet_func_(packet.get(), data_size);
  memcpy(packet->data, data, data_size);
  int ret = avcodec_send_packet_func_(codec_ctx, packet.get());
  if (ret < 0) {
    return nullptr;
  }

  ret = avcodec_receive_frame_func_(codec_ctx, frame.get());

  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
    return nullptr;
  }

  return frame;
}

bool FFmpegLibLoader::ReadVideoFramesData(
    const FFmpegVideoContext* video_context,
    std::vector<std::vector<uint8_t>>& frames_data,
    std::vector<int32_t>& key_frames) {
  if (!IsValid() || !video_context || !video_context->IsValid()) {
    return false;
  }

  auto format_ctx = video_context->GetFormatContext().get();
  auto stream = video_context->GetStream();

  FFmpegVideoContext::PacketPtr packet(av_packet_alloc_func_(),
                                       PacketDeleter());
  if (!packet) {
    return false;
  }

  int index = 0;
  while (av_read_frame_func_(format_ctx, packet.get()) >= 0) {
    if (packet->stream_index == stream->index) {
      if ((packet->flags & AV_PKT_FLAG_KEY) != 0) {
        key_frames.push_back(index);
      }

      auto& raw_data = frames_data.emplace_back(packet->size);
      memcpy(raw_data.data(), packet->data, packet->size);
      index++;
    }
    av_packet_unref_func_(packet.get());
  }

  return true;
}

bool FFmpegLibLoader::LoadLibrary() {
  ANIMAX_LOGI("Loading FFmpeg library");

  if (!dlls_.empty()) {
    ANIMAX_LOGI("Library already loaded");
    return true;
  }

  std::vector<std::string> candidate_dlls = {
      "ttffmpeg.dll",
      "lynxffmpeg.dll",
  };

  for (const auto& dll_name : candidate_dlls) {
    HMODULE dll = LoadLibraryExA(
        dll_name.c_str(), nullptr,
        LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (dll) {
      dlls_.push_back(dll);
      ANIMAX_LOGI(dll_name << " loaded successfully");
    }
  }

  if (dlls_.empty()) {
    ANIMAX_LOGE("Failed to load any FFmpeg dlls");
    return false;
  }

  ANIMAX_LOGI("ffmpeg.dll loaded successfully");
  return LoadAllSymbols();
}

bool FFmpegLibLoader::LoadAllSymbols() {
  ANIMAX_LOGI("Loading FFmpeg symbols");

  is_symbols_valid_ = false;

  if (dlls_.empty()) {
    ANIMAX_LOGE("dlls_ is empty before loading symbols");
    return false;
  }

  struct SymbolConfig {
    void** func_ptr;
    const char* symbol_name;
  };

  auto load_symbols = [this](const std::vector<SymbolConfig>& symbols) -> bool {
    for (const auto& symbol : symbols) {
      *symbol.func_ptr = nullptr;
      for (HMODULE dll : dlls_) {
        *symbol.func_ptr = GetProcAddress(dll, symbol.symbol_name);
        if (*symbol.func_ptr) {
          break;
        }
      }

      if (!*symbol.func_ptr) {
        ANIMAX_LOGE(
            "Failed to load symbol: " << std::string(symbol.symbol_name));
        return false;
      }
    }
    return true;
  };

  std::vector<SymbolConfig> symbols = {
      {reinterpret_cast<void**>(&avformat_open_input_func_),
       "avformat_open_input"},
      {reinterpret_cast<void**>(&avformat_close_input_func_),
       "avformat_close_input"},
      {reinterpret_cast<void**>(&avformat_find_stream_info_func_),
       "avformat_find_stream_info"},
      {reinterpret_cast<void**>(&avcodec_find_decoder_func_),
       "avcodec_find_decoder"},
      {reinterpret_cast<void**>(&avcodec_alloc_context3_func_),
       "avcodec_alloc_context3"},
      {reinterpret_cast<void**>(&avcodec_parameters_to_context_func_),
       "avcodec_parameters_to_context"},
      {reinterpret_cast<void**>(&avcodec_open2_func_), "avcodec_open2"},
      {reinterpret_cast<void**>(&avcodec_free_context_func_),
       "avcodec_free_context"},
      {reinterpret_cast<void**>(&av_packet_alloc_func_), "av_packet_alloc"},
      {reinterpret_cast<void**>(&av_new_packet_func_), "av_new_packet"},
      {reinterpret_cast<void**>(&av_packet_free_func_), "av_packet_free"},
      {reinterpret_cast<void**>(&av_frame_alloc_func_), "av_frame_alloc"},
      {reinterpret_cast<void**>(&av_frame_free_func_), "av_frame_free"},
      {reinterpret_cast<void**>(&av_read_frame_func_), "av_read_frame"},
      {reinterpret_cast<void**>(&avcodec_send_packet_func_),
       "avcodec_send_packet"},
      {reinterpret_cast<void**>(&avcodec_receive_frame_func_),
       "avcodec_receive_frame"},
      {reinterpret_cast<void**>(&av_packet_unref_func_), "av_packet_unref"},
      {reinterpret_cast<void**>(&av_seek_frame_func_), "av_seek_frame"},
      {reinterpret_cast<void**>(&avformat_network_init_func_),
       "avformat_network_init"},
      {reinterpret_cast<void**>(&av_log_set_callback_func_),
       "av_log_set_callback"},
      {reinterpret_cast<void**>(&av_log_set_level_func_), "av_log_set_level"}};

  if (!load_symbols(symbols)) {
    ANIMAX_LOGE("Failed to load all FFmpeg symbols");
    return false;
  }

  if (avformat_network_init_func_) {
    avformat_network_init_func_();
  }

  if (av_log_set_callback_func_ && av_log_set_level_func_) {
    av_log_set_level_func_(AV_LOG_ERROR);
    av_log_set_callback_func_(ffmpeg_log_callback_cstyle);
  }

  is_symbols_valid_ = true;
  ANIMAX_LOGI("All FFmpeg symbols loaded successfully");
  return is_symbols_valid_;
}

}  // namespace animax
}  // namespace lynx
