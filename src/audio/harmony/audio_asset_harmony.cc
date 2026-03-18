// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/harmony/audio_asset_harmony.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "src/audio/audio_player.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

constexpr static int kMaxInvalidLoopCount = 100;
constexpr static std::chrono::milliseconds kPollTimeout{2};

std::shared_ptr<AudioAsset> AudioAsset::Make(AudioAssetModel model) {
  return std::make_shared<AudioAssetHarmony>(model);
}

AudioAssetHarmony::AudioAssetHarmony(AudioAssetModel model)
    : AudioAsset(std::move(model)) {}

void AudioAssetHarmony::LoadLocal(const std::string& file_path) {
  int fd = open(file_path.c_str(), O_RDONLY);
  if (fd == -1) {
    ANIMAX_LOGE("Failed to open file: " << file_path)
    return;
  }

  struct stat file_stat;
  if (fstat(fd, &file_stat) == -1) {
    ANIMAX_LOGE("Failed to get file size: " << file_path)
    close(fd);
    return;
  }
  int64_t file_size = file_stat.st_size;

  auto source = OH_AVSource_CreateWithFD(fd, 0, file_size);
  if (!source) {
    ANIMAX_LOGE("OH_AVSource_CreateWithFD fail")
    close(fd);
    return;
  }
  av_source_.reset(source);

  auto demuxer = OH_AVDemuxer_CreateWithSource(source);
  if (!demuxer) {
    ANIMAX_LOGE("OH_AVDemuxer_CreateWithSource fail")
    close(fd);
    return;
  }
  av_demuxer_.reset(demuxer);

  FindAudioTrack();
  if (!InitAudioInfo()) {
    close(fd);
    return;
  }

  if (!PrepareDecodedData(file_size)) {
    close(fd);
    return;
  }

  if (!audio_data_.empty()) {
    is_valid_ = true;
  }
  close(fd);
}

const AudioInfo& AudioAssetHarmony::GetAudioInfo() const { return *info_; }

const std::vector<uint8_t>& AudioAssetHarmony::GetAudioData() const {
  return audio_data_;
}

void AudioAssetHarmony::FindAudioTrack() {
  if (!av_source_.Ptr() || !av_demuxer_.Ptr()) {
    ANIMAX_LOGE("FindAudioTrack failed: av_source or av_demuxer is null")
    return;
  }

  OH_AVFormat* source_format = OH_AVSource_GetSourceFormat(av_source_.Ptr());
  if (!source_format) {
    ANIMAX_LOGE("FindAudioTrack failed: could not get source format")
    return;
  }

  int32_t track_count = 0;
  OH_AVFormat_GetIntValue(source_format, OH_MD_KEY_TRACK_COUNT, &track_count);
  OH_AVFormat_Destroy(source_format);

  info_->track_index = -1;

  for (int32_t i = 0; i < track_count; ++i) {
    OH_AVFormat* track_format = OH_AVSource_GetTrackFormat(av_source_.Ptr(), i);
    if (!track_format) {
      continue;
    }

    int32_t track_type = 0;
    OH_AVFormat_GetIntValue(track_format, OH_MD_KEY_TRACK_TYPE, &track_type);

    if (track_type == MEDIA_TYPE_AUD) {
      info_->track_index = i;
      audio_format_.reset(track_format);
      OH_AVDemuxer_SelectTrackByID(av_demuxer_.Ptr(), info_->track_index);
      break;
    }

    OH_AVFormat_Destroy(track_format);
  }

  if (info_->track_index == -1) {
    ANIMAX_LOGE("FindAudioTrack failed: no audio track found")
  }
}

bool AudioAssetHarmony::InitAudioInfo() {
  if (info_->track_index == -1 || !audio_format_.Ptr()) {
    ANIMAX_LOGE("InitAudioInfo failed: audio track not found or format is null")
    return false;
  }
  OH_AVFormat* format = audio_format_.Ptr();

  int32_t sample_rate = 0;
  if (OH_AVFormat_GetIntValue(format, OH_MD_KEY_AUD_SAMPLE_RATE,
                              &sample_rate)) {
    info_->sample_rate = sample_rate;
  } else {
    ANIMAX_LOGE("InitAudioInfo failed: could not get sample rate")
    return false;
  }

  int32_t channel_count = 0;
  if (OH_AVFormat_GetIntValue(format, OH_MD_KEY_AUD_CHANNEL_COUNT,
                              &channel_count)) {
    info_->channel_count = channel_count;
  } else {
    ANIMAX_LOGE("InitAudioInfo failed: could not get channel count")
    return false;
  }

  int64_t duration_us = 0;
  if (OH_AVFormat_GetLongValue(format, OH_MD_KEY_DURATION, &duration_us)) {
    info_->duration = duration_us;
  } else {
    ANIMAX_LOGW("InitAudioInfo: could not get duration from track, set to 0")
  }

  ANIMAX_LOGI("InitAudioInfo Success. sampleRate:"
              << sample_rate << " channelCount:" << channel_count
              << " duration:" << duration_us)

  return true;
}

