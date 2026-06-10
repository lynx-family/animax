// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_DARWIN_PENDING_FRAME_SET_H_
#define ANIMAX_SRC_VIDEO_DARWIN_PENDING_FRAME_SET_H_

#import <CoreVideo/CoreVideo.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace lynx {
namespace animax {

class PendingFrameSet : public std::enable_shared_from_this<PendingFrameSet> {
 public:
  static std::shared_ptr<PendingFrameSet> Create();
  ~PendingFrameSet();

  PendingFrameSet(PendingFrameSet &&) = delete;
  PendingFrameSet &operator=(PendingFrameSet &&) = delete;
  PendingFrameSet(const PendingFrameSet &) = delete;
  PendingFrameSet &operator=(const PendingFrameSet &) = delete;

  /**
   * Called by VideoPlayerDarwin, means VideoPlayerDarwin::DecodeFrameData will
   * be called.
   * @param presentation_index frame to decode
   * @param need_output_frame  If true, presentation_index will be added to
   * pending_frame_set_, which means DecodeFrameData(presentation_index) being
   * in uncompleted status. If false, noting will happen, which means
   * DecodeFrameData(presentation_index) being in completed status unless error
   * occurs in the future.
   */
  void WillDecodeFrame(int32_t presentation_index, bool need_output_frame);
  /**
   * Called by VideoPlayerDarwin, means VideoPlayerDarwin::DecodeFrameData
   * finished with error.
   * @param presentation_index same as param presentation_index of
   * WillDecodeFrame
   */
  void DidDecodeFrameFailDirectly(int32_t presentation_index);
  /**
   * Called by system, means VideoPlayerDarwin::DecodeFrameData finished.
   * @param status             whether decode succeeds
   * @param image_buffer       decode result
   * @param presentation_index same as param presentation_index of
   * WillDecodeFrame. It will be removed from pending_frame_set_, which means
   * DecodeFrameData(presentation_index) being in completed status.
   */
  void DidDecodeFrame(OSStatus status, CVImageBufferRef image_buffer,
                      int32_t presentation_index);

  class FlushResult {
   public:
    std::unordered_map<int32_t, CVPixelBufferRef> ready_frame_map;
    OSStatus first_error_status;
    bool required_frame_pending;
  };
  /**
   * Flush and get ready frames.
   * @param required_frame specify which frame we need. If
   * required_frame_pending is true, it means required_frame will be ready in
   * the future.
   */
  FlushResult Flush(const int32_t required_frame);
  /**
   * Flush and get ready frames. Make sure DecodeFrameData(required_frame) have
   * been called.
   * @param required_frame specify which frame we need. If
   * required_frame_pending is true, it means time out and required_frame will
   * be ready in the future.
   * @param time_out_ms time out limit.
   */
  FlushResult FlushSync(const int32_t required_frame, int32_t time_out_ms);
  /**
   * Mark some frames we don't need. After calling this, DecodeFrameData with
   * these frames are in completed status.
   * @param from_frame frame included.
   * @param to_frame   frame excluded.
   */
  void ClearPendingFrame(int32_t from_frame, int32_t to_frame);
  /**
   * Mark all the frames not needed. After calling this, all the previous
   * DecodeFrameData are in completed status.
   */
  void ClearPendingFrameAll();
  /**
   * Reset. Same as creating a new PendingFrameSet.
   */
  void Reset();

 private:
  PendingFrameSet() = default;
  std::unordered_set<int32_t> pending_frame_set_;
  std::unordered_map<int32_t, CVPixelBufferRef> ready_frame_map_;
  OSStatus first_error_status_ = noErr;
  std::mutex m_;
  std::condition_variable cv_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_PENDING_FRAME_SET_H_
