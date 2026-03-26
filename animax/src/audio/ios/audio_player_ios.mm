// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/ios/audio_player_ios.h"

#include <algorithm>
#include <cstring>

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

std::unique_ptr<AudioPlayer> AudioPlayer::MakeAudioPlayer(std::shared_ptr<AnimaXAbility> ability,
                                                          std::shared_ptr<AudioAsset> asset) {
  if (!asset || !asset->IsValid()) {
    ANIMAX_LOGE("Failed to create AudioPlayerIOS: asset is null or not valid")
    return nullptr;
  }

  auto player = std::make_unique<AudioPlayerIOS>();

  if (!player->Init(asset)) {
    ANIMAX_LOGE("Failed to create AudioPlayerIOS: initialization failed in Init")
    return nullptr;
  }

  return player;
}

AudioPlayerIOS::~AudioPlayerIOS() {
  if (audio_unit_) {
    // TODO(lixianruo.cyrus): use SystemInvoke Service instead.
    AudioOutputUnitStop(audio_unit_);
    AudioUnitUninitialize(audio_unit_);
    AudioComponentInstanceDispose(audio_unit_);
    audio_unit_ = nullptr;
  }
}

bool AudioPlayerIOS::Init(std::shared_ptr<AudioAsset> asset) {
  asset_ios_ = std::static_pointer_cast<AudioAssetIOS>(asset);
  DCHECK(asset_ios_);
  const AudioInfo& info = asset_ios_->GetAudioInfo();
  const std::vector<uint8_t>& data = asset_ios_->GetAudioData();

  if (data.empty()) {
    ANIMAX_LOGE("Init failed: audio PCM data is empty")
    return false;
  }

  // Pre-calculate loop invariants to keep the render thread clean and fast.
  AudioStreamBasicDescription client_format = info.client_format;
  bytes_per_frame_ = client_format.mBytesPerFrame;
  sample_rate_ = client_format.mSampleRate;

  if (bytes_per_frame_ > 0) {
    total_frames_ = static_cast<int64_t>(data.size()) / bytes_per_frame_;
  }
  duration_ms_ = info.duration / 1000.0;

  // 1. Find the RemoteIO Audio Component
  AudioComponentDescription desc = {};
  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_RemoteIO;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;

  AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
  if (!comp) {
    ANIMAX_LOGE("Init failed: could not find RemoteIO AudioComponent")
    return false;
  }

  // 2. Create the AudioUnit instance
  OSStatus status = AudioComponentInstanceNew(comp, &audio_unit_);
  if (status != noErr || !audio_unit_) {
    ANIMAX_LOGE("Init failed: AudioComponentInstanceNew error: " << status)
    return false;
  }

  // 3. Set the Stream Format (Our exact ASBD from the Asset)
  // We apply this to the Input scope of the Output bus (Bus 0).
  const AudioUnitElement kOutputBus = 0;
  status = AudioUnitSetProperty(audio_unit_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
                                kOutputBus, &client_format, sizeof(client_format));
  if (status != noErr) {
    ANIMAX_LOGE("Init failed: AudioUnitSetProperty StreamFormat error: " << status)
    return false;
  }

  // 4. Set the Render Callback
  AURenderCallbackStruct callback_struct;
  callback_struct.inputProc = RenderCallback;
  callback_struct.inputProcRefCon = this;
  status = AudioUnitSetProperty(audio_unit_, kAudioUnitProperty_SetRenderCallback,
                                kAudioUnitScope_Global, kOutputBus, &callback_struct,
                                sizeof(callback_struct));
  if (status != noErr) {
    ANIMAX_LOGE("Init failed: AudioUnitSetProperty SetRenderCallback error: " << status)
    return false;
  }

  // 5. Initialize the AudioUnit
  status = AudioUnitInitialize(audio_unit_);
  if (status != noErr) {
    ANIMAX_LOGE("Init failed: AudioUnitInitialize error: " << status)
    return false;
  }

  ANIMAX_LOGI("AudioPlayerIOS Init success. SampleRate: " << sample_rate_
                                                          << ", TotalFrames: " << total_frames_
                                                          << ", Duration(ms): " << duration_ms_)
  return true;
}

void AudioPlayerIOS::Resume() {
  if (audio_unit_) {
    // TODO(lixianruo.cyrus): use SystemInvoke Service instead.
    AudioOutputUnitStart(audio_unit_);
  }
}

