// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/conformance_test/frame_progress_converter.h"

namespace lynx {
namespace animax {

float FrameProgressConverter::ProgressForFrame(int32_t frame) {
  if (!IsFrameValid(frame)) {
    return 0.0;
  }

  return static_cast<float>(frame - start_frame_) /
         static_cast<float>(end_frame_ - start_frame_);
}

}  // namespace animax
}  // namespace lynx
