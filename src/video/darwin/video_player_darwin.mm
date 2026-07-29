// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/darwin/video_player_darwin.h"

#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/player/animax_event.h"
#include "src/render/texture_info_mtl.h"
#include "src/video/darwin/pending_frame_set.h"
#include "src/video/darwin/video_asset_darwin.h"
#include "src/video/darwin/video_frame.h"
#include "src/video/darwin/video_frame_cache.h"

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
    //    kCreateBlockBufferError = 3,
    kDecodeFrameError = 4,
  };
  VideoPlayerErrorReporter(VideoPlayerDarwin *player) : player_(player) {}

  void HasDrewOnce();
  void ReportErrorOnce(const std::string &err_msg, const Error err_code);
  void ReportHasDrawnOnceAfterError();

 private:
  void Log(const std::string &err_msg) const;

  std::unordered_set<Error> error_reported_;
  int32_t error_count_ = 0;
  bool has_drawn_once_after_error_reported = false;
  VideoPlayerDarwin *player_ = nullptr;
};

void VideoPlayerErrorReporter::ReportHasDrawnOnceAfterError() {
  if (!has_drawn_once_after_error_reported && !error_reported_.empty()) {
    has_drawn_once_after_error_reported = true;
    const auto err_msg =
        std::string("Error has occurred before HasDrewOnce called, has decode frame error: ") +
        std::to_string(error_reported_.count(Error::kDecodeFrameError));
    if (auto listener = player_->weak_listener_.lock()) {
      listener->OnLayerError(EventError::kVideoPlayerError, err_msg);
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
  if (auto listener = player_->weak_listener_.lock()) {
    listener->OnLayerError(EventError::kVideoPlayerError, err_msg);
  }
}

void VideoPlayerErrorReporter::Log(const std::string &err_msg) const {
  ANIMAX_LOGE("Error(" << static_cast<int32_t>(error_count_) << "): " << err_msg);
}

VideoPlayerDarwin::VideoPlayerDarwin()
    : error_reporter_(
          std::unique_ptr<VideoPlayerErrorReporter>(new VideoPlayerErrorReporter(this))),
      pending_frame_set_(PendingFrameSet::Create()) {
  transform_.fill(0.f);
  transform_[0] = 1.f;
  transform_[5] = 1.f;
  transform_[10] = 1.f;
  transform_[15] = 1.f;
}

void VideoPlayerDarwin::UpdateOutputFrameSize(const int32_t width, const int32_t height) {
  if (width != output_width_ || height != output_height_) {
    should_restart_ = true;
    output_width_ = width;
    output_height_ = height;
  }
}

VideoPlayerDarwin::~VideoPlayerDarwin() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  current_frame_ = nullptr;
  frame_cache_ = nullptr;
  ReleaseSession();
}

VideoFrameCache *VideoPlayerDarwin::GetFrameCache() {
  if (!frame_cache_) {
    ThreadAssert::Assert(ThreadAssert::Type::kGPU);
    frame_cache_ = std::make_unique<VideoFrameCache>();
  }
  return frame_cache_.get();
}

int32_t VideoPlayerDarwin::GetNextFrameToDecode(int32_t presentation_frame) {
  const int decode_frame = asset_->GetDecodeFrame(presentation_frame);
  const int decode_start_frame = asset_->GetPrevKeyFrame(decode_frame);
  do {
    if (CURRENT_FRAME_INVALID == current_decoded_frame_) {
      break;
    }
    if (presentation_frame < current_presentation_frame_) {
      break;
    }
    if (current_decoded_frame_ < decode_start_frame || current_decoded_frame_ >= decode_frame) {
      break;
    }
    const int32_t decode_start_presentation_frame =
        asset_->GetFrameInfo(decode_start_frame).presentation_index_;
    GetFrameCache()->ClearCache(decode_start_presentation_frame, presentation_frame);
    pending_frame_set_->ClearPendingFrame(decode_start_presentation_frame, presentation_frame);
    return current_decoded_frame_ + 1;

  } while (0);
  GetFrameCache()->ClearAll();
  pending_frame_set_->ClearPendingFrameAll();
  return decode_start_frame;
}

void VideoPlayerDarwin::MoveFrameFromCache(int32_t presentation_frame) {
  current_frame_ = GetFrameCache()->Evict(presentation_frame);
  current_presentation_frame_ = presentation_frame;
}

std::unique_ptr<TextureInfo> VideoPlayerDarwin::UpdateTexture(const int32_t frame) {
  if (should_restart_) {
    should_restart_ = false;
    ResetSession();
  }
  if (!session_valid_) {
    return nullptr;
  }
  const int32_t presentation_frame = frame;
  DCHECK(0 <= presentation_frame && presentation_frame < asset_->GetFrameCount());

  if (presentation_frame < 0 || presentation_frame >= asset_->GetFrameCount() ||
      presentation_frame == current_presentation_frame_) {
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
    return std::make_unique<TextureInfoMTL>(current_frame_->GetMTLTexture(), output_width_,
                                            output_height_);
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
  return std::make_unique<TextureInfoMTL>(texture, output_width_, output_height_);
}

PendingFrameSet::FlushResult VideoPlayerDarwin::FlushPendingFrameSet(
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
    HandleDecodeError(result.first_error_status, "decode frame error from callback");
  }
  return result;
}

bool VideoPlayerDarwin::DecodeFrameData(CMSampleBufferRef sample_buffer,
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
    HandleDecodeError(status, "decode frame error");
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

bool VideoPlayerDarwin::IsRestartableDecodeError(OSStatus status) const {
  return status == kVTVideoDecoderMalfunctionErr || status == kVTVideoDecoderBadDataErr ||
         status == PendingFrameSet::kDecodeCallbackMissingImageBufferErr;
}

void VideoPlayerDarwin::HandleDecodeError(OSStatus status, const std::string &message_prefix) {
  session_valid_ = false;
  if (status == kVTInvalidSessionErr) {
    should_restart_ = true;
    ANIMAX_LOGI(message_prefix << ": kVTInvalidSessionErr, retry decoder");
    return;
  }

  if (IsRestartableDecodeError(status) && decoder_retry_count_ < kMaxRetryCount) {
    ++decoder_retry_count_;
    should_restart_ = true;
    ANIMAX_LOGI(message_prefix << ", retry decoder, status: " << status
                               << ", retry count: " << decoder_retry_count_);
    return;
  }

  error_reporter_->ReportErrorOnce(message_prefix + ", status: " + std::to_string(status),
                                   VideoPlayerErrorReporter::Error::kDecodeFrameError);
}

int32_t VideoPlayerDarwin::DecodeFrame(const int32_t decode_index,
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

void VideoPlayerDarwin::PrepareNextFrame(const int32_t target_presentation_index) {
  if (!session_valid_) {
    return;
  }
  if (asset_->GetFrameCount() == 0) {
    return;
  }
  const int32_t decode_index = (CURRENT_FRAME_INVALID == current_decoded_frame_)
                                   ? 0
                                   : ((current_decoded_frame_ + 1) % asset_->GetFrameCount());
  DecodeFrame(decode_index, target_presentation_index);
}

void VideoPlayerDarwin::AttachAsset(std::shared_ptr<VideoAsset> asset) {
  auto asset_darwin = std::static_pointer_cast<VideoAssetDarwin>(asset);
  if (!asset_darwin || !asset_darwin->IsValid()) {
    return;
  }

  asset_ = asset_darwin;
  decoder_retry_count_ = 0;
  output_width_ = asset_->GetVideoWidth();
  output_height_ = asset_->GetVideoHeight();
  ResetSession();
  PrepareNextFrame(0);
}

void VideoPlayerDarwin::ResetSession() {
  ReleaseSession();

  VTDecompressionOutputCallbackRecord callback{
      .decompressionOutputCallback = AnimaXVideoDecompression,
      .decompressionOutputRefCon = nullptr,
  };

  NSMutableDictionary *pixelBufferAttrs = [@{
    (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
    (id)kCVPixelBufferMetalCompatibilityKey : @YES,
    (id)kCVPixelBufferIOSurfacePropertiesKey : @{},
  } mutableCopy];

  if (output_width_ > 0 && output_height_ > 0) {
    pixelBufferAttrs[(id)kCVPixelBufferWidthKey] = @(output_width_);
    pixelBufferAttrs[(id)kCVPixelBufferHeightKey] = @(output_height_);
  }

  OSStatus status = VTDecompressionSessionCreate(
      kCFAllocatorDefault, asset_->GetFormatDescription(), nullptr,
      (__bridge CFDictionaryRef)pixelBufferAttrs, &callback, &session_);
  if (!session_) {
    error_reporter_->ReportErrorOnce("video toolbox create status: " + std::to_string(status) +
                                         ", session: " + std::to_string(!!session_),
                                     VideoPlayerErrorReporter::Error::kCreateSessionError);
  } else {
    session_valid_ = true;
  }
}

void VideoPlayerDarwin::ReleaseSession() {
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

CMSampleBufferRef VideoPlayerDarwin::PrepareFrameData(const FrameInfo &frame_info) {
  if (frame_info.beg_ < 0 || frame_info.end_ < 0 || frame_info.beg_ >= frame_info.end_ ||
      frame_info.end_ > asset_->GetFrameDataLength()) {
    return nullptr;
  }

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
    ANIMAX_LOGE("create block buffer error: " + std::to_string(status) +
                ", block buffer: " + std::to_string(!!block_buffer));
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

const std::array<float, 16> &VideoPlayerDarwin::GetTransform() { return transform_; }

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(std::shared_ptr<AnimaXAbility> ability) {
  return std::make_unique<VideoPlayerDarwin>();
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
  }

  delete decompression_info_ptr;
}