void AudioPlayerIOS::Pause() {
  if (audio_unit_) {
    // TODO(lixianruo.cyrus): use SystemInvoke Service instead.
    AudioOutputUnitStop(audio_unit_);
  }
}

void AudioPlayerIOS::SeekToProgress(double progress) {
  if (!audio_unit_ || total_frames_ <= 0) {
    return;
  }

  int64_t target_frame = static_cast<int64_t>(progress * total_frames_);
  target_frame = std::max<int64_t>(0, std::min<int64_t>(target_frame, total_frames_ - 1));

  AudioUnitReset(audio_unit_, kAudioUnitScope_Global, 0);

  std::lock_guard<std::mutex> lock(state_mutex_);
  current_read_frame_ = target_frame;

  // Reset anchors. This achieves two things naturally:
  // 1. GetAudioTime() will immediately return target_frame (since hw diff is 0).
  // 2. Next RenderCallback will see current_hardware_sample_time_ == 0 and establish a new anchor.
  anchor_read_frame_ = target_frame;
  anchor_hardware_sample_time_ = 0.0;
  current_hardware_sample_time_ = 0.0;
}

double AudioPlayerIOS::GetDuration() { return duration_ms_; }

double AudioPlayerIOS::GetAudioTime() {
  std::lock_guard<std::mutex> lock(state_mutex_);

  if (sample_rate_ <= 0.0) {
    return 0.0;
  }

  Float64 hw_delta_frames =
      std::max(0.0, current_hardware_sample_time_ - anchor_hardware_sample_time_);

  int64_t current_played_frames = anchor_read_frame_ + static_cast<int64_t>(hw_delta_frames);
  if (total_frames_ > 0) {
    current_played_frames %= total_frames_;
  }

  return (static_cast<double>(current_played_frames) / sample_rate_) * 1000.0;
}

OSStatus AudioPlayerIOS::RenderCallback(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags,
                                        const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                                        UInt32 inNumberFrames, AudioBufferList* ioData) {
  auto* player = static_cast<AudioPlayerIOS*>(inRefCon);
  if (!player || !player->asset_ios_) {
    if (ioData) {
      for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
        if (ioData->mBuffers[i].mData) {
          std::memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
        }
      }
    }
    if (ioActionFlags) {
      *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
    }
    return noErr;
  }

  const std::vector<uint8_t>& data = player->asset_ios_->GetAudioData();
  if (data.empty() || player->total_frames_ <= 0) {
    if (ioData) {
      for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
        if (ioData->mBuffers[i].mData) {
          std::memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
        }
      }
    }
    if (ioActionFlags) {
      *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
    }
    return noErr;
  }

  std::lock_guard<std::mutex> lock(player->state_mutex_);

  Float64 current_hw_frames = 0.0;
  if (inTimeStamp->mFlags & kAudioTimeStampSampleTimeValid) {
    current_hw_frames = inTimeStamp->mSampleTime;
  } else {
    current_hw_frames = player->current_hardware_sample_time_ + inNumberFrames;
  }

  // Handle hardware time jump/reset (e.g., initial start, OS interruption, or after Seek)
  if (player->current_hardware_sample_time_ == 0.0 ||
      current_hw_frames < player->anchor_hardware_sample_time_) {
    player->anchor_read_frame_ = player->current_read_frame_;
    player->anchor_hardware_sample_time_ = current_hw_frames;
  }
  player->current_hardware_sample_time_ = current_hw_frames;

  uint32_t frames_to_fill = inNumberFrames;
  uint8_t* dest_buffer = static_cast<uint8_t*>(ioData->mBuffers[0].mData);

  // Loop filling logic for infinite playback
  while (frames_to_fill > 0) {
    int64_t available_linear_frames = player->total_frames_ - player->current_read_frame_;
    uint32_t frames_to_copy =
        std::min<uint32_t>(frames_to_fill, static_cast<uint32_t>(available_linear_frames));

    size_t bytes_to_copy = frames_to_copy * player->bytes_per_frame_;
    size_t read_offset_bytes = player->current_read_frame_ * player->bytes_per_frame_;

    std::memcpy(dest_buffer, data.data() + read_offset_bytes, bytes_to_copy);

    dest_buffer += bytes_to_copy;
    frames_to_fill -= frames_to_copy;

    player->current_read_frame_ =
        (player->current_read_frame_ + frames_to_copy) % player->total_frames_;
  }

  return noErr;
}

}  // namespace animax
}  // namespace lynx
