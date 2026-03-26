// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/ios/audio_asset_ios.h"

#include <utility>

#include <CoreFoundation/CoreFoundation.h>

#include "src/audio/audio_player.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

std::shared_ptr<AudioAsset> AudioAsset::Make(AudioAssetModel model) {
  return std::make_shared<AudioAssetIOS>(std::move(model));
}

AudioAssetIOS::AudioAssetIOS(AudioAssetModel model) : AudioAsset(std::move(model)) {}

const AudioInfo& AudioAssetIOS::GetAudioInfo() const { return info_; }

const std::vector<uint8_t>& AudioAssetIOS::GetAudioData() const { return audio_data_; }

void AudioAssetIOS::LoadLocal(const std::string& file_path) {
  CFStringRef cf_path =
      CFStringCreateWithCString(kCFAllocatorDefault, file_path.c_str(), kCFStringEncodingUTF8);
  if (!cf_path) {
    ANIMAX_LOGE("Failed to create CFString from file path: " << file_path)
    return;
  }

  CFURLRef file_url =
      CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cf_path, kCFURLPOSIXPathStyle, false);
  CFRelease(cf_path);

  if (!file_url) {
    ANIMAX_LOGE("Failed to create CFURL from file path: " << file_path)
    return;
  }

  ExtAudioFileRef ext_audio_file = nullptr;
  OSStatus status = ExtAudioFileOpenURL(file_url, &ext_audio_file);
  CFRelease(file_url);

  if (status != noErr || !ext_audio_file) {
    ANIMAX_LOGE("ExtAudioFileOpenURL failed with status: " << status << " path: " << file_path)
    return;
  }

  if (!InitAudioFormatAndInfo(ext_audio_file)) {
    ExtAudioFileDispose(ext_audio_file);
    return;
  }

  if (!ReadAllFrames(ext_audio_file)) {
    ExtAudioFileDispose(ext_audio_file);
    return;
  }

  ExtAudioFileDispose(ext_audio_file);

  if (!audio_data_.empty()) {
    is_valid_ = true;
    ANIMAX_LOGI("Successfully loaded and decoded audio asset. Total bytes: " << audio_data_.size())
  }
}

bool AudioAssetIOS::InitAudioFormatAndInfo(ExtAudioFileRef ext_audio_file) {
  // --- STEP 1: Read Original Format ---
  AudioStreamBasicDescription file_format = {};
  UInt32 size = sizeof(file_format);
  OSStatus status = ExtAudioFileGetProperty(ext_audio_file, kExtAudioFileProperty_FileDataFormat,
                                            &size, &file_format);

  if (status != noErr) {
    ANIMAX_LOGE("Failed to get original file format. Status: " << status)
    return false;
  }

  // --- STEP 2: Customize Client Format ---
  AudioStreamBasicDescription client_format = {};
  client_format.mFormatID = kAudioFormatLinearPCM;
  client_format.mSampleRate = file_format.mSampleRate;
  client_format.mChannelsPerFrame = file_format.mChannelsPerFrame;

  if (file_format.mSampleRate <= 0.0 || file_format.mChannelsPerFrame <= 0) {
    ANIMAX_LOGE("Invalid audio file format. SampleRate: "
                << file_format.mSampleRate << ", Channels: " << file_format.mChannelsPerFrame)
    return false;
  }

  if (file_format.mFormatID == kAudioFormatLinearPCM) {
    // Original is PCM: inherit its bit depth, but strictly remove the NonInterleaved (Planar) flag.
    client_format.mBitsPerChannel = file_format.mBitsPerChannel;
    client_format.mFormatFlags = file_format.mFormatFlags & ~kAudioFormatFlagIsNonInterleaved;
  } else {
    // Original is compressed (e.g., MP3/AAC): no native bit depth exists.
    // Default to a safe standard: 16-bit Signed Integer, Packed, Interleaved.
    client_format.mBitsPerChannel = 16;
    client_format.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
  }

  client_format.mBytesPerFrame =
      (client_format.mBitsPerChannel / 8) * client_format.mChannelsPerFrame;
  client_format.mFramesPerPacket = 1;
  client_format.mBytesPerPacket = client_format.mBytesPerFrame;

  // Tell the ExtAudioFile API to output data in our newly customized format.
  status = ExtAudioFileSetProperty(ext_audio_file, kExtAudioFileProperty_ClientDataFormat,
                                   sizeof(client_format), &client_format);

  if (status != noErr) {
    ANIMAX_LOGE("Failed to set ClientDataFormat. Status: " << status)
    return false;
  }

  // --- STEP 3: Store Final Format and Extract Length ---
  info_.client_format = client_format;

  SInt64 total_frames = 0;
  size = sizeof(total_frames);
  status = ExtAudioFileGetProperty(ext_audio_file, kExtAudioFileProperty_FileLengthFrames, &size,
                                   &total_frames);

  if (status == noErr) {
    info_.duration = static_cast<int64_t>(
        (static_cast<double>(total_frames) / client_format.mSampleRate) * 1000000.0);
    audio_data_.reserve(total_frames * info_.client_format.mBytesPerFrame);
  } else {
    ANIMAX_LOGI("Could not get file length frames natively. Duration will be calculated later.")
  }

  ANIMAX_LOGI("InitAudioFormatAndInfo success. SampleRate: "
              << info_.client_format.mSampleRate
              << ", Channels: " << info_.client_format.mChannelsPerFrame << ", BitsPerChannel: "
              << info_.client_format.mBitsPerChannel << ", Duration(us): " << info_.duration)

  return true;
}

