// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_DARWIN_FRAME_INFO_H_
#define ANIMAX_SRC_VIDEO_DARWIN_FRAME_INFO_H_

#import <CoreMedia/CoreMedia.h>

namespace lynx {
namespace animax {

class FrameInfo {
 public:
  int32_t beg_ = 0;
  int32_t end_ = 0;
  CMSampleTimingInfo timing_info_;
  int32_t presentation_index_ = 0;
  FrameInfo(int32_t beg, int32_t end, const CMSampleTimingInfo &timing_info)
      : beg_(beg), end_(end), timing_info_(timing_info) {}
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_FRAME_INFO_H_
