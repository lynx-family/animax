// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_HARMONY_AUDIO_ASSET_HARMONY_H_
#define ANIMAX_SRC_AUDIO_HARMONY_AUDIO_ASSET_HARMONY_H_

#include <multimedia/player_framework/native_avcodec_audiocodec.h>
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avsource.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "src/base/util/harmony/scoped_object_harmony.h"
#include "src/resource/asset/audio_asset.h"

namespace lynx {
namespace animax {

struct AudioInfo {
  int64_t duration = 0;
  int channel_count = 0;
  int sample_rate = 0;
  int track_index = -1;
  int sample_format = SAMPLE_S16LE;
  int byte_per_sample = 2;
};

struct BufferInfo {
  uint32_t index;
  OH_AVBuffer* buffer;
};

struct CodecUserData {
  std::mutex in_mutex;
  std::condition_variable in_cond;
  std::queue<BufferInfo> input_queue;

  std::mutex out_mutex;
  std::condition_variable out_cond;
  std::queue<BufferInfo> output_queue;

  std::atomic<bool> is_eos{false};
  std::atomic<bool> has_error{false};
  std::weak_ptr<AudioInfo> info;
};

class AudioAssetHarmony : public AudioAsset {
 public:
  ~AudioAssetHarmony() override = default;
  explicit AudioAssetHarmony(AudioAssetModel model);
  void LoadLocal(const std::string& file_path) override;
  const AudioInfo& GetAudioInfo() const;
  const std::vector<uint8_t>& GetAudioData() const;

 private:
  // Find audio track by av format
  void FindAudioTrack();
  // Read audio basic parameters
  bool InitAudioInfo();
  // Read the audio raw buffer by demuxer and decode it by codec, saving into
  // buffer_data.
  bool PrepareDecodedData(int64_t file_size);
  bool InitCodec();

  static void OnCodecError(OH_AVCodec* codec, int32_t errorCode,
                           void* userData);
  static void OnCodecFormatChanged(OH_AVCodec* codec, OH_AVFormat* format,
                                   void* userData);
  static void OnNeedInputBuffer(OH_AVCodec* codec, uint32_t index,
                                OH_AVBuffer* buffer, void* userData);
  static void OnNewOutputBuffer(OH_AVCodec* codec, uint32_t index,
                                OH_AVBuffer* buffer, void* userData);

  ScopedOHObject<OH_AVSource, OH_AVSource_Destroy> av_source_;
  ScopedOHObject<OH_AVDemuxer, OH_AVDemuxer_Destroy> av_demuxer_;
  ScopedOHObject<OH_AVCodec, OH_AudioCodec_Destroy> av_codec_;
  ScopedOHObject<OH_AVFormat, OH_AVFormat_Destroy> audio_format_;

  std::shared_ptr<AudioInfo> info_ = std::make_shared<AudioInfo>();
  std::vector<uint8_t> audio_data_;
  CodecUserData codec_ctx_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_HARMONY_AUDIO_ASSET_HARMONY_H_
