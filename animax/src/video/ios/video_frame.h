// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_IOS_VIDEO_FRAME_H_
#define ANIMAX_SRC_VIDEO_IOS_VIDEO_FRAME_H_

#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>

namespace lynx {
namespace animax {

class VideoFrame final {
 public:
  /**
   * Constructor
   * @param pixel_buffer      getting from VTDecompressionSessionDecodeFrame, so
   * CVPixelBufferRetain won't be called in constructor, CVPixelBufferRelease
   * will be called in destructor.
   * @param texture_cache_ref should stay available in lifecycle of VideoFrame,
   * VideoFrame won't retain it
   */
  VideoFrame(CVPixelBufferRef pixel_buffer,
             CVMetalTextureCacheRef texture_cache_ref);
  ~VideoFrame();
  VideoFrame(VideoFrame &&other);
  VideoFrame &operator=(VideoFrame &&other);
  VideoFrame(const VideoFrame &) = delete;
  VideoFrame &operator=(const VideoFrame &) = delete;

  id<MTLTexture> GetMTLTexture();

 private:
  CVPixelBufferRef pixel_buffer_ = nullptr;
  CVMetalTextureCacheRef texture_cache_ref_ = nullptr;
  CVMetalTextureRef texture_ref_ = nullptr;
  id<MTLTexture> texture_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_IOS_VIDEO_FRAME_H_