bool AudioAssetIOS::ReadAllFrames(ExtAudioFileRef ext_audio_file) {
  // Read in chunks of 4096 frames.
  const UInt32 kMaxFramesPerRead = 4096;
  UInt32 bytes_per_read = kMaxFramesPerRead * info_.client_format.mBytesPerFrame;
  std::vector<uint8_t> read_buffer(bytes_per_read);

  while (true) {
    // Prepare the Core Audio buffer list.
    // mNumberBuffers = 1 because we explicitly stripped the Planar flag.
    AudioBufferList buffer_list;
    buffer_list.mNumberBuffers = 1;
    buffer_list.mBuffers[0].mNumberChannels = info_.client_format.mChannelsPerFrame;
    buffer_list.mBuffers[0].mDataByteSize = bytes_per_read;
    buffer_list.mBuffers[0].mData = read_buffer.data();

    // frames_to_read operates as both input and output.
    // Input: we want to read up to kMaxFramesPerRead.
    // Output: ExtAudioFileRead modifies it to tell us how many frames were actually read.
    UInt32 frames_to_read = kMaxFramesPerRead;

    OSStatus status = ExtAudioFileRead(ext_audio_file, &frames_to_read, &buffer_list);

    if (status != noErr) {
      ANIMAX_LOGE("ExtAudioFileRead failed with status: " << status)
      return false;
    }

    if (frames_to_read == 0) {
      break;
    }

    size_t actual_bytes = frames_to_read * info_.client_format.mBytesPerFrame;
    audio_data_.insert(audio_data_.end(), read_buffer.data(), read_buffer.data() + actual_bytes);
  }

  int32_t byte_per_sample = info_.client_format.mBitsPerChannel / 8;
  double total_seconds =
      static_cast<double>(audio_data_.size()) /
      (info_.client_format.mSampleRate * info_.client_format.mChannelsPerFrame * byte_per_sample);
  info_.duration = static_cast<int64_t>(total_seconds * 1000000.0);
  ANIMAX_LOGI("Calculated duration from PCM data: " << info_.duration)
  ANIMAX_LOGI("Decode finished. Total PCM size: " << audio_data_.size() << " bytes.")
  return true;
}

}  // namespace animax
}  // namespace lynx
