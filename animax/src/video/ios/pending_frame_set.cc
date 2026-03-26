// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/ios/pending_frame_set.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

std::shared_ptr<PendingFrameSet> PendingFrameSet::Create() {
  std::shared_ptr<PendingFrameSet> pending_set{new PendingFrameSet};
  return pending_set;
}

PendingFrameSet::~PendingFrameSet() { Reset(); }

void PendingFrameSet::WillDecodeFrame(int32_t presentation_index,
                                      bool need_output_frame) {
  if (need_output_frame) {
    std::scoped_lock<std::mutex> lock(m_);
    pending_frame_set_.insert(presentation_index);
  }
}

PendingFrameSet::FlushResult PendingFrameSet::Flush(
    const int32_t required_frame) {
  std::scoped_lock<std::mutex> lock(m_);
  bool required_frame_pending =
      pending_frame_set_.end() != pending_frame_set_.find(required_frame);
  FlushResult result{
      .ready_frame_map = std::move(ready_frame_map_),
      .first_error_status = first_error_status_,
      .required_frame_pending = required_frame_pending,
  };
  ready_frame_map_.clear();
  first_error_status_ = noErr;
  return result;
}

PendingFrameSet::FlushResult PendingFrameSet::FlushSync(
    const int32_t required_frame, int32_t time_out_ms) {
  std::unique_lock<std::mutex> lock(m_);
  bool required_frame_pending =
      pending_frame_set_.end() != pending_frame_set_.find(required_frame);
  if (required_frame_pending) {
    // Usually the frame rate of video is 30, so we set 33ms for the limit of
    // longest process time.
    time_out_ms = time_out_ms <= 0 ? 33 : time_out_ms;
    cv_.wait_for(lock, std::chrono::milliseconds(time_out_ms),
                 [required_frame, this] {
                   return (ready_frame_map_.end() !=
                           ready_frame_map_.find(required_frame)) ||
                          (noErr != first_error_status_);
                 });
    required_frame_pending =
        pending_frame_set_.end() != pending_frame_set_.find(required_frame);
  } else {
    DCHECK(ready_frame_map_.end() != ready_frame_map_.find(required_frame));
  }
  FlushResult result{
      .ready_frame_map = std::move(ready_frame_map_),
      .first_error_status = first_error_status_,
      .required_frame_pending = required_frame_pending,
  };
  ready_frame_map_.clear();
  first_error_status_ = noErr;
  return result;
}

void PendingFrameSet::ClearPendingFrame(int32_t from_frame, int32_t to_frame) {
  std::scoped_lock<std::mutex> lock(m_);
  auto pending_frame_set_it = pending_frame_set_.begin();
  while (pending_frame_set_.end() != pending_frame_set_it) {
    if (*pending_frame_set_it >= from_frame &&
        *pending_frame_set_it < to_frame) {
      pending_frame_set_it = pending_frame_set_.erase(pending_frame_set_it);
    } else {
      ++pending_frame_set_it;
    }
  }

  auto ready_frame_map_it = ready_frame_map_.begin();
  while (ready_frame_map_.end() != ready_frame_map_it) {
    if (ready_frame_map_it->first >= from_frame &&
        ready_frame_map_it->first < to_frame) {
      CVPixelBufferRelease(ready_frame_map_it->second);
      ready_frame_map_it = ready_frame_map_.erase(ready_frame_map_it);
    } else {
      ++ready_frame_map_it;
    }
  }
}

void PendingFrameSet::ClearPendingFrameAll() {
  std::scoped_lock<std::mutex> lock(m_);
  pending_frame_set_.clear();
  for (auto &[presentation_index, pixel_buffer] : ready_frame_map_) {
    CVPixelBufferRelease(pixel_buffer);
  }
  ready_frame_map_.clear();
}

void PendingFrameSet::DidDecodeFrame(OSStatus status,
                                     CVImageBufferRef image_buffer,
                                     int32_t presentation_index) {
  // If error occurs, DidDecodeFrameFailDirectly and DidDecodeFrame may be
  // called both.
  bool need_notify = false;
  {
    std::scoped_lock<std::mutex> lock(m_);
    bool need_pixel_buffer = pending_frame_set_.count(presentation_index);
    // If image buffer is needed or there is error, notify the event.
    need_notify = need_pixel_buffer || (noErr != status);
    // update status
    if (noErr == first_error_status_ && noErr != status) {
      first_error_status_ = status;
    }
    // store image buffer if necessary
    CVPixelBufferRef pixel_buffer = nullptr;
    if (need_pixel_buffer && (noErr == status) && image_buffer) {
      pixel_buffer = CVPixelBufferRetain(image_buffer);
    }
    if (pixel_buffer) {
      DCHECK(ready_frame_map_.end() ==
             ready_frame_map_.find(presentation_index));
      if (auto it = ready_frame_map_.find(presentation_index);
          ready_frame_map_.end() != it) {
        // should never reach
        CVPixelBufferRelease(it->second);
      }
      ready_frame_map_[presentation_index] = pixel_buffer;
    } else if (need_pixel_buffer && (noErr == first_error_status_)) {
      // If we need pixel buffer, but can't get it from image buffer and origin
      // status is noError. Updata first_error_status_
      first_error_status_ = -1;
    }
    pending_frame_set_.erase(presentation_index);
  }
  if (need_notify) {
    cv_.notify_all();
  }
}

void PendingFrameSet::DidDecodeFrameFailDirectly(int32_t presentation_index) {
  // If error occurs, DidDecodeFrameFailDirectly and DidDecodeFrame may be
  // called both.
  std::scoped_lock<std::mutex> lock(m_);
  pending_frame_set_.erase(presentation_index);
}

void PendingFrameSet::Reset() {
  std::scoped_lock<std::mutex> lock(m_);
  pending_frame_set_.clear();
  for (auto &[presentation_index, pixel_buffer] : ready_frame_map_) {
    CVPixelBufferRelease(pixel_buffer);
  }
  ready_frame_map_.clear();
  first_error_status_ = noErr;
}

}  // namespace animax
}  // namespace lynx