bool AudioAssetHarmony::PrepareDecodedData(int64_t file_size) {
  if (!InitCodec()) {
    return false;
  }
  if (OH_AudioCodec_Start(av_codec_.Ptr()) != AV_ERR_OK) {
    ANIMAX_LOGE("PrepareDecodedData failed: OH_AudioCodec_Start failed");
    return false;
  }

  bool is_input_eos = false;
  int invalid_loop_count = 0;

  if (info_->duration > 0) {
    size_t estimated_size = (info_->duration / 1000000.0) * info_->sample_rate *
                            info_->channel_count * info_->byte_per_sample;
    audio_data_.reserve(estimated_size);
  }

  while (!codec_ctx_.is_eos && !codec_ctx_.has_error) {
    bool has_action = false;
    if (!is_input_eos) {
      uint32_t in_index = -1;
      OH_AVBuffer* in_buffer = nullptr;
      bool has_input = false;

      {
        std::unique_lock<std::mutex> lock(codec_ctx_.in_mutex);
        if (codec_ctx_.input_queue.empty()) {
          codec_ctx_.in_cond.wait_for(lock, kPollTimeout, [this]() {
            return !codec_ctx_.input_queue.empty() || codec_ctx_.has_error;
          });
        }

        if (!codec_ctx_.input_queue.empty()) {
          auto& in_info = codec_ctx_.input_queue.front();
          in_index = in_info.index;
          in_buffer = in_info.buffer;
          codec_ctx_.input_queue.pop();
          has_input = true;
        }
      }

      if (has_input) {
        has_action = true;

        if (in_buffer) {
          if (OH_AVDemuxer_ReadSampleBuffer(av_demuxer_.Ptr(),
                                            info_->track_index,
                                            in_buffer) == AV_ERR_OK) {
            OH_AVCodecBufferAttr attr;
            OH_AVBuffer_GetBufferAttr(in_buffer, &attr);
            if ((attr.flags & AVCODEC_BUFFER_FLAGS_EOS) || attr.size == 0) {
              is_input_eos = true;
              attr.flags = AVCODEC_BUFFER_FLAGS_EOS;
              attr.size = 0;
              OH_AVBuffer_SetBufferAttr(in_buffer, &attr);
            }
          } else {
            is_input_eos = true;
            OH_AVCodecBufferAttr attr = {0, 0, 0, AVCODEC_BUFFER_FLAGS_EOS};
            OH_AVBuffer_SetBufferAttr(in_buffer, &attr);
          }
          if (OH_AudioCodec_PushInputBuffer(av_codec_.Ptr(), in_index) !=
              AV_ERR_OK) {
            ANIMAX_LOGE("PushInputBuffer failed")
            codec_ctx_.has_error = true;
          }
        }
      }
    }

    if (codec_ctx_.has_error) {
      break;
    }

    uint32_t out_index = -1;
    OH_AVBuffer* out_buffer = nullptr;
    bool has_output = false;

    {
      std::unique_lock<std::mutex> lock(codec_ctx_.out_mutex);
      if (codec_ctx_.output_queue.empty()) {
        codec_ctx_.out_cond.wait_for(lock, kPollTimeout, [this]() {
          return !codec_ctx_.output_queue.empty() || codec_ctx_.has_error;
        });
      }

      if (!codec_ctx_.output_queue.empty()) {
        auto& out_info = codec_ctx_.output_queue.front();
        out_index = out_info.index;
        out_buffer = out_info.buffer;
        codec_ctx_.output_queue.pop();
        has_output = true;
      }
    }

    if (has_output) {
      has_action = true;

      if (out_buffer) {
        OH_AVCodecBufferAttr attr;
        OH_AVBuffer_GetBufferAttr(out_buffer, &attr);

        if (attr.size > 0) {
          uint8_t* pcm_data = OH_AVBuffer_GetAddr(out_buffer);
          if (pcm_data) {
            audio_data_.insert(audio_data_.end(), pcm_data,
                               pcm_data + attr.size);
          }
        }

        if (attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
          ANIMAX_LOGI("Decoder received EOS from Output.")
          codec_ctx_.is_eos = true;
        }

        OH_AudioCodec_FreeOutputBuffer(av_codec_.Ptr(), out_index);
      }
    }

    if (!has_action) {
      invalid_loop_count++;
      if (invalid_loop_count > kMaxInvalidLoopCount) {
        ANIMAX_LOGE(
            "Failed to decode audio asset: the decoder is in infinite loop.")
        break;
      }
    } else {
      invalid_loop_count = 0;
    }
  }
  OH_AudioCodec_Stop(av_codec_.Ptr());

  if (!codec_ctx_.is_eos && !codec_ctx_.has_error) {
    ANIMAX_LOGE("Failed to decode audio asset: exited loop without EOS.")
  }

  if (info_->duration <= 0 && !audio_data_.empty()) {
    double total_seconds =
        static_cast<double>(audio_data_.size()) /
        (info_->sample_rate * info_->channel_count * info_->byte_per_sample);
    info_->duration = static_cast<int64_t>(total_seconds * 1000000.0);
    ANIMAX_LOGI("Calculated duration from PCM data: " << info_->duration)
  }

  ANIMAX_LOGI("Decode finished. Total PCM size: " << audio_data_.size())
  return !codec_ctx_.has_error && codec_ctx_.is_eos;
}

