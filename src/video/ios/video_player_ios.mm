// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/ios/video_player_ios.h"

#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/player/animax_event.h"
#include "src/player/ios/animax_ability_ios.h"
#include "src/render/texture_info_mtl.h"
#include "src/video/ios/pending_frame_set.h"
#include "src/video/ios/video_asset_ios.h"
#include "src/video/ios/video_frame.h"
#include "src/video/ios/video_frame_cache.h"

static void AnimaXVideoDecompression(void *decompressionOutputRefCon, void *sourceFrameRefCon,
                                     OSStatus status, VTDecodeInfoFlags infoFlags,
                                     CVImageBufferRef imageBuffer, CMTime presentationTimeStamp,
                                     CMTime presentationDuration);

namespace lynx {
namespace animax {

class DecompressionInfo {
 public:
  std::weak_ptr<PendingFrameSet> weak_pending_frame_set;
  int32_t presentation_index = -1;
};

class VideoPlayerErrorReporter {
 public:
  enum class Error : int32_t {
    kCreateSessionError = 1,
    kCreateSampleBufferError = 2,
    kCreateBlockBufferError = 3,
    kDecodeFrameError = 4,
  };
  VideoPlayerErrorReporter(VideoPlayerIOS *player) : player_(player) {}

  void HasDrewOnce();
  void ReportErrorOnce(const std::string &err_msg, const Error err_code);
  void ReportDecodeFrameError();
  void ReportHasDrawnOnceAfterError();

 private:
  void Log(const std::string &err_msg) const;

