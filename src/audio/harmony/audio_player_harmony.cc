// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/harmony/audio_player_harmony.h"

#include <algorithm>
#include <cstring>

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

std::unique_ptr<AudioPlayer> AudioPlayer::MakeAudioPlayer(
    std::shared_ptr<AnimaXAbility> ability, std::shared_ptr<AudioAsset> asset) {
  if (!asset || !asset->IsValid()) {
    ANIMAX_LOGE(
        "Failed to create AudioPlayerHarmony: asset is null or not valid")
    return nullptr;
  }

  auto player = std::make_unique<AudioPlayerHarmony>();

  if (!player->Init(asset)) {
    ANIMAX_LOGE(
        "Failed to create AudioPlayerHarmony: initialization failed in Init")
    return nullptr;
  }

  return player;
}

AudioPlayerHarmony::~AudioPlayerHarmony() {
  if (renderer_.Ptr()) {
    OH_AudioRenderer_Stop(renderer_.Ptr());
  }
}

bool AudioPlayerHarmony::Init(std::shared_ptr<AudioAsset> asset) {
  asset_harmony_ = std::static_pointer_cast<AudioAssetHarmony>(asset);
  if (!asset_harmony_) {
    ANIMAX_LOGE("Init failed: asset is not an instance of AudioAssetHarmony")
    return false;
  }

  const auto& info = asset_harmony_->GetAudioInfo();
  const auto& data = asset_harmony_->GetAudioData();

  if (data.empty()) {
    ANIMAX_LOGE("Init failed: audio PCM data is empty")
    return false;
  }

  OH_AudioStream_SampleFormat stream_sample_format = AUDIOSTREAM_SAMPLE_S16LE;
  switch (info.sample_format) {
    case SAMPLE_U8:
      stream_sample_format = AUDIOSTREAM_SAMPLE_U8;
      break;
    case SAMPLE_S16LE:
      stream_sample_format = AUDIOSTREAM_SAMPLE_S16LE;
      break;
    case SAMPLE_S24LE:
      stream_sample_format = AUDIOSTREAM_SAMPLE_S24LE;
      break;
    case SAMPLE_S32LE:
      stream_sample_format = AUDIOSTREAM_SAMPLE_S32LE;
      break;
    case SAMPLE_F32LE:
      stream_sample_format = AUDIOSTREAM_SAMPLE_F32LE;
      break;
    default:
      ANIMAX_LOGE("Init failed: Unexpected sample format passed from asset: "
                  << info.sample_format)
      return false;
  }

  bytes_per_frame_ = info.channel_count * info.byte_per_sample;

  OH_AudioStreamBuilder* builder = nullptr;
  if (OH_AudioStreamBuilder_Create(&builder, AUDIOSTREAM_TYPE_RENDERER) !=
      AUDIOSTREAM_SUCCESS) {
    ANIMAX_LOGE("Init failed: OH_AudioStreamBuilder_Create failed")
    return false;
  }

  ScopedOHObject<OH_AudioStreamBuilder, OH_AudioStreamBuilder_Destroy>
      safe_builder;
  safe_builder.reset(builder);

  OH_AudioStreamBuilder_SetSamplingRate(builder, info.sample_rate);
  OH_AudioStreamBuilder_SetChannelCount(builder, info.channel_count);
  OH_AudioStreamBuilder_SetSampleFormat(builder, stream_sample_format);
  OH_AudioStreamBuilder_SetEncodingType(builder, AUDIOSTREAM_ENCODING_TYPE_RAW);
  OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);

  OH_AudioRenderer_Callbacks callbacks = {
      &AudioPlayerHarmony::OnWriteData, &AudioPlayerHarmony::OnStreamEvent,
      &AudioPlayerHarmony::OnInterruptEvent, &AudioPlayerHarmony::OnError};
  OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, this);

  OH_AudioRenderer* renderer = nullptr;
  if (OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer) !=
      AUDIOSTREAM_SUCCESS) {
    ANIMAX_LOGE("Init failed: OH_AudioStreamBuilder_GenerateRenderer failed")
    return false;
  }

  renderer_.reset(renderer);

  ANIMAX_LOGI("AudioPlayerHarmony Init success. sampleRate:"
              << info.sample_rate << " channelCount:" << info.channel_count
              << " sampleFormat:" << info.sample_format)

  return true;
}