bool AudioAssetHarmony::InitCodec() {
  if (!audio_format_.Ptr()) return false;

  codec_ctx_.info = info_;

  const char* mime_type = nullptr;
  if (!OH_AVFormat_GetStringValue(audio_format_.Ptr(), OH_MD_KEY_CODEC_MIME,
                                  &mime_type)) {
    ANIMAX_LOGE("InitCodec failed: could not get codec mime type");
    return false;
  }

  int32_t max_input_size = 0;
  if (!OH_AVFormat_GetIntValue(audio_format_.Ptr(), OH_MD_KEY_MAX_INPUT_SIZE,
                               &max_input_size)) {
    OH_AVFormat_SetIntValue(audio_format_.Ptr(), OH_MD_KEY_MAX_INPUT_SIZE,
                            32 * 1024);
    ANIMAX_LOGI("Patched OH_MD_KEY_MAX_INPUT_SIZE to 32KB");
  }

  auto codec = OH_AudioCodec_CreateByMime(mime_type, false);
  if (!codec) {
    ANIMAX_LOGE("InitCodec failed: OH_AudioCodec_CreateByMime failed");
    return false;
  }
  av_codec_.reset(codec);

  OH_AVCodecCallback cb = {&AudioAssetHarmony::OnCodecError,
                           &AudioAssetHarmony::OnCodecFormatChanged,
                           &AudioAssetHarmony::OnNeedInputBuffer,
                           &AudioAssetHarmony::OnNewOutputBuffer};

  if (OH_AudioCodec_RegisterCallback(av_codec_.Ptr(), cb, &codec_ctx_) !=
      AV_ERR_OK) {
    ANIMAX_LOGE("InitCodec failed: OH_AudioCodec_RegisterCallback failed");
    return false;
  }

  if (OH_AudioCodec_Configure(av_codec_.Ptr(), audio_format_.Ptr()) !=
      AV_ERR_OK) {
    ANIMAX_LOGE("InitCodec failed: OH_AudioCodec_Configure failed");
    return false;
  }

  return true;
}

void AudioAssetHarmony::OnCodecError(OH_AVCodec* codec, int32_t errorCode,
                                     void* userData) {
  ANIMAX_LOGE("AudioCodec Error: " << errorCode);
  auto* ctx = static_cast<CodecUserData*>(userData);
  ctx->has_error = true;
  ctx->in_cond.notify_all();
  ctx->out_cond.notify_all();
}

void AudioAssetHarmony::OnCodecFormatChanged(OH_AVCodec* codec,
                                             OH_AVFormat* format,
                                             void* userData) {
  ANIMAX_LOGI("AudioCodec Format Changed");
  auto* ctx = static_cast<CodecUserData*>(userData);
  std::lock_guard<std::mutex> lock(ctx->out_mutex);
  auto info_ptr = ctx->info.lock();
  if (format && info_ptr) {
    int32_t sample_format = 0;
    if (OH_AVFormat_GetIntValue(format, OH_MD_KEY_AUDIO_SAMPLE_FORMAT,
                                &sample_format)) {
      switch (sample_format) {
        case SAMPLE_U8:
          info_ptr->byte_per_sample = 1;
          break;
        case SAMPLE_S16LE:
          info_ptr->byte_per_sample = 2;
          break;
        case SAMPLE_S24LE:
          info_ptr->byte_per_sample = 3;
          break;
        case SAMPLE_S32LE:
        case SAMPLE_F32LE:
          info_ptr->byte_per_sample = 4;
          break;
        default:
          ANIMAX_LOGE(
              "Invalid sample format or planar detected: " << sample_format);
          ctx->has_error = true;
          return;
      }
      info_ptr->sample_format = sample_format;
      ANIMAX_LOGI("Update sample format to: " << sample_format);
    }
  }
}

void AudioAssetHarmony::OnNeedInputBuffer(OH_AVCodec* codec, uint32_t index,
                                          OH_AVBuffer* buffer, void* userData) {
  auto* ctx = static_cast<CodecUserData*>(userData);
  std::lock_guard<std::mutex> lock(ctx->in_mutex);
  ctx->input_queue.push({index, buffer});
  ctx->in_cond.notify_all();
}

void AudioAssetHarmony::OnNewOutputBuffer(OH_AVCodec* codec, uint32_t index,
                                          OH_AVBuffer* buffer, void* userData) {
  auto* ctx = static_cast<CodecUserData*>(userData);
  std::lock_guard<std::mutex> lock(ctx->out_mutex);
  ctx->output_queue.push({index, buffer});
  ctx->out_cond.notify_all();
}

}  // namespace animax
}  // namespace lynx