  std::unordered_set<Error> error_reported_;
  int32_t error_count_ = 0;
  bool decode_frame_error_reported = false;
  bool has_drawn_once_after_error_reported = false;
  VideoPlayerIOS *player_ = nullptr;
};

void VideoPlayerErrorReporter::ReportDecodeFrameError() {
  if (!decode_frame_error_reported && error_reported_.count(Error::kDecodeFrameError)) {
    decode_frame_error_reported = true;
    const auto err_msg = std::string("DecodeFrame error has occurred");
    if (player_->listener_) {
      player_->listener_->OnVideoPlayerError(EventError::kVideoPlayerErrorHasOccurred, err_msg);
    }
  }
}

void VideoPlayerErrorReporter::ReportHasDrawnOnceAfterError() {
  if (!has_drawn_once_after_error_reported && !error_reported_.empty()) {
    has_drawn_once_after_error_reported = true;
    const auto err_msg =
        std::string("Error has occurred before HasDrewOnce called, has decode frame error: ") +
        std::to_string(error_reported_.count(Error::kDecodeFrameError));
    if (player_->listener_) {
      player_->listener_->OnVideoPlayerError(err_msg);
    }
  }
}

void VideoPlayerErrorReporter::HasDrewOnce() { ReportHasDrawnOnceAfterError(); }

void VideoPlayerErrorReporter::ReportErrorOnce(const std::string &err_msg, const Error err_code) {
  ++error_count_;
  if (error_reported_.count(err_code)) {
    Log(err_msg);
    return;
  }
  error_reported_.insert(err_code);
  Log(err_msg);
  if (player_->listener_) {
    player_->listener_->OnVideoPlayerError(err_msg);
  }
  ReportDecodeFrameError();
}

void VideoPlayerErrorReporter::Log(const std::string &err_msg) const {
  ANIMAX_LOGE("Error(" << static_cast<int32_t>(error_count_) << "): " << err_msg);
}

VideoPlayerIOS::VideoPlayerIOS(bool enable_opt_vtb_decoder_handler)
    : error_reporter_(
          std::unique_ptr<VideoPlayerErrorReporter>(new VideoPlayerErrorReporter(this))),
      pending_frame_set_(PendingFrameSet::Create()),
      enable_opt_vtb_decoder_handler_(enable_opt_vtb_decoder_handler) {
  transform_.fill(0.f);
  transform_[0] = 1.f;
  transform_[5] = 1.f;
  transform_[10] = 1.f;
  transform_[15] = 1.f;
}

VideoPlayerIOS::~VideoPlayerIOS() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  current_frame_ = nullptr;
  frame_cache_ = nullptr;
  ReleaseSession();
}

VideoFrameCache *VideoPlayerIOS::GetFrameCache() {
  if (!frame_cache_) {
    ThreadAssert::Assert(ThreadAssert::Type::kGPU);
    frame_cache_ = std::make_unique<VideoFrameCache>();
  }
  return frame_cache_.get();
}

int32_t VideoPlayerIOS::GetNextFrameToDecode(int32_t presentation_frame) {
  const int32_t key_frame = asset_->GetPrevKeyFrame(presentation_frame);
  do {
    if (CURRENT_FRAME_INVALID == current_decoded_frame_) {
      break;
    }
    if (presentation_frame < current_presentation_frame_) {
      break;
    }
    // presentation_frame > current_presentation_frame_
    // continue to decode or decode from key frame
    if (key_frame != asset_->GetPrevKeyFrame(current_decoded_frame_)) {
      // in different GOP
      break;
    }
    GetFrameCache()->ClearCache(key_frame, presentation_frame);
    pending_frame_set_->ClearPendingFrame(key_frame, presentation_frame);
    return current_decoded_frame_ + 1;

  } while (0);
  GetFrameCache()->ClearAll();
  pending_frame_set_->ClearPendingFrameAll();
  return key_frame;
}

void VideoPlayerIOS::MoveFrameFromCache(int32_t presentation_frame) {
  current_frame_ = GetFrameCache()->Evict(presentation_frame);
  current_presentation_frame_ = presentation_frame;
}

std::unique_ptr<TextureInfo> VideoPlayerIOS::UpdateTexture(const int32_t frame) {
  if (should_restart_) {
    should_restart_ = false;
    ResetSession();
  }
  if (!session_valid_) {
    return nullptr;
  }
  const int32_t presentation_frame = frame;
  DCHECK(0 <= presentation_frame && presentation_frame < asset_->GetFrameCount());

  if (presentation_frame == current_presentation_frame_) {
    // don't need draw
    return nullptr;
  }
  // there may be pending frames become ready, or error occurs
  PendingFrameSet::FlushResult flush_result = FlushPendingFrameSet(presentation_frame, false);
  if (!session_valid_) {
    // error occurs
    return nullptr;
  }
  // check whether cache hit
  if (GetFrameCache()->Contains(presentation_frame)) {
    MoveFrameFromCache(presentation_frame);
    error_reporter_->HasDrewOnce();
    PrepareNextFrame(presentation_frame);
    return std::make_unique<TextureInfoMTL>(current_frame_->GetMTLTexture(),
                                            asset_->GetVideoWidth(), asset_->GetVideoHeight());
  }
  if (!flush_result.required_frame_pending) {
    int32_t frame_to_decode = GetNextFrameToDecode(presentation_frame);
    int32_t presentation_index_decoded = -1;
    do {
      presentation_index_decoded = DecodeFrame(frame_to_decode, presentation_frame);
      if (presentation_index_decoded < 0) {
        // error occurs
        return nullptr;
      }
      ++frame_to_decode;
    } while (presentation_index_decoded != presentation_frame);
  }
  flush_result = FlushPendingFrameSet(presentation_frame, true);
  if (!session_valid_) {
    // error occurs
    return nullptr;
  }

  id<MTLTexture> texture = nil;
  if (GetFrameCache()->Contains(presentation_frame)) {
    MoveFrameFromCache(presentation_frame);
    texture = current_frame_->GetMTLTexture();
    error_reporter_->HasDrewOnce();
    PrepareNextFrame(presentation_frame);
  } else {
    // time out
    DCHECK(flush_result.required_frame_pending);
  }
  return std::make_unique<TextureInfoMTL>(texture, asset_->GetVideoWidth(),
                                          asset_->GetVideoHeight());
}

PendingFrameSet::FlushResult VideoPlayerIOS::FlushPendingFrameSet(
    const int32_t required_presentation_index, const bool sync) {
  DCHECK(session_valid_);
  PendingFrameSet::FlushResult result =
      sync ? pending_frame_set_->FlushSync(required_presentation_index,
                                           1000.f / asset_->GetFrameRate())
           : pending_frame_set_->Flush(required_presentation_index);
  for (auto &[presentation_index, pixel_buffer] : result.ready_frame_map) {
    GetFrameCache()->Add(presentation_index, std::move(pixel_buffer));
  }
  result.ready_frame_map.clear();
  if (noErr != result.first_error_status) {
    if (kVTInvalidSessionErr == result.first_error_status) {
      ANIMAX_LOGI("decode frame error from callback: kVTInvalidSessionErr");
      should_restart_ = true;
    } else {
      error_reporter_->ReportErrorOnce(
          "decode frame error from callback, status: " + std::to_string(result.first_error_status),
          VideoPlayerErrorReporter::Error::kDecodeFrameError);
    }
    session_valid_ = false;
  }
  return result;
}

bool VideoPlayerIOS::DecodeFrameData(CMSampleBufferRef sample_buffer,
                                     const int32_t presentation_index,
                                     const bool need_output_frame) {
  pending_frame_set_->WillDecodeFrame(presentation_index, need_output_frame);

  DecompressionInfo *decompression_info_ptr = nullptr;
  decompression_info_ptr = new DecompressionInfo{
      .weak_pending_frame_set = pending_frame_set_,
      .presentation_index = presentation_index,
  };
  VTDecodeFrameFlags decode_flags = kVTDecodeFrame_EnableAsynchronousDecompression;
  if (!need_output_frame) {
    decode_flags |= kVTDecodeFrame_DoNotOutputFrame;
  }
  VTDecodeInfoFlags flags_out = 0;
  OSStatus status = VTDecompressionSessionDecodeFrame(session_, sample_buffer, decode_flags,
                                                      decompression_info_ptr, &flags_out);
  if (noErr != status) {
    session_valid_ = false;
    if (kVTInvalidSessionErr == status) {
      ANIMAX_LOGI("decode frame error: kVTInvalidSessionErr");
      should_restart_ = true;
    } else {
      error_reporter_->ReportErrorOnce("decode frame error, status: " + std::to_string(status),
                                       VideoPlayerErrorReporter::Error::kDecodeFrameError);
      ProcessVtbCodecError(status);
    }
    pending_frame_set_->DidDecodeFrameFailDirectly(presentation_index);

    // From @discussion of VTDecompressionSessionDecodeFrame:
    // If an error is returned from this function, there will be no callback.  Otherwise the
    // callback provided during VTDecompressionSessionCreate will be called.

    // But in fact, if an error is returned from VTDecompressionSessionDecodeFrame, the callback
    // provided during VTDecompressionSessionCreate may also be called. So we can't delete
    // decompression_info_ptr here. Just leaving memory leaking.

    // delete decompression_info_ptr;
    return false;
  }
  return true;
}

int32_t VideoPlayerIOS::DecodeFrame(const int32_t decode_index,
                                    const int32_t target_presentation_index) {
  DCHECK(session_valid_);
  const FrameInfo &info = asset_->GetFrameInfo(decode_index);
  CMSampleBufferRef sample_buffer = PrepareFrameData(info);
  if (!sample_buffer) {
    // prepare frame data error. Try again next time
    return -1;
  }
  bool success = DecodeFrameData(sample_buffer, info.presentation_index_,
                                 info.presentation_index_ >= target_presentation_index);
  CFRelease(sample_buffer);
  if (!success) {
    // decode error, session is invalid
    DCHECK(!session_);
    return -1;
  }
  current_decoded_frame_ = decode_index;
  return info.presentation_index_;
}

void VideoPlayerIOS::PrepareNextFrame(const int32_t target_presentation_index) {
  if (!session_valid_) {
    return;
  }
  const int32_t decode_index = (CURRENT_FRAME_INVALID == current_decoded_frame_)
                                   ? 0
                                   : ((current_decoded_frame_ + 1) % asset_->GetFrameCount());
  DecodeFrame(decode_index, target_presentation_index);
}

void VideoPlayerIOS::AttachAsset(VideoAsset *asset) {
  auto asset_ios = static_cast<VideoAssetIOS *>(asset);
  if (!asset_ios || !asset_ios->IsValid()) {
    return;
  }

  asset_ = asset_ios;
  ResetSession();
  PrepareNextFrame(0);
}

void VideoPlayerIOS::ResetSession() {
  ReleaseSession();

  VTDecompressionOutputCallbackRecord callback{
      .decompressionOutputCallback = AnimaXVideoDecompression,
      .decompressionOutputRefCon = enable_opt_vtb_decoder_handler_ ? this : nullptr,
  };
  OSStatus status = VTDecompressionSessionCreate(
      kCFAllocatorDefault, asset_->GetFormatDescription(), nullptr, (__bridge CFDictionaryRef) @{
        (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
        (id)kCVPixelBufferMetalCompatibilityKey : @YES,
        (id)kCVPixelBufferIOSurfacePropertiesKey : @{},
      },
      &callback, &session_);
  if (!session_) {
    error_reporter_->ReportErrorOnce("video toolbox create status: " + std::to_string(status) +
                                         ", session: " + std::to_string(!!session_),
                                     VideoPlayerErrorReporter::Error::kCreateSessionError);
  } else {
    session_valid_ = true;
  }
}

void VideoPlayerIOS::ReleaseSession() {
  if (session_) {
    VTDecompressionSessionWaitForAsynchronousFrames(session_);
    VTDecompressionSessionInvalidate(session_);
    CFRelease(session_);
    session_ = nullptr;
  }
  session_valid_ = false;
  current_decoded_frame_ = CURRENT_FRAME_INVALID;
  current_presentation_frame_ = CURRENT_FRAME_INVALID;
  current_frame_ = nullptr;
  frame_cache_ = nullptr;
  pending_frame_set_->Reset();
}

CMSampleBufferRef VideoPlayerIOS::PrepareFrameData(const FrameInfo &frame_info) {
  size_t frame_size = frame_info.end_ - frame_info.beg_;
  CMBlockBufferRef block_buffer = nullptr;
  CMSampleBufferRef sample_buffer = nullptr;
  std::vector<size_t> sample_size(1, frame_size);
  std::vector<CMSampleTimingInfo> timing_infos(1, frame_info.timing_info_);
  OSStatus status;
  status = CMBlockBufferCreateWithMemoryBlock(
      kCFAllocatorDefault, asset_->GetFrameData() + frame_info.beg_, frame_size, kCFAllocatorNull,
      nullptr, 0, frame_size, 0, &block_buffer);
  if (kCMBlockBufferNoErr != status || !block_buffer) {
    error_reporter_->ReportErrorOnce("create block buffer error: " + std::to_string(status) +
                                         ", block buffer: " + std::to_string(!!block_buffer),
                                     VideoPlayerErrorReporter::Error::kCreateBlockBufferError);
    if (block_buffer) {
      CFRelease(block_buffer);
    }
    return nullptr;
  }

  status =
      CMSampleBufferCreateReady(kCFAllocatorDefault, block_buffer, asset_->GetFormatDescription(),
                                sample_size.size(), timing_infos.size(), timing_infos.data(),
                                sample_size.size(), sample_size.data(), &sample_buffer);
  CFRelease(block_buffer);

  if (noErr != status) {
    error_reporter_->ReportErrorOnce("create sample buffer error: " + std::to_string(status) +
                                         ", sample buffer: " + std::to_string(!!sample_buffer),
                                     VideoPlayerErrorReporter::Error::kCreateSampleBufferError);
    if (sample_buffer) {
      CFRelease(sample_buffer);
    }
    return nullptr;
  }
  return sample_buffer;
}

void VideoPlayerIOS::ProcessVtbCodecError(OSStatus status) {
  std::unique_lock lock(mutex_);

  if (noErr == status || !enable_opt_vtb_decoder_handler_) {
    return;
  }

  switch (status) {
    case kVTInvalidSessionErr:
      session_valid_ = false;
      should_restart_ = true;
      break;
    case kVTVideoDecoderUnsupportedDataFormatErr:
    case kVTVideoDecoderNotAvailableNowErr:
      session_valid_ = false;
      break;
    case kVTVideoDecoderBadDataErr:
    case kVTVideoDecoderMalfunctionErr:
      session_valid_ = false;
      if (decoder_retry_count_ < kMaxRetryCount) {
        decoder_retry_count_++;
        should_restart_ = true;
        ANIMAX_LOGE("decode frame error and retry, error :" << std::to_string(status));
      }
      break;
    default:
      break;
  }
}

const std::array<float, 16> &VideoPlayerIOS::GetTransform() { return transform_; }

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(const AnimaXAbility *ability_ptr) {
  bool enable_opt_vtb_decoder_handler = false;
  if (ability_ptr) {
    auto *ios_ability = static_cast<const AnimaXAbilityIOS *>(ability_ptr);
    enable_opt_vtb_decoder_handler = ios_ability->EnableOptVtbErrorHandler();
  }

  return std::make_unique<VideoPlayerIOS>(enable_opt_vtb_decoder_handler);
  ;
}

}  // namespace animax
}  // namespace lynx

static void AnimaXVideoDecompression(void *decompressionOutputRefCon, void *sourceFrameRefCon,
                                     OSStatus status, VTDecodeInfoFlags infoFlags,
                                     CVImageBufferRef imageBuffer, CMTime presentationTimeStamp,
                                     CMTime presentationDuration) {
  DCHECK(sourceFrameRefCon);
  auto *decompression_info_ptr =
      reinterpret_cast<lynx::animax::DecompressionInfo *>(sourceFrameRefCon);
  const int32_t presentation_index = decompression_info_ptr->presentation_index;
  auto pending_frame_set = decompression_info_ptr->weak_pending_frame_set.lock();
  const bool alive = pending_frame_set.get();
  if (pending_frame_set) {
    pending_frame_set->DidDecodeFrame(status, imageBuffer, presentation_index);
  }
  if (noErr != status) {
    ANIMAX_LOGE("AnimaXVideoDecompression error: " << std::to_string(status)
                                                   << ", pending_frame_set alive: "
                                                   << std::to_string(alive));

    auto player = (lynx::animax::VideoPlayerIOS *)decompressionOutputRefCon;
    if (player) {
      player->ProcessVtbCodecError(status);
    }
  }

  delete decompression_info_ptr;
}
