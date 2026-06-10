// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_DARWIN_VIDEO_FRAME_CACHE_H_
#define ANIMAX_SRC_VIDEO_DARWIN_VIDEO_FRAME_CACHE_H_

#import <CoreVideo/CoreVideo.h>

#include <memory>
#include <unordered_map>

#include "src/video/darwin/video_frame.h"

namespace lynx {
namespace animax {
class VideoFrame;
class VideoFrameCache final {
 public:
  VideoFrameCache();
  ~VideoFrameCache();
  VideoFrameCache(VideoFrameCache &&other);
  VideoFrameCache &operator=(VideoFrameCache &&other) = delete;
  VideoFrameCache(const VideoFrameCache &) = delete;
  VideoFrameCache &operator=(const VideoFrameCache &) = delete;

  /**
   * Add a pixel buffer to cache.
   * @param presentation_index index of pixel buffer in video.
   * @param pixel_buffer       pixel buffer of video frame.
   */
  void Add(int32_t presentation_index, CVPixelBufferRef pixel_buffer);
  /**
   * Whether cache contains frame of presentation_index.
   * @param presentation_index index of VideoFrame.
   * @return true if contains
   */
  bool Contains(int32_t presentation_index) const;
  /**
   * Evict video frame and get its ownership.
   * After calling this, VideoFrameCache doesn't contain VideoFrame of
   * presentation_index.
   * @param presentation_index index of VideoFrame.
   * @return VideoFrame evicted
   */
  std::unique_ptr<VideoFrame> Evict(int32_t presentation_index);
  /**
   * Clear cache from 'from_frame' to 'to_frame'.
   * @param from_frame presentation_index included.
   * @param to_frame   presentation_index excluded.
   */
  void ClearCache(int32_t from_frame, int32_t to_frame);
  /**
   * Clear all cache.
   */
  void ClearAll();

 private:
  // first is presentation index, second is VideoFrame
  std::unordered_map<int32_t, std::unique_ptr<VideoFrame>> frames_;
  CVMetalTextureCacheRef texture_cache_ref_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_VIDEO_FRAME_CACHE_H_