void AudioPlayerHarmony::Resume() {
  if (renderer_.Ptr()) {
    OH_AudioRenderer_Start(renderer_.Ptr());
  }
}

void AudioPlayerHarmony::Pause() {
  if (renderer_.Ptr()) {
    OH_AudioRenderer_Pause(renderer_.Ptr());
  }
}

void AudioPlayerHarmony::SeekToProgress(double progress) {
  if (!renderer_.Ptr() || !asset_harmony_) {
    return;
  }

  const auto& data = asset_harmony_->GetAudioData();
  int64_t data_size = static_cast<int64_t>(data.size());
  if (data_size == 0) {
    return;
  }

  OH_AudioRenderer_Flush(renderer_.Ptr());

  std::lock_guard<std::mutex> lock(playback_mutex_);
  start_position_bytes_ =
      AlignByteToFrames(static_cast<int64_t>(progress * data_size));
  read_offset_ = start_position_bytes_;
}

double AudioPlayerHarmony::GetDuration() {
  if (!asset_harmony_) {
    return 0;
  }
  return asset_harmony_->GetAudioInfo().duration / 1000.0;
}

double AudioPlayerHarmony::GetAudioTime() {
  if (!asset_harmony_ || !renderer_.Ptr()) {
    return 0.0;
  }

  const auto& data = asset_harmony_->GetAudioData();
  int64_t data_size = static_cast<int64_t>(data.size());
  if (data_size == 0) {
    return 0.0;
  }

  int64_t frame_position = 0;
  int64_t timestamp = 0;

  std::lock_guard<std::mutex> lock(playback_mutex_);

  OH_AudioStream_Result res = OH_AudioRenderer_GetTimestamp(
      renderer_.Ptr(), CLOCK_MONOTONIC, &frame_position, &timestamp);

  int64_t played_bytes = 0;
  if (res == AUDIOSTREAM_SUCCESS && frame_position > 0) {
    played_bytes = frame_position * bytes_per_frame_;
  }
  int64_t current_byte_pos = (start_position_bytes_ + played_bytes) % data_size;
  double ratio = static_cast<double>(current_byte_pos) / data.size();
  return ratio * GetDuration();
}

int64_t AudioPlayerHarmony::AlignByteToFrames(int64_t byte_position) const {
  if (bytes_per_frame_ <= 0) {
    return byte_position;
  }
  return (byte_position / bytes_per_frame_) * bytes_per_frame_;
}

int32_t AudioPlayerHarmony::OnWriteData(OH_AudioRenderer* renderer,
                                        void* userData, void* buffer,
                                        int32_t bufferLen) {
  auto* player = static_cast<AudioPlayerHarmony*>(userData);
  if (!player || !player->asset_harmony_ || !buffer || bufferLen <= 0) {
    return AUDIO_DATA_CALLBACK_RESULT_INVALID;
  }
  const auto& data = player->asset_harmony_->GetAudioData();
  int64_t data_size = static_cast<int64_t>(data.size());
  if (data_size == 0) {
    return AUDIO_DATA_CALLBACK_RESULT_INVALID;
  }
  std::lock_guard<std::mutex> lock(player->playback_mutex_);
  uint8_t* dest = static_cast<uint8_t*>(buffer);
  int64_t remaining = bufferLen;
  while (remaining > 0) {
    int64_t available_linear_size = data_size - player->read_offset_;
    int64_t to_copy = std::min(available_linear_size, remaining);
    std::memcpy(dest, data.data() + player->read_offset_, to_copy);
    dest += to_copy;
    remaining -= to_copy;
    player->read_offset_ = (player->read_offset_ + to_copy) % data_size;
  }
  return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

int32_t AudioPlayerHarmony::OnStreamEvent(OH_AudioRenderer* renderer,
                                          void* userData,
                                          OH_AudioStream_Event event) {
  return 0;
}

int32_t AudioPlayerHarmony::OnInterruptEvent(OH_AudioRenderer* renderer,
                                             void* userData,
                                             OH_AudioInterrupt_ForceType type,
                                             OH_AudioInterrupt_Hint hint) {
  return 0;
}

int32_t AudioPlayerHarmony::OnError(OH_AudioRenderer* renderer, void* userData,
                                    OH_AudioStream_Result error) {
  ANIMAX_LOGE("OHAudio Renderer Error: " << error)
  return 0;
}

}  // namespace animax
}  // namespace lynx
